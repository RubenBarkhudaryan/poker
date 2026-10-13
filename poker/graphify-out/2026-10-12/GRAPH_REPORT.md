# Graph Report - poker  (2026-10-12)

## Corpus Check
- 203 files · ~3,655,567 words
- Verdict: corpus is large enough that graph structure adds value.

## Summary
- 2657 nodes · 5754 edges · 128 communities (96 shown, 5 thin omitted)
- Extraction: 96% EXTRACTED · 4% INFERRED · 0% AMBIGUOUS · INFERRED: 224 edges (avg confidence: 0.84)
- Token cost: 0 input · 0 output

## Graph Freshness
- Built from commit: `1829f48b`
- Run `git rev-parse HEAD` and compare to check if the graph is stale.
- Run `graphify update .` after code changes (no API cost).

## Community Hubs (Navigation)
- json.hpp
- half
- .create
- TreeBuilder.py
- tinyformat.h
- String
- rbegin
- RangeSelectorTableModel
- BoardSelectorTableModel
- MainWindow
- functions
- TableStrategyModel
- sax_parse_internal
- QSolverJob
- half-1-12-0.h
- runtime_error
- lexer
- T
- GameTreeNode
- string_t
- RangeSelector
- CommandLineTool
- PCfrSolver
- from_json
- DetailWindowSetting
- StrategyExplorer
- Card
- DiscountedCfrTrainableHF
- PCfrSolver.cpp
- PokerSolver
- TCfrSolver
- DiscountedCfrTrainable
- DiscountedCfrTrainableSF
- RiverRangeManager
- Rule
- primitive_iterator_t
- wide_string_input_adapter
- BestResponse
- Dic5Compairer
- TreeModel
- CfrPlusTrainable
- mainwindow.h
- diyfp
- GameTreeBuildingSettings
- vector
- input_adapter
- boardselector
- PrivateCards
- serializer
- string
- .size
- size_t
- ActionNode
- LocalBackend
- DetailViewerModel
- htmltableview.cpp
- .tgamma
- RoughStrategyViewerModel
- Combinations
- ActionLabelDto
- GameActions
- iteration_proxy_value
- local_backend.cpp
- operator+=
- HtmlTableRangeView
- ShowdownNode
- Solver
- QDebugStream
- Trainable
- QSTextEdit
- HtmlTableView
- TreeItem
- json_sax
- worditemdelegate.cpp
- TerminalNode
- SettingEditor
- handle_value
- RiverCombs
- DropTextEdit
- json_ref
- htmltableview.h
- Deck
- SolveRequest
- .cast_impl
- parser
- main.cpp
- lookup8.cpp
- .fmax
- .operator()
- build-AppImage.sh
- pre-commit
- replace.sh
- riversolver.sh
- README.md
- ChanceNode
- ISolverBackend
- ArgumentParser
- argparse.hpp
- main_backup
- StrategyGridDto
- Usage

## God Nodes (most connected - your core abstractions)
1. `half` - 120 edges
2. `expr` - 117 edges
3. `string` - 111 edges
4. `functions` - 68 edges
5. `MainWindow` - 63 edges
6. `TableStrategyModel` - 62 edges
7. `GameTreeNode` - 60 edges
8. `PCfrSolver` - 59 edges
9. `Card` - 55 edges
10. `PrivateCards` - 48 edges

## Surprising Connections (you probably didn't know these)
- `StrategyExplorer::StrategyExplorer()` --calls--> `itemMouseChange`  [INFERRED]
  strategyexplorer.cpp → htmltableview.h
- `main()` --references--> `MainWindow`  [INFERRED]
  main.cpp → mainwindow.h
- `LocalBackend::getStrategyAt()` --calls--> `activeSolver`  [INFERRED]
  src/api/local_backend.cpp → include/api/local_backend.h
- `LocalBackend::hasGameTree()` --calls--> `activeSolver`  [INFERRED]
  src/api/local_backend.cpp → include/api/local_backend.h
