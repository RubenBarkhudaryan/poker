# 04 — CFR Solver

Файлы:
- `src/solver/Solver.*`
- `src/solver/PCfrSolver.*` (основной)
- `src/solver/BestResponse.*`
- `src/solver/CfrSolver.*` (старый/альтернативный путь)
- `src/experimental/TCfrSolver.*` (эксперимент)

---

## `Solver` (абстрактный базовый)

| Метод | Роль |
|-------|------|
| конструкторы | опционально сохранить `tree` |
| `getTree()` | доступ к дереву |
| `train()` = 0 | запуск обучения |
| `stop()` = 0 | мягкая остановка |
| `dumps(with_status, depth)` = 0 | JSON стратегии |
| `get_strategy(node, chance_cards)` = 0 | стратегия в узле с учётом карт |
| `get_evs(node, chance_cards)` = 0 | EV по действиям/рукам |

**`MonteCarolAlg`:** `NONE` | `PUBLIC` — в GUI-пути всегда `NONE` (полный public tree).

---

## `PCfrSolver` — главный солвер

Parallel / production CFR с discounted regrets, OpenMP, suit isomorphism, half-floats.

### Конструктор

Принимает дерево, ranges, board, compairer, deck, число итераций, debug, print_interval, logfile, trainer-имя (`"discounted_cfr"` и т.д.), MC-alg, warmup, accuracy, flags isomorphism/halffloats, `num_threads`.

**Что делает:**
1. Чистит ranges от дублей/пересечений с бордом.
2. Инициализирует `RiverRangeManager`, `PrivateCardsManager`.
3. `omp_set_num_threads`.
4. `setTrainable(root)` — развешивает storage на все ActionNode.
5. Выбирает `split_round` (следующая улица после корня) для параллельного разреза chance.

### `playerHands(player)`

Range игрока 0 или 1.

### `getReachProbs()`

Начальные веса рук (`PrivateCards.weight`) → `vector[player][hand]`.

### `noDuplicateRange`

Тот же фильтр, что во runtime (дубли + board block).

### `setTrainable(root)`

Рекурсия по дереву: на каждом `ActionNode` выделяет вектор `trainables` размера = число deal-комбинаций / абстракций; на chance-узлах продолжает вниз. Привязывает `player_privates` действующего игрока.

### `train()`

Главный цикл:

1. Опционально `findGameSpecificIsomorphisms()`.
2. Создаёт `BestResponse`, печатает начальную exploitability.
3. Для `i in 0..iteration_number-1`:
   - для каждого `player_id` вызывает `cfr(player, root, opp_reach, i, board, deal=0)`.
4. Каждые `print_interval` (после warmup) считает exploitability; если ≤ `accuracy` или `nowstop` — выход.

### `stop()` / `nowstop`

Флаг прерывания между итерациями (из UI Stop).

### `cfr(player, node, reach_probs, iter, current_board, deal)`

Диспетчер по `node->getType()`:
- ACTION → `actionUtility`
- CHANCE → `chanceUtility`
- SHOWDOWN → `showdownUtility`
- TERMINAL → `terminalUtility`

Возвращает EV-вектор по рукам **текущего** traversing-игрока (или нужный viewpoint — см. implementation detail в cpp).

### `actionUtility(...)`

1. Берёт `Trainable` для текущего `deal`.
2. Считает current strategy из regrets.
3. Для каждого действия умножает reach на σ(a), рекурсивно `cfr` в child.
4. Если это ход traversing-игрока — считает regrets и `updateRegrets`.
5. Возвращает ожидаемый utility.

### `chanceUtility(...)`

Перебирает возможные карты (с учётом isomorphism / `color_iso_offset`), обновляет `current_board` bitmask, корректирует reach (карты, которые «убивают» руку → 0), рекурсия в child, усреднение.

`getAllAbstractionDeal(deal)` помогает схлопнуть симметричные масти.

### `showdownUtility(...)`

Через `RiverRangeManager` получает ранги рук на борде, сравнивает, начисляет payoffs из `ShowdownNode` × reach оппонента.

### `terminalUtility(...)`

Фолд: payoffs из `TerminalNode` × вероятности дойти (без сравнения карт, но с блокировками пересечений рук).

### `findGameSpecificIsomorphisms()`

Заполняет `color_iso_offset` по борду/колоде: какие масти эквивалентны, чтобы не считать дубликаты turn/river карт.

### `purnTree()`

Заглушка (TODO): усечение дерева после warmup для экономии памяти — не реализовано.

### `dumps(with_status, depth)` / `reConvertJson` / `exchangeRange`

Сериализация стратегии в JSON. `reConvertJson` обходит дерево до `max_depth`; при isomorphism — `exchangeRange` переставляет руки по мастям, чтобы JSON был в «каноническом» виде.

### `get_strategy(node, chance_cards)` / `get_evs(...)`

API для UI: по текущему ActionNode и выбранным turn/river картам вернуть тензор стратегий/EV `[c1][c2][action]` (или аналогичную раскладку по deal).

---

## `BestResponse`

Считает, насколько текущая стратегия эксплуатируема: лучший ответ одного игрока против зафиксированной стратегии другого.

| Метод | Роль |
|-------|------|
| конструктор | ranges, pcm, rrm, deck, iso offsets, threads |
| `printExploitability(root, iter, pot, board)` | BR для обоих игроков → % pot; печатает в лог; возвращает число |
| `getBestReponseEv(...)` | EV лучшего ответа одного игрока |
| `bestResponse` | диспетчер по типу узла |
| `actionBestResponse` | в своём узле — max по действиям; в чужом — ожидание по стратегии оппонента |
| `chanceBestReponse` | усреднение по картам |
| `terminalBestReponse` / `showdownBestResponse` | payoffs |

Используется **внутри** `PCfrSolver::train` как метрика остановки, не обучает стратегию сам.

---

## `CfrSolver` (`CfrSolver.*`)

Более ранняя/альтернативная реализация CFR (не тот путь, что GUI вызывает через `PokerSolver::train` → всегда `PCfrSolver`). Имеет смысл смотреть только если сравниваете алгоритмы или чините legacy.

---

## `TCfrSolver` (`src/experimental/`)

Экспериментальный вариант. Не часть основного GUI-потока. Можно игнорировать при изучении «как работает продукт», пока не рефакторите experimental.

---

## Минимальная ментальная модель CFR здесь

```
для каждой итерации:
  для каждого игрока p:
    пройти дерево с reach_probs оппонента
    в узлах p: обновить regrets по разнице EV(action) - EV(strategy)
    стратегия = normalized positive regrets (discounted)
каждые N итераций:
  BestResponse → exploitability
  если достаточно мала → стоп
```

Средняя стратегия (`getAverageStrategy`) — то, что обычно показывают как «GTO-результат».
