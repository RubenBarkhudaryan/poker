# 05 — Trainable (хранение regrets / strategy)

Файлы: `src/trainable/*`

На каждом `ActionNode` × deal-индексе висит объект `Trainable`. CFR не хранит стратегию «в узле как таблицу строк» — он хранит regrets и кумулятивы, а стратегию вычисляет.

---

## Интерфейс `Trainable`

| Метод | Зачем |
|-------|--------|
| `getAverageStrategy()` | усреднённая стратегия (то, что показывают как результат) |
| `getcurrentStrategy()` | текущая стратегия из instantaneous regrets |
| `updateRegrets(regrets, iteration, reach_probs)` | шаг обучения |
| `setEv(evs)` | сохранить EV по (hand, action) для UI |
| `copyStrategy(other)` | копирование (warmup / изоморфизмы) |
| `dump_strategy(with_state)` | JSON кусок стратегии узла |
| `dump_evs()` | JSON EV |
| `get_type()` | `CFR_PLUS_TRAINABLE` или `DISCOUNTED_CFR_TRAINABLE` |

Размерность векторов обычно: `action_number * card_number` (flat), где `card_number` = число рук в range игрока.

---

## `DiscountedCfrTrainable` (основной, float32)

Параметры DCFR (классика):
- `alpha = 1.5` — discount положительных regrets
- `beta = 0.5` — discount отрицательных
- `gamma = 2` — discount для average strategy accumulators
- `theta = 0.9`

### Поля

| Поле | Смысл |
|------|--------|
| `r_plus` | текущие (положительные) regrets |
| `r_plus_sum` | сумма для нормализации current strategy |
| `cum_r_plus` | кумулятив для average strategy |
| `evs` | EV для отображения |
| `privateCards` | указатель на range игрока (для dump по именам рук) |
| `action_node` | ссылка на узел (список действий) |

### Методы

**Конструктор `(privateCards, actionNode)`** — аллоцирует векторы нужного размера.

**`isAllZeros`** — проверка «пустой» стратегии.

**`getcurrentStrategy` / `getcurrentStrategyNoCache`** — regret matching: для каждой руки нормализует `max(r_plus,0)`; если сумма 0 — равномерно по действиям.

**`getAverageStrategy`** — то же по `cum_r_plus`.

**`updateRegrets`** — применяет discount-формулы DCFR и добавляет новые regrets × reach; обновляет кумулятив average.

**`setEv`** — копирует EV-вектор.

**`copyStrategy`** — копирует внутренние массивы с другого trainable того же типа.

**`dump_strategy` / `dump_evs`** — сериализация в JSON (ключи — руки/действия).

**`get_type`** — `DISCOUNTED_CFR_TRAINABLE`.

---

## `DiscountedCfrTrainableHF` (half float)

Та же логика DCFR, но хранит regrets/кумулятивы в `half` (16-bit) для экономии RAM на river. API тот же, что у float-версии.

Выбирается в `ActionNode::getTrainable` при `use_halffloats == 2` и round == RIVER.

---

## `DiscountedCfrTrainableSF` («semi» / смешанный)

Промежуточный режим (`use_halffloats == 1`): часть данных в уменьшенной точности (см. реализацию в `.cpp` — обычно компромисс precision/RAM). Тот же интерфейс Trainable.

---

## `CfrPlusTrainable`

Классический CFR+ (без полного discounted набора параметров DCFR). Поля: `regrets`, accumulators average strategy.

Методы зеркалят интерфейс `Trainable`. В GUI-пути по умолчанию создаётся **Discounted\***, но код CFR+ оставлен для выбора trainer-строкой / legacy.

---

## `Trainable.cpp`

Минимальные/общие куски, если есть; основная логика в классах выше.

---

## Как это стыкуется с узлом

```
ActionNode::getTrainable(deal_index, true, use_halffloats)
  → если trainables[deal] == null:
       river + HF → DiscountedCfrTrainableHF
       river + SF → DiscountedCfrTrainableSF
       иначе      → DiscountedCfrTrainable
PCfrSolver::actionUtility
  → strategy = trainable->getcurrentStrategy()
  → ... recurse ...
  → trainable->updateRegrets(...)
UI / dump
  → trainable->getAverageStrategy() / dump_strategy()
```

Память солвера ≈ число ActionNode × число deal-путей × (руки × действия) × sizeof(float или half) × несколько массивов (r_plus, cum, …). Поэтому estimate memory в UI важен.