- `LocalBackend::hasSolver()` --calls--> `activeSolver`  [INFERRED]
  src/api/local_backend.cpp → include/api/local_backend.h

## Import Cycles
- None detected.

## Communities (128 total, 5 thin omitted)

### Community 0 - "json.hpp"
Cohesion: 0.02
Nodes (114): array_t, B1, boolean_t, CompatibleNumberIntegerType, CompatibleType, conditional<bool(B1::value), conjunction<Bn...>, B1>::type, ConstructibleArrayType, ConstructibleObjectType (+106 more)

### Community 1 - "half"
Cohesion: 0.05
Nodes (54): float_round_style, abs(), acos(), acosh(), asin(), asinh(), atan(), atan2() (+46 more)

### Community 2 - ".create"
Cohesion: 0.12
Nodes (47): const_iterator, const_reference, at(), back(), basic_json, BasicJsonType* handle_value(Value&& v)(), begin(), cbegin() (+39 more)

### Community 3 - "TreeBuilder.py"
Cohesion: 0.06
Nodes (16): ActionNode, BoardcardChanceNode, ChanceNode, check_number_this_round(), DealCardNode, FiveCardTexasTreeBuilder, HolecardChanceNode, Node (+8 more)

### Community 4 - "tinyformat.h"
Cohesion: 0.06
Nodes (44): FormatListRef, convertToInt, convertToInt<T,true>, fail, dummy, format(), FormatArg, m_value (+36 more)

### Community 5 - "String"
Cohesion: 0.17
Nodes (7): Argument, fixed, name, optional, short_name, String, StringVector

### Community 6 - "rbegin"
Cohesion: 0.53
Nodes (6): const_reverse_iterator, crbegin(), crend(), rbegin(), rend(), reverse_iterator

### Community 7 - "RangeSelectorTableModel"
Cohesion: 0.06
Nodes (46): QModelIndex, QObject, QPainter, QStringList, QStyleOptionViewItem, Q_OBJECT, QStringList, RangeSelectorTableDelegate (+38 more)

### Community 8 - "BoardSelectorTableModel"
Cohesion: 0.06
Nodes (45): BoardSelectorTableDelegate, BoardSelectorTableDelegate::BoardSelectorTableDelegate(), boardSelectorTableModel, paint, public, rank_list, QModelIndex, QObject (+37 more)

### Community 9 - "MainWindow"
Cohesion: 0.05
Nodes (51): QModelIndex, QString, QWidget, vector, getParams(), Q_OBJECT, iso_corh(), MainWindow (+43 more)

### Community 10 - "functions"
Cohesion: 0.06
Nodes (14): HALF_CONSTEXPR, binary_t, functions, HALF_CONSTEXPR half_float::half lowest(), half_float::half denorm_min(), half_float::half epsilon(), half_float::half infinity(), half_float::half max() (+6 more)

### Community 11 - "TableStrategyModel"
Cohesion: 0.07
Nodes (44): Orientation, pair, QModelIndex, QVariant, vector, map, pair, Q_OBJECT (+36 more)

### Community 12 - "sax_parse_internal"
Cohesion: 0.27
Nodes (9): get_bson_string(), get_number(), get_string(), json_sax_acceptor, sax_parse(), sax_parse_internal(), input_format_t, NumberType (+1 more)

### Community 13 - "QSolverJob"
Cohesion: 0.06
Nodes (41): MissionType, QString, Mode, Q_OBJECT, QString, shared_ptr, QSolverJob, accuracy (+33 more)

### Community 14 - "half-1-12-0.h"
Cohesion: 0.06
Nodes (39): conditional<B,T,F>, digits, binary_specialized<half,half>, bits, bits<const T>, bits<const volatile T>, bits<double>, bits<float> (+31 more)

### Community 15 - "runtime_error"
Cohesion: 0.16
Nodes (34): BetType, runtime_error, ActionNode, ChanceNode, GameRound, json, shared_ptr, ShowdownNode (+26 more)

