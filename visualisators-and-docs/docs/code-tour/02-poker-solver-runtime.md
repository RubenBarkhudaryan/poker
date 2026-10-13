# 02 — Runtime: QSolverJob и PokerSolver

Файлы: `src/runtime/qsolverjob.*`, `src/runtime/PokerSolver.*`

Это фасад между UI/CLI и ядром (дерево + CFR).

---

## `QSolverJob` (`qsolverjob.h` / `.cpp`)

`QThread`-обёртка: тяжёлая работа не блокирует Qt event loop.

### Enums

**`Mode`:** `HOLDEM` | `SHORTDECK` — какой из двух `PokerSolver` активен.

**`MissionType`:**
| Значение | Действие в `run()` |
|----------|-------------------|
| `LOADING` | загрузка словарей силы рук |
| `BUILDTREE` | построение game tree |
| `SOLVING` | CFR train |
| `SAVING` | dump strategy JSON |

### Поля-параметры спота

Хранят то, что UI/CLI настраивает перед миссией: commits, round, raise_limit, blinds, stack, ranges, board, accuracy, iterations, isomorphism, half-floats, threads, `gtbs` (bet settings), `savefile`.

Два солвера: `ps_holdem`, `ps_shortdeck` — оба создаются при LOADING, чтобы переключение mode было мгновенным.

### `setContext(QSTextEdit* textEdit)`

Сохраняет указатель на лог (исторически для `QDebugStream`; сейчас логи идут через global message handler).

### `get_solver() → PokerSolver*`

Возвращает `&ps_holdem` или `&ps_shortdeck` по `mode`. Кидает, если mode неизвестен.

### `run()`

Точка входа `QThread`: switch по `current_mission`, ловит `runtime_error` и печатает в `qDebug`.

### `loading()`

**Зачем:** один раз при старте приложения подготовить компараторы.

**Что делает:**
- Holdem: ranks `2..A`, dic `card5_dic_sorted.txt` / `.bin`, ~2.6M строк.
- Shortdeck: ranks `6..A`, отдельный dic, ~377k строк.
- Ресурсы из Qt resource path `:/resources`.

### `build_tree()`

Делегирует в активный `PokerSolver::build_game_tree(...)` с полями job + `*gtbs`.

### `solving()`

Вызывает `PokerSolver::train(...)` с алгоритмом `"discounted_cfr"`, `warmup=-1`, ranges/board из полей job.

В коде есть TODO про утечки памяти при повторных solve из UI.

### `saving()`

Читает `dump_round` из `QSettings`, вызывает `dump_strategy(savefile, dump_rounds)`. Предупреждает, если dump до river (дорого по RAM).

### `stop()`

Пробрасывает `stop()` в активный `PokerSolver`.

### `estimate_tree_memory(range1, range2, board)`

Прокси к `PokerSolver::estimate_tree_memory`.

---

## `PokerSolver` (`PokerSolver.h` / `.cpp`)

Единый фасад одного варианта колоды (holdem или shortdeck).

### Состояние

| Поле | Смысл |
|------|--------|
| `compairer` | `Dic5Compairer` для showdown |
| `deck` | колода карт |
| `game_tree` | текущее дерево спота |
| `solver` | `PCfrSolver` после `train` |
| `player1Range` / `player2Range` | диапазоны после фильтра борда |

### Конструкторы

**`PokerSolver()`** — пустой (нужен для массива полей до LOADING).

**`PokerSolver(ranks, suits, compairer_file, lines, bin)`** — создаёт `Deck` из CSV ranks/suits и грузит `Dic5Compairer`.

### `load_game_tree(string game_tree_file)`

Строит `GameTree` из JSON-файла (альтернатива программному build). Используется чаще в консольных сценариях.

### `build_game_tree(oop_commit, ip_commit, current_round, raise_limit, blinds, stack, buildingSettings, allin_threshold)`

Создаёт дерево программно через конструктор `GameTree(deck, ...)`. Это путь GUI.

### `noDuplicateRange(range, board_long)` (свободная в cpp)

Фильтрует руки: убирает дубликаты hash и руки, пересекающиеся с бордом. Используется в `train`.

### `train(p1_range, p2_range, boards, log_file, iteration_number, print_interval, algorithm, warmup, accuracy, use_isomorphism, use_halffloats, threads)`

**Зачем:** полный цикл «диапазоны → CFR».

**Что делает:**
1. Парсит board-строку (`"Qs,Jh,2h"`) в int-карты.
2. `PrivateRangeConverter::rangeStr2Cards` для обоих игроков.
3. Фильтрует ranges через `noDuplicateRange`.
4. Создаёт `PCfrSolver` (algorithm строкой, обычно `"discounted_cfr"`).
5. `solver->train()`.

Параметр `algorithm` передаётся в конструктор solver как trainer-имя; Monte Carlo = `NONE`.

### `stop()`

Если solver есть — `solver->stop()` (флаг `nowstop` внутри CFR-цикла).

### `estimate_tree_memory(range1, range2, board)`

Требует уже построенное дерево. Парсит ranges/board, вызывает `game_tree->estimate_tree_memory(deck_left, p1_size, p2_size)`.

### `dump_strategy(dump_file, dump_rounds)`

`solver->dumps(false, dump_rounds)` → JSON в файл. `dump_rounds` ограничивает глубину (flop/turn/river) — чем больше, тем тяжелее.

### Геттеры

- `get_game_tree()` / `getGameTree()` — дерево
- `get_deck()` — колода
- `get_solver()` — текущий CFR solver (для UI StrategyExplorer)

---

## Как UI использует runtime

```
LOADING  (конструктор MainWindow)
   ↓
пользователь правит UI
   ↓
BUILDTREE  (Build Tree)
   ↓
SOLVING    (Solve)
   ↓
StrategyExplorer читает qSolverJob->get_solver()->get_solver()
   ↓
SAVING     (Export JSON)
```
