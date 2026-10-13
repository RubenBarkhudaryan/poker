# 06 — Карты, колода, диапазоны, library

Файлы:
- `src/Card.*`, `src/Deck.*`, `src/library.*`
- `src/ranges/*`
- `src/tools/PrivateRangeConverter.*`

---

## `Card`

Представление одной карты и утилиты bitmask-борда.

### Инстанс

| Метод | Роль |
|-------|------|
| конструкторы | из строки `"Ah"`, опционально индекс в колоде |
| `getCard` / `getCardInt` / `getNumberInDeckInt` | строка / 0..51 / позиция в deck |
| `empty` | пустая карта |
| `toString` / `toFormattedString` / `toFormattedHtml` | отображение (UI) |

### Статика — кодирование

Карта: `(rank-2)*4 + suit`, ranks 2..A → 0..12, suits c/d/h/s → 0..3.

| Метод | Роль |
|-------|------|
| `card2int` / `strCard2int` / `intCard2Str` | конвертации |
| `rankToInt` / `suitToInt` / `rankToString` / `suitToString` / `getSuits` | разбор символов |
| `boardInt2long` / `boardInts2long` / `boardCard2long` / `boardCards2long` | набор карт → `uint64_t` битовая маска (бит = id карты) |
| `boardsHasIntercept(a,b)` | `(a & b) != 0` — пересечение (блокер) |
| `long2board` / `long2boardCards` | маска → список |
| `boardCards2html` | HTML для UI |

Битовые маски — основа скорости: проверка «рука пересекается с бордом» = одна AND-операция.

---

## `Deck`

| Метод | Роль |
|-------|------|
| `Deck()` / `Deck(ranks, suits)` | декартово произведение ranks×suits → `vector<Card>` |
| `getCards()` | все карты колоды |
| `getRanks()` | список рангов (holdem 13, shortdeck 9) |

Shortdeck — просто колода без 2–5.

---

## `library.h` / `.cpp`

Общие утилиты:

### `Combinations<T>`

Итератор всех C(n,k) подмножеств. Используется при оценке рук / комбинаторике (5 карт из 7 и т.п. внутри компаратора/рейнджей).

| Метод | Роль |
|-------|------|
| конструктор `(set, m)` | генерирует все комбинации размера m |
| `begin` / `end` / `get` | доступ к результату |
| `generate` / `comb` | внутренняя генерация и C(n,k) |

### Свободные функции

| Функция | Роль |
|---------|------|
| `string_split(str, char)` | CSV-парсинг (`"As,Kh,2d"`) |
| `timeSinceEpochMillisec()` | тайминг итераций CFR |
| `random(min,max)` | RNG |
| `normalization_tanh(stack, ev, ratio)` | нормализация EV для UI-раскраски |

---

## `PrivateCards`

Одна рука диапазона: две hole-карты + вес.

| Поле/метод | Роль |
|------------|------|
| `card1`, `card2` | int-id карт |
| `weight` | абсолютный вес из range-строки (`AA:0.5`) |
| `relative_prob` | нормализованная вероятность (после `setRelativeProbs`) |
| конструкторы | создать руку |
| `toBoardLong()` | bitmask двух карт |
| `hashCode()` | уникальный ключ руки |
| `toString()` | `"AhKd"` |
| `get_hands()` | `{card1,card2}` |

---

## `PrivateCardsManager`

Индексация ranges обоих игроков и маппинг «рука игрока A ↔ совместимая рука игрока B».

| Метод | Роль |
|-------|------|
| конструкторы | сохранить `private_cards[player]`, board mask |
| `getPreflopCards(player)` | range игрока |
| `indPlayer2Player(from, to, index)` | индекс руки одного игрока → индекс совместимой руки другого (для showdown/BR) |
| `getInitialReachProb(player, board)` | начальные reach с учётом блокеров борда |
| `setRelativeProbs()` | заполнить `relative_prob` |

---

## `RiverCombs`

Кэш одной руки на конкретном ривере: `rank` силы + ссылка на `PrivateCards` + `reach_prob_index`.

Нужен, чтобы showdown не пересчитывал `get_rank` каждый раз с нуля для одной и той же маски борда.

---

## `RiverRangeManager`

Кэш `board_long → vector<RiverCombs>` отдельно для p1/p2.

| Метод | Роль |
|-------|------|
| конструкторы | привязать `Compairer` |
| `getRiverCombos(player, range, board\|board_long)` | вернуть (и закэшировать) отсортированные по rank руки, валидные на борде |

Mutex `maplock` — потокобезопасность при OpenMP.

---

## `PrivateRangeConverter`

| Метод | Роль |
|-------|------|
| `rangeStr2Cards(range_str, initial_boards)` | парсит строку вида `AA,KK,AKs:0.5,72o` в `vector<PrivateCards>`, выкидывая руки, пересекающиеся с бордом |

Это мост «текст из UI» → «структуры солвера».

---

## Типичный путь данных range

```
UI text "AA,AKs:0.5"
  → PrivateRangeConverter::rangeStr2Cards
  → vector<PrivateCards>
  → PokerSolver / PCfrSolver
  → PrivateCardsManager + веса reach_probs
  → на river: RiverRangeManager::getRiverCombos (+ Dic5Compairer ranks)
```