### Community 16 - "lexer"
Cohesion: 0.08
Nodes (18): lexer, current, decimal_point_char, error_message, ia, JSON_HEDLEY_CONST, next_unget, position (+10 more)

### Community 17 - "T"
Cohesion: 0.09
Nodes (29): builtin_isnan(), builtin_signbit(), fabs(), float2half(), float2half_impl(), fmod(), frexp(), false_type (+21 more)

### Community 18 - "GameTreeNode"
Cohesion: 0.16
Nodes (19): GameRound, shared_ptr, GameTreeNode, depth, GameTreeNode::GameTreeNode(), getParent, getPot, getRound (+11 more)

### Community 19 - "string_t"
Cohesion: 0.09
Nodes (26): CharType, binary_writer, is_little_endian, oa, basic_ostream, vector, output_adapter, oa (+18 more)

### Community 20 - "RangeSelector"
Cohesion: 0.09
Nodes (35): QModelIndex, QString, Mode, Q_OBJECT, QDialog, QFileSystemModel, QStringList, QTextEdit (+27 more)

### Community 21 - "CommandLineTool"
Cohesion: 0.08
Nodes (24): CommandLineTool, accuracy, allin_threshold, big_blind, board, current_round, dump_rounds, gtbs (+16 more)

### Community 22 - "PCfrSolver"
Cohesion: 0.06
Nodes (36): GameRound, MonteCarolAlg, shared_ptr, vector, PCfrSolver, accuracy, collecting_statics, color_iso_offset (+28 more)

### Community 23 - "from_json"
Cohesion: 0.09
Nodes (36): Allocator, ArithmeticType, BasicJsonType, Compare, CompatibleArrayType, CompatibleNumberUnsignedType, CompatibleObjectType, CompatibleString (+28 more)

### Community 24 - "DetailWindowSetting"
Cohesion: 0.10
Nodes (16): QStringList, DetailWindowMode, QString, map, QStyledItemDelegate, ActionNode, QAbstractItemModel, QObject (+8 more)

### Community 25 - "StrategyExplorer"
Cohesion: 0.08
Nodes (35): QItemSelection, QModelIndex, QWidget, Q_OBJECT, QDialog, QTimer, vector, StrategyExplorer (+27 more)

### Community 26 - "Card"
Cohesion: 0.15
Nodes (28): Card, boardCard2long, boardCards2html, boardCards2long, boardInt2long, boardInts2long, card, card2int (+20 more)

### Community 27 - "DiscountedCfrTrainableHF"
Cohesion: 0.09
Nodes (32): ActionNode, json, shared_ptr, TrainableType, vector, DiscountedCfrTrainableHF, action_number, alpha (+24 more)

### Community 28 - "PCfrSolver.cpp"
Cohesion: 0.15
Nodes (29): random(), timeSinceEpochMillisec(), gameRound2int, toString, ActionNode, ChanceNode, json, MonteCarolAlg (+21 more)

### Community 29 - "PokerSolver"
Cohesion: 0.08
Nodes (27): toRoundDto, LocalBackend::getStrategyAt(), Dic5Compairer::Dic5Compairer(), vector, string_split(), QString, vector, shared_ptr (+19 more)

### Community 30 - "TCfrSolver"
Cohesion: 0.06
Nodes (31): MonteCarolAlg, shared_ptr, vector, TCfrSolver, actionUtility, cfr, chanceUtility, compairer (+23 more)

### Community 31 - "DiscountedCfrTrainable"
Cohesion: 0.11
Nodes (30): ActionNode, json, shared_ptr, TrainableType, vector, DiscountedCfrTrainable, action_number, alpha (+22 more)

### Community 32 - "DiscountedCfrTrainableSF"
Cohesion: 0.10
Nodes (30): ActionNode, json, TrainableType, vector, DiscountedCfrTrainableSF, action_number, alpha, beta (+22 more)

### Community 33 - "RiverRangeManager"
Cohesion: 0.09
Nodes (16): mutex, Compairer, compair, dic_dir, get_rank, lines, shared_ptr, shared_ptr (+8 more)

