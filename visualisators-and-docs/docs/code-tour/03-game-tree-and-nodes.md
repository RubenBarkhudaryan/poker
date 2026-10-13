# 03 — Game Tree и узлы

Файлы:
- `src/GameTree.h` / `.cpp`
- `src/nodes/*`
- `src/tools/Rule.*`, `StreetSetting.*`, `GameTreeBuildingSettings.*`

Дерево описывает все легальные линии спота. CFR потом «обучает» только `ActionNode`.

---

## Настройки построения

### `StreetSetting`

Хранит сайзинги одного игрока на одном стрите:

- `bet_sizes` — % банка (или ×100 от `x`-нотации)
- `raise_sizes`
- `donk_sizes` (обычно только OOP turn/river)
- `allin` — разрешён ли all-in как отдельное действие

**Конструктор** просто сохраняет поля.

### `GameTreeBuildingSettings`

Шесть `StreetSetting`: flop/turn/river × ip/oop.

**`get_setting(player, round)`** — строковый доступ (`"ip"|"oop"`, `"flop"|"turn"|"river"`).

### `Rule`

Контекст построения дерева (живой «стейт» по мере рекурсии):

| Поле | Смысл |
|------|--------|
| `deck` | колода |
| `oop_commit` / `ip_commit` | уже вложенное |
| `current_round` | 0 preflop … 3 river |
| `raise_limit` | макс. число рейзов в линии |
| `small_blind` / `big_blind` / `stack` | структура стека |
| `build_settings` | сайзинги |
| `allin_threshold` | если remaining stack / pot ниже порога — форсировать all-in вместо мелкого бета |
| `initial_effective_stack` | для расчётов |

**`get_pot()`** — сумма commits.  
**`get_commit(player)`** — commit конкретного игрока.

---

## Узлы (`src/nodes`)

### Базовый `GameTreeNode`

**Enums:**
- `PokerActions`: BEGIN, ROUNDBEGIN, BET, RAISE, CHECK, FOLD, CALL
- `GameTreeNodeType`: ACTION, SHOWDOWN, TERMINAL, CHANCE
- `GameRound`: PREFLOP, FLOP, TURN, RIVER

**Поля:** `round`, `pot`, `parent` (weak), `depth`, `subtree_size`.

| Метод | Зачем |
|-------|--------|
| конструкторы | задать round/pot/parent |
| `intToGameRound` / `gameRound2int` | конвертация int↔enum |
| `getParent` / `setParent` | навигация |
| `getRound` / `getPot` | метаданные узла |
| `printHistory` / `printNodeHistory` / `toString` | отладка линии |
| `getType()` | pure virtual — тип узла |

### `GameActions`

Одно действие: тип (`BET`/`RAISE`/…) + `amount` (для check/fold amount обычно 0).

| Метод | Роль |
|-------|------|
| конструкторы | создать действие |
| `getAction` / `getAmount` | чтение |
| `toString` / `pokerActionToString` | человекочитаемая метка для UI/JSON |

### `ActionNode`

Узел решения игрока.

| Метод | Зачем / что делает |
|-------|---------------------|
| конструктор | actions + childrens + player + round + pot + parent |
| `getActions` / `getChildrens` / `getPlayer` | структура ветвления |
| `setActions` / `setChildrens` | правка после build (редко) |
| `setTrainable(trainables, player_privates)` | привязать storage стратегий (массив по deal-индексам) |
| `getTrainable(i, create_on_site, use_halffloats)` | lazy-create `DiscountedCfrTrainable` / SF / HF: half-floats только на river |
| `getType()` | `ACTION` |

`trainables[i]` — отдельная стратегия для каждого «deal path» (разные карты chance выше по дереву). Это критично для памяти.

### `ChanceNode`

Раздача карты(и). Один общий `children` (поддерево переиспользуется); конкретная карта — параметр обхода, не отдельный child на каждую карту в идеальной схеме. В реализации `cards` — ссылка на список возможных карт.

