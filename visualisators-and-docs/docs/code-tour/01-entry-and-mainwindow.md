# 01 — Точка входа и главное окно

Файлы: `main.cpp`, `mainwindow.h` / `mainwindow.cpp`, плюс локальные хелперы в `mainwindow.cpp`.

---

## `main.cpp`

### `myMessageOutput(QtMsgType, QMessageLogContext, QString)`

**Зачем:** перехватчик `qDebug` / warning / fatal, чтобы логи шли в окно приложения, а не только в stderr.

**Что делает:**
- Пока `MainWindow::s_textEdit == 0` (окно ещё не создано) — пишет в `stderr`.
- После создания окна — вызывает `s_textEdit->log_with_signal(msg)`.
- На `QtFatalMsg` вызывает `abort()`.

**Кто вызывает:** Qt через `qInstallMessageHandler` в `main`.

### `main(int argc, char* argv[])`

**Зачем:** старт GUI-приложения.

**Что делает:**
1. Ставит message handler.
2. Создаёт `QApplication`.
3. Читает `QSettings("TexasSolver","Setting")` → язык; если пусто — диалог EN/CN и сохранение.
4. Грузит `:/lang_en.qm` или `:/lang_cn.qm`.
5. Если `dump_round == 0`, ставит default `2` (глубина дампа стратегии).
6. Создаёт `MainWindow`, `show()`, `a.exec()`.

---

## `MainWindow` (`mainwindow.h` / `.cpp`)

Центральный Qt-экран: параметры спота, кнопки build/solve/stop, превью ranges, список файлов `parameters/`.

### Поля (зачем нужны)

| Поле | Роль |
|------|------|
| `s_textEdit` | static-указатель на лог для message handler |
| `qSolverJob` | фоновый поток с двумя `PokerSolver` |
| `qFileSystemModel` | дерево файлов в папке `parameters/` |
| `strategyExplorer` | окно просмотра стратегии после solve |
| `rangeSelector` / `boardSelector` / `settingEditor` | модальные диалоги |
| `ip_model` / `oop_model` + delegates | мини-матрицы ranges в главном окне |

### `MainWindow(QWidget* parent)`

**Зачем:** сборка UI и старт загрузки компараторов.

**Что делает:**
- `setupUi`, привязывает меню (json/import/export/settings/clear).
- Создаёт `QSolverJob`, `setContext(log)`, миссия `LOADING`, `start()`.
- Настраивает `QFileSystemModel` на `parameters/*.txt` (на macOS root path особый).
- Создаёт thumbnail-таблицы IP/OOP range (`RangeSelectorTableModel` + Delegate).
- Скрывает `tabWidget` (заготовка под доп. вкладки).

### `~MainWindow()`

Удаляет job, model, delegates, `ui`.

### `get_logwindow()` / `getLogArea()` / `getPriUi()`

Геттеры: виджет лога и сгенерированный `Ui::MainWindow*`.

### `on_actionjson_triggered()`

Диалог «сохранить strategy.json» → `qSolverJob->savefile`, миссия `SAVING`, `start()`.

### `getParams(QString input, QString key)` (свободная функция в cpp)

Если строка содержит `key`, возвращает хвост без ключа; иначе `"INVALID"`. Парсер строк parameter-файлов.

### `on_actionclear_all_triggered()` / `clear_all_params()`

Обнуляет все поля спота (pot, stack, board, ranges, bet sizes, solver options).

### `import_from_file(QString fileName)`

Читает txt-скрипт команд (`set_pot`, `set_board`, `set_bet_sizes ...`, `set_accuracy`, …) и заполняет виджеты. Формат тот же, что у CLI/FFI.

### `on_actionimport_triggered()`

Диалог выбора `.txt` → `import_from_file` + refresh таблиц ranges.

### `on_actionexport_triggered()` / `on_exportCurrentParameterButton_clicked()`

Собирает текущие поля UI в тот же текстовый формат команд (включая `build_tree`, `start_solve`, `dump_result`) и пишет файл. Нужен, чтобы повторить спот в консоли/FFI.

### `on_actionSettings_triggered()`

Открывает `SettingEditor` (язык, dump_round и т.п.).

### `on_ip_range(QString)`

Слот для записи текста IP range (используется селектором).

### `on_buttomSolve_clicked()`

Читает iteration / accuracy / print_interval / threads из UI → миссия `SOLVING` → `start()`.

**Важно:** дерево и ranges уже должны быть заданы через Build Tree; solve только запускает CFR.

### `on_clearLogButtom_clicked()`

Очищает лог.

### `sizes_convert(QString input)` (свободная)

Парсит строку размеров бетов: `"50 100"` или `"0.5x 1x"` (суффикс `x` → умножение на 100, т.е. % pot). Возвращает `vector<float>`.

### `on_buildTreeButtom_clicked()`

**Зачем:** главная кнопка «построить дерево».

**Что делает:**
1. Копирует ranges/board из UI в `qSolverJob`.
2. По числу карт борда ставит `current_round` (3→flop/1, 4→turn/2, 5→river/3).
3. Считает commits из pot/2, stack = effective + commit.
4. Mode holdem/shortdeck, isomorphism, half-floats.
5. Собирает 6× `StreetSetting` (flop/turn/river × ip/oop) → `GameTreeBuildingSettings`.
6. Миссия `BUILDTREE`, `start()`.

### `on_copyButtom_clicked()`

Копирует bet/raise/allin IP → OOP на всех стритах (удобно выровнять сайзинги).

### `on_showResultButton_clicked()`

Открывает `StrategyExplorer(this, qSolverJob)` для навигации по дереву и стратегиям.

### `on_stopSolvingButton_clicked()`

`qSolverJob->stop()` → пробрасывается в `PCfrSolver::stop()`.

### `on_ipRangeSelectButtom_clicked()` / `on_oopRangeSelectButtom_clicked()`

Открывает `RangeSelector` привязанный к соответствующему `QTextEdit`.

### `iso_corh(QString board)` (свободная)

Грубая оценка коэффициента экономии памяти от suit-isomorphism по борду (перемножает 0.70 за совпадающие suit-hash). Используется только в оценке памяти.

### `on_estimateMemoryButtom_clicked()`

Вызывает `qSolverJob->estimate_tree_memory`, умножает на `iso_corh` и коэффициент half-floats (1 / 0.75 / 0.5), показывает Mb/Gb.

### `on_selectBoardButton_clicked()`

Открывает `boardselector`.

### `on_openParametersFolderButton_clicked()`

Открывает папку `parameters/` в файловом менеджере.

### `item_clicked(QModelIndex)`

Клик по `.txt` в дереве параметров → `import_from_file`.

### `on_ipRangeText_textChanged()` / `on_oopRangeText_textChanged()`

Синхронизирует текстовый range с thumbnail-моделью матрицы.

### `onExpanded(QModelIndex)`

Подгоняет ширину колонки в `parametersTreeView`.

---

## Связанные корневые виджеты (кратко)

Полный разбор — в [08-ui-views.md](08-ui-views.md). Здесь только роль относительно MainWindow:

| Файл | Роль |
|------|------|
| `qstextedit.*` | лог с signal-safe записью из других потоков |
| `qstreeview.*` | дерево навигации в StrategyExplorer |
| `rangeselector.*` | визуальный выбор 13×13 ranges |
| `boardselector.*` | выбор карт борда |
| `settingeditor.*` | глобальные настройки |
| `strategyexplorer.*` | просмотр результата solve |
| `htmltableview.*` | HTML-отрисовка ячеек стратегии |