### Community 34 - "Rule"
Cohesion: 0.10
Nodes (19): ifstream, GameTree::GameTree(), readAllBytes, vector, Rule, allin_threshold, big_blind, build_settings (+11 more)

### Community 35 - "primitive_iterator_t"
Cohesion: 0.10
Nodes (16): base_iterator, difference_type, internal_iterator, array_iterator, object_iterator, primitive_iterator, iter_impl, m_it (+8 more)

### Community 36 - "wide_string_input_adapter"
Cohesion: 0.09
Nodes (17): FILE, file_input_adapter, m_file, int_type, input_adapter_protocol, get_character, input_buffer_adapter, cursor (+9 more)

### Community 37 - "BestResponse"
Cohesion: 0.15
Nodes (25): BestResponse, actionBestResponse, bestResponse, chanceBestReponse, color_iso_offset, debug, getBestReponseEv, nthreads (+17 more)

### Community 38 - "Dic5Compairer"
Cohesion: 0.16
Nodes (21): CompairResult, unordered_map, vector, Dic5Compairer, cardslong2rank, compair, compairRanks, fcs (+13 more)

### Community 39 - "TreeModel"
Cohesion: 0.17
Nodes (21): ItemFlags, GameRoundDto, Orientation, QModelIndex, QVariant, Q_OBJECT, QAbstractItemModel, TreeModel (+13 more)

### Community 40 - "CfrPlusTrainable"
Cohesion: 0.12
Nodes (24): CfrPlusTrainable, action_node, action_number, card_number, CfrPlusTrainable::CfrPlusTrainable(), cum_r_plus, cum_r_plus_sum, dump_strategy (+16 more)

### Community 41 - "mainwindow.h"
Cohesion: 0.16
Nodes (7): QDialog, QTextEdit, QObject, QFileSystemModel, QMainWindow, QThread, QTimer

### Community 42 - "diyfp"
Cohesion: 0.11
Nodes (18): FloatType, boundaries, minus, plus, w, cached_power, e, f (+10 more)

### Community 43 - "GameTreeBuildingSettings"
Cohesion: 0.09
Nodes (17): CommandLineTool::CommandLineTool(), GameTreeBuildingSettings, flop_ip, flop_oop, GameTreeBuildingSettings::GameTreeBuildingSettings(), river_ip, river_oop, turn_ip (+9 more)

### Community 44 - "vector"
Cohesion: 0.08
Nodes (20): toStreetSetting, vector, ProgressEvent, exploitability, iteration, message, StreetSettingDto, allin (+12 more)

### Community 45 - "input_adapter"
Cohesion: 0.22
Nodes (15): A1, A2, ContiguousContainer, from_bson(), from_cbor(), from_msgpack(), from_ubjson(), CharT (+7 more)

### Community 46 - "boardselector"
Cohesion: 0.11
Nodes (22): boardselector, boardEdit, boardselector::boardselector(), boardSelectorTableDelegate, boardSelectorTableModel, mode, on_boardEdit_textChanged, on_boardEdit_textEdited (+14 more)

### Community 47 - "PrivateCards"
Cohesion: 0.08
Nodes (25): vector, PrivateCards, board_long, card1, card2, card_vec, hash_code, hashCode (+17 more)

### Community 48 - "serializer"
Cohesion: 0.11
Nodes (17): error_handler_t, find_largest_pow10(), serializer, decimal_point, error_handler, indent_char, indent_string, loc (+9 more)

### Community 49 - "string"
Cohesion: 0.07
Nodes (24): exception, id, m, invalid_iterator, string, out_of_range, type_error, PokerActions (+16 more)

### Community 50 - ".size"
Cohesion: 0.09
Nodes (21): array, contains(), dump_integer(), escape(), flatten(), get_to(), hash<nlohmann::json>, json (+13 more)

### Community 51 - "size_t"
Cohesion: 0.11
Nodes (15): binary_reader, chars_read, current, ia, is_little_endian, sax, clear(), parse_error (+7 more)

