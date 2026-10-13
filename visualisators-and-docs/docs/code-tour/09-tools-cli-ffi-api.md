# 09 — Tools, CLI, FFI, API layer

Файлы:
- `src/tools/*` (кроме вендорных half/tinyformat/argparse в `include/tools`)
- `src/console.cpp`
- `src/api.cpp`
- `src/pybind/bindSolver.cpp`
- `include/api/solver_dtos.h`, `include/api/isolver_backend.h`

---

## CommandLineTool

Консольный / скриптовый аналог MainWindow+QSolverJob **без Qt UI**.

### Конструктор `(mode, resource_dir)`

Создаёт один `PokerSolver` (holdem или shortdeck) и пустые `GameTreeBuildingSettings` (сайзинги заполняются командами).

### `startWorking()`

Читает stdin построчно → `processCommand`.

### `execFromFile(input_file)`

То же из файла (путь GUI export / FFI sample).

### `processCommand(input)`

Парсер команд (формат как в parameter txt):

| Команда | Эффект |
|---------|--------|
| `set_pot` | ip/oop commit = pot/2 |
| `set_effective_stack` | stack = value + commit |
| `set_board` | board + current_round по числу карт |
| `set_range_ip` / `set_range_oop` | ranges |
| `set_bet_sizes` | парсит `player,round,type,sizes...` или allin → правит `gtbs` |
| `set_allin_threshold` / `set_raise_limit` | параметры дерева |
| `build_tree` | `ps.build_game_tree(...)` |
| `set_thread_num` / `set_accuracy` / `set_max_iteration` / `set_print_interval` / `set_use_isomorphism` | параметры solve |
| `start_solve` | `ps.train(...)` |
| `set_dump_rounds` | глубина dump |
| `dump_result` | `ps.dump_strategy(path, dump_rounds)` |

Это **единый язык конфигурации** для GUI export, CLI и FFI.

---

## `console.cpp`

Альтернативная точка входа консольной сборки (`main_backup` / argparse). Запускает `CommandLineTool` с аргументами mode/resources/input. Точное имя `main` зависит от CMake target (GUI vs console).

---

## `api.cpp` — C FFI

```cpp
EXPORT int api(const char* input_file,
               const char* resource_dir = "./resources",
               const char* mode = "holdem");
```

**Зачем:** вызов из Python/Rust/Dart через `ctypes` / FFI без Qt GUI.

**Что делает:**
- Валидирует mode.
- Если `input_file` пуст — интерактивный `startWorking()`.
- Иначе `execFromFile(input_file)`.

Собирается как `.dll` / `.so` / `.dylib`. Документация: `resources/ffi_api/README.MD`.

---

## `bindSolver.cpp` (pybind)

Заготовка/обвязка Python bindings вокруг `PokerSolver`. Файл может быть тонким в текущем дереве — смотрите CMake / pybind target. Идея: экспортировать класс солвера нативно, а не только через текстовый `api()`.

---

## Прочие tools

### `progressbar.*`

Текстовый progress bar при загрузке большого dic-файла компаратора.

### `lookup8.*`

Хеш-функции (lookup8) — вспомогательные для быстрых ключей/таблиц.

### `qdebugstream.h`

Редирект `std::ostream` / qDebug в `QTextEdit` (исторический путь логирования).

### `utils.h` — `exchange_color(...)`

Переставляет значения в векторе (стратегия/EV) при обмене двух мастей в range — поддержка suit isomorphism при dump/UI.

### `PrivateRangeConverter` — см. [06-cards-ranges.md](06-cards-ranges.md)

### `Rule` / `StreetSetting` / `GameTreeBuildingSettings` — см. [03-game-tree-and-nodes.md](03-game-tree-and-nodes.md)

---

## Новый API layer (`include/api`) — заготовка

Это **ещё не полный backend**, а черновик чистого интерфейса над легаси.

### `solver_dtos.h`

DTO без Qt:

| Тип | Назначение |
|-----|------------|
| `SolverMode` | Holdem / Shortdeck |
| `NodeType` / `GameRoundDto` | зеркала узлов/улиц |
| `StreetSettingDto` | bet/raise/donk/allin |
| `TreeBuildSettingsDto` | 6 street settings |
| `SolveRequest` | полный запрос на build+solve (поля как у `QSolverJob`) |
| `TreeNodeDto` | узел для UI/API навигации |
| `ActionLabelDto` | подпись действия |
| `StrategyGridDto` | сетка strategy/evs/ranges для отображения |
| `ProgressEvent` | iteration + exploitability + message |

> В файле есть опечатка `std:vector` (одно двоеточие) в `StreetSettingDto::raise_sizes` — пока код не компилируется как есть, пока не поправят.

### `isolver_backend.h`

```cpp
class ISolverBackend {
public:
  virtual ~ISolverBackend() = default;
};
```

Пустой интерфейс-заготовка. Ожидаемый следующий шаг проекта: методы вроде `buildTree(SolveRequest)`, `solve()`, `stop()`, `getStrategy(nodeId)`, сигналы `ProgressEvent` — и адаптер, который внутри зовёт `PokerSolver`/`PCfrSolver`.

---

## Как слои стыкуются сегодня vs куда идут

**Сейчас:**
```
GUI  → QSolverJob → PokerSolver → PCfrSolver
CLI  → CommandLineTool → PokerSolver → …
FFI  → api() → CommandLineTool → …
```

**Черновик API:**
```
SolveRequest / StrategyGridDto
        ↓
  ISolverBackend  (пока пустой)
        ↓
  PokerSolver (легаси ядро)
```

Имеет смысл не ломать `PokerSolver`, а обернуть его адаптером, когда будете рефакторить UI/сервисный слой.