| Метод | Роль |
|-------|------|
| конструктор | children, round, pot, parent, cards, donk-флаг |
| `getCards` / `getChildren` / `setChildren` | структура |
| `getPlayer` | обычно не decision player |
| `isDonk` | метка, что следующий action — donk-линия |
| `getType()` | `CHANCE` |

### `ShowdownNode`

Линия дошла до вскрытия.

| Метод | Роль |
|-------|------|
| конструктор | `tie_payoffs`, `player_payoffs[winner][player]`, round, pot, parent |
| `get_payoffs(result, winner)` | вектор выплат всем |
| `get_payoffs(result, winner, player)` | выплата одному |
| `getType()` | `SHOWDOWN` |

`ShowDownResult`: `NOTTIE` / `TIE`.

### `TerminalNode`

Линия закончилась фолдом (без сравнения рук).

| Метод | Роль |
|-------|------|
| конструкторы | payoffs + winner + meta |
| `get_payoffs()` | чипсы по игрокам |
| `getType()` | `TERMINAL` |

---

## `GameTree`

### Конструкторы

1. **`GameTree(tree_json_dir, deck)`** — загрузка из JSON: читает файл → `recurrentGenerateTreeNode`.
2. **`GameTree(deck, oop_commit, …, buildingSettings, allin_threshold)`** — программный build: создаёт `Rule`, корневой action/chance и вызывает `__build`.

### Построение

#### `__build(node, rule)` / `__build(node, rule, last_action, check_times, raise_times)`

Рекурсивный диспетчер: смотрит тип узла / round / last action и вызывает `buildAction` или `buildChance`, пока не дойдёт до terminal/showdown.

`check_times` / `raise_times` ограничивают ветвление (два check → следующая улица; raise_limit).

#### `buildChance(root, rule)`

Создаёт дочерний action-узел следующей улицы (или river showdown path), увеличивает `current_round` в правиле.

#### `buildAction(root, rule, last_action, check_times, raise_times)`

Генерирует список легальных `GameActions` (fold/check/call/bet/raise/allin) через `get_possible_bets` и настройки улицы; для каждого действия — child (новый Action / Chance / Terminal / Showdown) и рекурсия `__build`.

#### `get_possible_bets(root, player, next_player, rule, betType)`

Считает абсолютные размеры бетов/рейзов/donk из % pot относительно commits и stack. `BetType`: BET / DONK / RAISE. Учитывает `allin_threshold` и округление.

#### `round_nearest(number, round_num)`

Округление размера ставки к сетке (чипы).

#### `getSettings(int_round, player, settings)`

Достаёт нужный `StreetSetting` из `GameTreeBuildingSettings`.

### Загрузка из JSON

| Метод | Роль |
|-------|------|
| `readAllBytes` | открыть файл как stream |
| `strToGameRound` | `"flop"` → enum |
| `recurrentGenerateTreeNode` | json-объект → нужный тип узла |
| `generateActionNode` | meta + children actions/nodes |
| `generateChanceNode` | chance + один child |
| `generateShowdownNode` | payoffs из meta |
| `generateTerminalNode` | fold payoffs |

### Утилиты

| Метод | Роль |
|-------|------|
| `getRoot()` | корень для CFR/UI |
| `recurrentSetDepth` | проставить depth/subtree_size |
| `estimate_tree_memory` / `re_estimate_tree_memory` | оценка float-ячеек (действия × руки × deals) |
| `printTree` / `recurrentPrintTree` | отладочная печать |
| деструктор | освобождение дерева |

---

## Как читать дерево головой

```
Action (OOP flop)
 ├─ check → Action (IP)
 │            ├─ check → Chance (turn card) → Action ...
 │            ├─ bet 50% → Action (OOP) ...
 │            └─ ...
 ├─ bet 33% → ...
 └─ allin → Action (IP call/fold) → Showdown / Terminal
```

CFR обходит это дерево с `reach_probs` по каждой руке диапазона; стратегия живёт только в `ActionNode::trainables`.