### Community 52 - "ActionNode"
Cohesion: 0.16
Nodes (19): ActionNode, ActionNode::~ActionNode(), actions, childrens, getPlayer, getTrainable, getType, player (+11 more)

### Community 53 - "LocalBackend"
Cohesion: 0.06
Nodes (37): ProgressCallback, Q_OBJECT, SolverMode, weak_ptr, LocalBackend, actionLabel, buildTree, estimateMemory (+29 more)

### Community 54 - "DetailViewerModel"
Cohesion: 0.17
Nodes (19): Orientation, QModelIndex, QObject, QVariant, DetailViewerModel, clicked_event, columnCount, columns (+11 more)

### Community 55 - "htmltableview.cpp"
Cohesion: 0.15
Nodes (17): QMouseEvent, QPoint, QString, QWidget, anchorAt, HtmlTableView::HtmlTableView(), mouseMoveEvent, mousePressEvent (+9 more)

### Community 56 - ".tgamma"
Cohesion: 0.13
Nodes (11): builtin_isinf(), erf(), erfc(), exp(), exp2(), expm1(), isinf(), lgamma() (+3 more)

### Community 57 - "RoughStrategyViewerModel"
Cohesion: 0.19
Nodes (17): Orientation, QModelIndex, QObject, QVariant, Q_OBJECT, QAbstractItemModel, RoughStrategyViewerModel, columnCount (+9 more)

### Community 58 - "Combinations"
Cohesion: 0.15
Nodes (13): Combinations, comb, count, generate, M, N, out, partial (+5 more)

### Community 59 - "ActionLabelDto"
Cohesion: 0.08
Nodes (52): ActionLabelDto, amount, label, normalization_tanh(), QModelIndex, QPainter, QStyleOptionViewItem, DetailItemDelegate (+44 more)

### Community 60 - "GameActions"
Cohesion: 0.24
Nodes (10): PokerActions, GameActions, action, amount, GameActions::GameActions(), getAction, getAmount, pokerActionToString (+2 more)

### Community 61 - "iteration_proxy_value"
Cohesion: 0.13
Nodes (12): accept(), iteration_proxy, container, iteration_proxy_value, anchor, array_index, array_index_last, array_index_str (+4 more)

### Community 62 - "local_backend.cpp"
Cohesion: 0.09
Nodes (31): activeSolver, clearNodeMap, emitProgress, toDto, toTreeSettings, GameRound, GameRoundDto, GameTreeNodeType (+23 more)

### Community 63 - "operator+="
Cohesion: 0.13
Nodes (9): basic_ostream, charT, isgreater(), isgreaterequal(), isless(), islessequal(), isnan(), operator+=() (+1 more)

### Community 64 - "HtmlTableRangeView"
Cohesion: 0.13
Nodes (16): Mode, QTextEdit, QWidget, grid_pressed, grid_release, RangeSelector::RangeSelector(), QEvent, QObject (+8 more)

### Community 65 - "ShowdownNode"
Cohesion: 0.18
Nodes (12): ShowDownResult, GameRound, GameTreeNodeType, shared_ptr, vector, vector, ShowdownNode, get_payoffs (+4 more)

### Community 66 - "Solver"
Cohesion: 0.15
Nodes (11): shared_ptr, shared_ptr, Solver, dumps, get_evs, get_strategy, getTree, Solver::Solver() (+3 more)

### Community 67 - "QDebugStream"
Cohesion: 0.15
Nodes (10): basic_streambuf<char>, streambuf, int_type, ostream, streambuf, QDebugStream, log_window, m_old_buf (+2 more)

### Community 68 - "Trainable"
Cohesion: 0.14
Nodes (11): shared_ptr, copyStrategy, Trainable, copyStrategy, dump_evs, dump_strategy, get_type, getAverageStrategy (+3 more)

### Community 69 - "QSTextEdit"
Cohesion: 0.16
Nodes (13): get_logwindow, getLogArea, QString, QWidget, Q_OBJECT, QTextEdit, QSTextEdit, log_with_signal (+5 more)

