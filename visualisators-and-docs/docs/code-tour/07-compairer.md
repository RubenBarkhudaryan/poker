# 07 — Compairer (сила рук)

Файлы: `src/compairer/Compairer.h`, `src/compairer/Dic5Compairer.*`

Без этого модуля showdown невозможен: солвер должен быстро сказать, какая 7-карточная рука сильнее.

---

## `Compairer` (абстрактный)

**`CompairResult`:** `LARGER` | `EQUAL` | `SMALLER`.

| Метод | Роль |
|-------|------|
| конструктор `(dic_dir, lines)` | путь к словарю + ожидаемое число строк |
| `compair(private_a, private_b, board)` = 0 | кто выиграл (Card или int перегрузки) |
| `get_rank(hand, board)` = 0 | числовой ранг (меньше = сильнее или наоборот — см. dic; используется для сортировки) |
| `get_rank(uint64_t hand, uint64_t board)` = 0 | то же по bitmask |

---

## `FiveCardsStrength` (внутри Dic5)

Компактное хранение силы **ровно 5 карт** (после выбора лучшей пятёрки из 7).

| Метод | Роль |
|-------|------|
| `convert(strength_map)` | построить внутренние flush/other maps |
| `load` / `save` | бинарный кэш (`card5_dic_zipped.bin`) |
| `operator[](hash)` | lookup ранга по hash пятёрки |
| `check` | верификация против полной map |

Разделение `flush_map` / `other_map` ускоряет и сжимает данные (флеши кодируются иначе).

Вспомогательные в cpp: `ranks_hash`, `is_flush` — разбор битовой маски карты на ранг/масть для хеша.

---

## `Dic5Compairer`

Реализация: lookup dictionary всех C(52,5) (holdem) или short-deck аналог.

### Конструктор `Dic5Compairer(dic_dir, lines, dic_dir_bin)`

1. Пытается быстро загрузить `.bin` через `FiveCardsStrength::load`.
2. Если нет — читает текстовый словарь, конвертирует, может сохранить bin.
3. Progress bar при долгой загрузке txt.

Именно поэтому `QSolverJob::loading` занимает время при старте GUI.

### `getRank(vector<Card|int>)` (private)

Для 5–7 карт: перебирает комбинации из 5, берёт лучший (минимальный/максимальный — по соглашению словаря) ранг через `fcs[hash]` / `cardslong2rank`.

### Публичные API

| Метод | Роль |
|-------|------|
| `get_rank(hand, board)` | сила руки на борде |
| `get_rank(uint64_t, uint64_t)` | то же через long |
| `compair(...)` | сравнить две руки: get_rank обеих → `compairRanks` |
| `compairRanks(a,b)` | static: LARGER/EQUAL/SMALLER |

### Где используется

- `RiverRangeManager` — проставить `RiverCombs.rank`
- `PCfrSolver::showdownUtility` / `BestResponse::showdownBestResponse` — выплаты
- Косвенно любой showdown-path в дереве

---

## Почему «Dic5»

Оценка холдем-руки = лучшая 5-карточная комбинация из 7 (2 hole + 5 board). Словарь хранит силу каждой возможной пятёрки; солвер не считает покерные паттерны вручную на каждом showdown — только hash + lookup. Это ключ к скорости относительно наивных evaluator’ов.
