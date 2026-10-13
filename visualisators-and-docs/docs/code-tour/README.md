# Разбор кодовой базы TexasSolver (домен)

Документация по **рабочим** исходникам солвера: зачем файл существует и что делает каждая важная функция.

Читать по порядку — так проще понять поток данных от GUI до CFR.

## Что сюда входит

| Документ | Содержание |
|----------|------------|
| [01-entry-and-mainwindow.md](01-entry-and-mainwindow.md) | `main.cpp`, `MainWindow`, хелперы UI входа |
| [02-poker-solver-runtime.md](02-poker-solver-runtime.md) | `QSolverJob`, `PokerSolver` |
| [03-game-tree-and-nodes.md](03-game-tree-and-nodes.md) | `GameTree`, узлы дерева, `Rule` / bet settings |
| [04-cfr-solver.md](04-cfr-solver.md) | `Solver`, `PCfrSolver`, `BestResponse`, experimental |
| [05-trainable.md](05-trainable.md) | `Trainable` и варианты CFR storage |
| [06-cards-ranges.md](06-cards-ranges.md) | `Card`, `Deck`, ranges, `library` |
| [07-compairer.md](07-compairer.md) | Оценка силы рук |
| [08-ui-views.md](08-ui-views.md) | StrategyExplorer, selectors, models/delegates |
| [09-tools-cli-ffi-api.md](09-tools-cli-ffi-api.md) | CLI, FFI, pybind, новый API layer |

## Что намеренно не разбирается построчно

- `include/json.hpp` — сторонняя JSON-библиотека
- `include/tools/half-*.h` — half-float arithmetic
- `include/tools/tinyformat.h`, `argparse.hpp` — утилиты форматирования/CLI
- `include/**` shim-заголовки вида `#include "src/..."` — только реэкспорт путей
- `.ui` / `.qrc` / переводы `.ts` — Qt-ресурсы, не логика солвера

## Карта потока

```
main() → MainWindow
           ↓
        QSolverJob (QThread: LOADING | BUILDTREE | SOLVING | SAVING)
           ↓
        PokerSolver (holdem / shortdeck)
           ├─ Deck + Dic5Compairer
           ├─ GameTree (Action / Chance / Showdown / Terminal)
           └─ PCfrSolver
                ├─ Trainable на ActionNode
                ├─ BestResponse (exploitability)
                └─ RiverRangeManager / PrivateCardsManager
```

## Соглашения в документах

Для каждой функции обычно:

1. **Сигнатура** (упрощённо)
2. **Зачем** — роль в системе
3. **Что делает** — шаги
4. **Кто вызывает** — если неочевидно

Публичные методы разобраны полностью; крошечные геттеры/сеттеры — кратко.

## Индекс файлов → документ

| Путь | Документ |
|------|----------|
| `main.cpp`, `mainwindow.*` | 01 |
| `src/runtime/*` | 02 |
| `src/GameTree.*`, `src/nodes/*`, `src/tools/{Rule,StreetSetting,GameTreeBuildingSettings}.*` | 03 |
| `src/solver/*`, `src/experimental/*` | 04 |
| `src/trainable/*` | 05 |
| `src/Card.*`, `src/Deck.*`, `src/library.*`, `src/ranges/*`, `src/tools/PrivateRangeConverter.*` | 06 |
| `src/compairer/*` | 07 |
| `strategyexplorer.*`, `rangeselector.*`, `boardselector.*`, `settingeditor.*`, `qs*.*`, `htmltableview.*`, `src/ui/*` | 08 |
| `src/tools/CommandLineTool.*`, `progressbar.*`, `lookup8.*`, `utils.*`, `qdebugstream.h`, `src/console.cpp`, `src/api.cpp`, `src/pybind/*`, `include/api/*` | 09 |

## Как пользоваться

1. Прочитай этот README и схему потока.
2. Пройди 01 → 04, если цель — понять solve.
3. 05–07 — данные и память.
4. 08–09 — UI и внешние входы / будущий API.