### Community 70 - "HtmlTableView"
Cohesion: 0.14
Nodes (14): QEvent, QObject, Q_OBJECT, HtmlTableView, eventFilter, itemMouseChange, last_bearing_i, last_bearing_j (+6 more)

### Community 71 - "TreeItem"
Cohesion: 0.17
Nodes (16): QList, QVariant, TreeItem, child, childCount, columnCount, data, insertChild (+8 more)

### Community 72 - "json_sax"
Cohesion: 0.15
Nodes (12): json_sax, boolean, end_array, end_object, key, null, number_float, number_integer (+4 more)

### Community 73 - "worditemdelegate.cpp"
Cohesion: 0.21
Nodes (11): QSize, QModelIndex, QObject, QPainter, QPoint, QString, QStyleOptionViewItem, anchorAt (+3 more)

### Community 74 - "TerminalNode"
Cohesion: 0.20
Nodes (11): GameRound, GameTreeNodeType, shared_ptr, vector, vector, TerminalNode, get_payoffs, getType (+3 more)

### Community 75 - "SettingEditor"
Cohesion: 0.22
Nodes (10): QWidget, Q_OBJECT, QDialog, SettingEditor, initized, on_confirmBox_accepted, on_languageBox_currentIndexChanged, public (+2 more)

### Community 76 - "handle_value"
Cohesion: 0.08
Nodes (17): handle_value(), json_sax_dom_callback_parser, allow_exceptions, callback, discarded, errored, keep_stack, key_keep_stack (+9 more)

### Community 77 - "RiverCombs"
Cohesion: 0.20
Nodes (8): vector, vector, RiverCombs, board, private_cards, rank, reach_prob_index, RiverCombs::RiverCombs()

### Community 79 - "DropTextEdit"
Cohesion: 0.22
Nodes (8): QDropEvent, QWidget, DropTextEdit, dropEvent, DropTextEdit::DropTextEdit(), public, Q_OBJECT, QTextEdit

### Community 80 - "json_ref"
Cohesion: 0.31
Nodes (6): json_ref, is_rvalue, owned_value, value_ref, initializer_list, value_type

### Community 81 - "htmltableview.h"
Cohesion: 0.29
Nodes (5): QTableView, MouseTracker, pressed_i, pressed_j, tracking

### Community 82 - "Deck"
Cohesion: 0.22
Nodes (6): Deck, cards, cards_str, ranks, suits, vector

### Community 83 - "SolveRequest"
Cohesion: 0.09
Nodes (22): SolverMode, SolveRequest, accuracy, allin_threshold, big_blind, board, current_round, dump_rounds (+14 more)

### Community 84 - ".cast_impl"
Cohesion: 0.33
Nodes (4): half_cast(), half_caster<half,half,R>, half_caster<half,U,R>, U

### Community 85 - "parser"
Cohesion: 0.29
Nodes (6): parser, allow_exceptions, last_token, m_lexer, lexer_t, parser_callback_t

### Community 86 - "main.cpp"
Cohesion: 0.33
Nodes (5): QString, main(), myMessageOutput(), QMessageLogContext, QtMsgType

### Community 87 - "lookup8.cpp"
Cohesion: 0.53
Nodes (5): hash1(), hash2(), hash3(), ub1, ub8

### Community 88 - ".fmax"
Cohesion: 0.40
Nodes (3): binary_specialized, fmax(), fmin()

### Community 89 - ".operator()"
Cohesion: 0.50
Nodes (3): argument_type, hash<half_float::half>, result_type

### Community 118 - "README.md"
Cohesion: 0.10
Nodes (19): Console version Usage, CPP Texas Solver, Email, GUI version Usage, Install, Introduction, License, Q & As (+11 more)

### Community 119 - "ChanceNode"
Cohesion: 0.15
Nodes (16): ChanceNode, ChanceNode::ChanceNode(), children, donk, getChildren, getPlayer, getType, isDonk (+8 more)

