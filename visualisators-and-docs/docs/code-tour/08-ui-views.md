# 08 — UI просмотра и селекторы

Файлы в корне: `strategyexplorer.*`, `rangeselector.*`, `boardselector.*`, `settingeditor.*`, `qstextedit.*`, `qstreeview.*`, `htmltableview.*`

Файлы в `src/ui/`: models + delegates для таблиц/дерева.

---

## StrategyExplorer

Окно результата после Solve: дерево линий + матрица стратегий + detail/rough панели.

### Конструктор / деструктор

Создаёт модели (`TreeModel`, `TableStrategyModel`, detail/rough viewers + delegates), таймер обновления, связывает клики/selection.

### Навигация

| Метод | Роль |
|-------|------|
| `item_clicked` / `item_expanded` | клик/раскрытие узла в дереве |
| `selection_changed` | смена выделенного узла → обновить стратегии |
| `process_treeclick(TreeItem*)` | достать `GameTreeNode` из TreeItem, обновить table strategy |
| `process_board(TreeItem*)` | согласовать turn/river combo-боксы с путём в дереве |
| `on_turnCardBox_currentIndexChanged` / `on_riverCardBox_...` | фильтр стратегий по выбранным картам |
| `update_second` | периодический refresh (таймер) |
| `onMouseMoveEvent(i,j)` | hover по ячейке 13×13 → detail viewer |

### Режимы отображения

| Кнопка/слот | Режим |
|-------------|--------|
| `on_strategyModeButtom_clicked` | частоты действий |
| `on_evModeButtom_clicked` | EV + стратегия |
| `on_evOnlyModeButtom_clicked` | только EV |
| `on_ipRangeButtom_clicked` / `on_oopRangeButtom_clicked` | показать веса range игрока |

Режим хранится в `DetailWindowSetting`.

---

## TreeItem / TreeModel

### `TreeItem`

Qt-обёртка над `weak_ptr<GameTreeNode>` для `QTreeView`.

| Метод | Роль |
|-------|------|
| конструктор | связать с узлом + parent |
| `insertChild` / `removeChild` / `child` / `childCount` | дерево UI |
| `data()` | текст узла (round + action label) |
| `get_round_str` / `get_game_action_str` | форматирование |
| `row` / `parentItem` / `setParentItem` | индексы Qt model |

### `TreeModel` (`QAbstractItemModel`)

| Метод | Роль |
|-------|------|
| конструктор | `setupModelData` из `qSolverJob->get_solver()->get_game_tree()` |
| стандартные `index/parent/rowCount/columnCount/data/flags/headerData` | Qt API |
| `reGenerateTreeItem(round, node)` | lazy-достройка children при expand (дерево большое) |
| `clicked_event` | слот клика |

---

## TableStrategyModel

Модель сетки 13×13 (ранг×ранг) со стратегиями текущей руки/узла.

| Метод | Роль |
|-------|------|
| `setGameTreeNode(TreeItem*)` | выбрать узел |
| `setTrunCard` / `setRiverCard` | зафиксировать chance-карты |
| `updateStrategyData` | вызвать `solver->get_strategy` / `get_evs`, заполнить `current_strategy`, `current_evs`, ranges |
| `get_strategy(i,j)` | список (action, freq) для ячейки |
| `get_total_strategy` | агрегация по всему range |
| `get_ev_grid` / `get_strategies_evs` | EV данные |
| Qt model API (`data`, `index`, …) | отрисовка |
| `clicked_event` | клик по ячейке |

Внутренние таблицы `ui_strategy_table` мапят UI-координаты (suited/offsuit/pair) → card ids.

---

## Detail / Rough viewers

| Класс | Роль |
|-------|------|
| `DetailViewerModel` + `DetailItemDelegate` | детальная таблица по конкретной клетке (все действия, EV) |
| `RoughStrategyViewerModel` + `RoughStrategyItemDelegate` | укрупнённый обзор стратегии |
| `DetailWindowSetting` | enum/флаги режима (strategy/ev/range) |
| `StrategyItemDelegate` | отрисовка цветных баров частот в 13×13 |
| `WordItemDelegate` | текстовые ячейки |

Delegates в Qt: рисуют ячейку (`paint`), не хранят данные солвера.

---

## RangeSelector

Диалог выбора диапазона (матрица + слайдер веса + import/export файлов ranges).

| Метод | Роль |
|-------|------|
| конструктор `(rangeEdit, parent, mode)` | привязка к QTextEdit MainWindow; holdem/shortdeck ranks |
| `confirm_text` signal | отдать строку range наружу |
| `on_confirmButtom_clicked` / `on_cancelButton_clicked` | OK/Cancel |
| `grid_pressed` / `grid_area` / `grid_release` | paint selection мышью |
| `on_rangeNumberSlider_valueChanged` / `on_rangeNumberEdit_textEdited` | вес 0..1 (через max_val=1000) |
| `on_clearRangeButtom_clicked` | очистить |
| `on_textEdit_textChanged` | sync текст ↔ матрица |
| `import_range` / `on_importRangeButton` / `on_exportRangeButton` | файлы ranges |
| `item_clicked` / `onExpanded` | дерево пресетов |
| `update_second` | таймер UI |
| `on_openRangeFolderButton_clicked` | открыть папку ranges |

### `RangeSelectorTableModel` / `Delegate`

Данные и отрисовка 13×13 весов. `setRangeText` парсит строку range. Используется и в MainWindow thumbnail, и в диалоге.

---

## boardselector

| Метод | Роль |
|-------|------|
| конструктор `(boardEdit, mode)` | привязка к полю борда |
| `on_boardSelectorTable_clicked` | toggle карты |
| `on_boardEdit_textChanged/Edited` | sync текст |
| `on_confirmButton_clicked` / `cancel` / `clearBoard` | закрытие/очистка |

### `BoardSelectorTableModel` / `Delegate`

Сетка доступных карт колоды; выбранные подсвечены.

---

## SettingEditor

| Метод | Роль |
|-------|------|
| конструктор | читает `QSettings` (language, dump_round, …) |
| `on_languageBox_currentIndexChanged` | смена языка (с флагом `initized`, чтобы не писать при load) |
| `on_confirmBox_accepted` | сохранить settings |

---

## Вспомогательные виджеты корня

### `QSTextEdit`

Лог с `log_with_signal` — безопасная запись из worker-thread через Qt signal/slot.

### `QSTreeView`

Кастомизация tree view под game tree (expand/click hooks).

### `HtmlTableView` / `HtmlTableRangeView`

Таблица с HTML в ячейках (цветные карты, strategy bars). `itemMouseChange` — hover для StrategyExplorer.

### `DropTextEdit` (`src/ui/droptextedit.*`)

TextEdit с drag&drop файлов параметров/ranges.

---

## Как UI читает результат солвера

```
StrategyExplorer
  → TreeModel (структура GameTree)
  → клик по TreeItem
  → TableStrategyModel::updateStrategyData
       → PokerSolver::get_solver()  // PCfrSolver
       → get_strategy(actionNode, {turn,river})
       → get_evs(...)
  → StrategyItemDelegate рисует частоты
```