### Community 120 - "ISolverBackend"
Cohesion: 0.06
Nodes (30): ISolverBackend, buildTree, estimateMemory, getBoard, getDeckCards, getRanks, getStack, getStrategyAt (+22 more)

### Community 121 - "ArgumentParser"
Cohesion: 0.12
Nodes (12): AnyVector, ArgumentVector, ArgumentParser, app_name_, arguments_, final_name_, ignore_first_, index_ (+4 more)

### Community 124 - "argparse.hpp"
Cohesion: 0.20
Nodes (8): Any, content, Holder, held_, ValueType, PlaceHolder, clone, toPtr()

### Community 130 - "main_backup"
Cohesion: 0.31
Nodes (8): EXPORT, api(), main_backup(), execFromFile, processCommand, startWorking, vector, split()

### Community 132 - "StrategyGridDto"
Cohesion: 0.10
Nodes (21): GameRoundDto, NodeType, StrategyGridDto, actions, card_int_to_str, current_player, evs, node_round (+13 more)

## Knowledge Gaps
- **694 isolated node(s):** `public`, `ui`, `boardEdit`, `mode`, `rank_list` (+689 more)
  These have ≤1 connection - possible missing edges or undocumented components. (Counts symbols only; 1075 node(s) total have ≤1 connection when file, concept and rationale nodes are included.)
- **5 thin communities (<3 nodes) omitted from report** — run `graphify query` to explore isolated nodes.

## Suggested Questions
_Questions this graph is uniquely positioned to answer:_

- **Why does `string` connect `string` to `json.hpp`, `.create`, `main_backup`, `StrategyGridDto`, `tinyformat.h`, `sax_parse_internal`, `QSolverJob`, `runtime_error`, `GameTreeNode`, `string_t`, `CommandLineTool`, `PCfrSolver`, `Card`, `PCfrSolver.cpp`, `PokerSolver`, `TCfrSolver`, `RiverRangeManager`, `Rule`, `mainwindow.h`, `GameTreeBuildingSettings`, `vector`, `.size`, `size_t`, `LocalBackend`, `Combinations`, `ActionLabelDto`, `GameActions`, `local_backend.cpp`, `QDebugStream`, `json_sax`, `Deck`, `SolveRequest`, `main.cpp`, `argparse.hpp`?**
  _High betweenness centrality (0.299) - this node is a cross-community bridge._
- **Why does `PrivateCards` connect `PrivateCards` to `DiscountedCfrTrainableSF`, `RiverRangeManager`, `Solver`, `Trainable`, `BestResponse`, `CfrPlusTrainable`, `RiverCombs`, `Deck`, `ActionNode`, `PCfrSolver`, `DetailWindowSetting`, `DiscountedCfrTrainableHF`, `PCfrSolver.cpp`, `PokerSolver`, `TCfrSolver`, `DiscountedCfrTrainable`?**
  _High betweenness centrality (0.076) - this node is a cross-community bridge._
- **Why does `GameTreeNode` connect `GameTreeNode` to `RiverRangeManager`, `Rule`, `ShowdownNode`, `BestResponse`, `mainwindow.h`, `TerminalNode`, `vector`, `PCfrSolver.cpp`, `runtime_error`, `ActionNode`, `LocalBackend`, `ChanceNode`, `DetailWindowSetting`, `GameActions`?**
  _High betweenness centrality (0.060) - this node is a cross-community bridge._
- **Are the 74 inferred relationships involving `runtime_error` (e.g. with `boardselector::boardselector()` and `RangeSelector::RangeSelector()`) actually correct?**
  _`runtime_error` has 74 INFERRED edges - model-reasoned connections that need verification._
- **What connects `public`, `ui`, `boardEdit` to the rest of the system?**
  _694 weakly-connected nodes found - possible documentation gaps or missing edges._
- **Should `json.hpp` be split into smaller, more focused modules?**
  _Cohesion score 0.02193419740777667 - nodes in this community are weakly interconnected._
- **Should `half` be split into smaller, more focused modules?**
  _Cohesion score 0.052289815447710185 - nodes in this community are weakly interconnected._