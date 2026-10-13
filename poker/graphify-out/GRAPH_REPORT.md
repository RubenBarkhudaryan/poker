# Graph Report - poker  (2026-10-13)

## Corpus Check
- 208 files · ~3,280,045 words
- Verdict: corpus is large enough that graph structure adds value.

## Summary
- 2792 nodes · 6031 edges · 144 communities (110 shown, 6 thin omitted)
- Extraction: 96% EXTRACTED · 4% INFERRED · 0% AMBIGUOUS · INFERRED: 261 edges (avg confidence: 0.84)
- Token cost: 0 input · 0 output

## Graph Freshness
- Built from commit: `1829f48b`
- Run `git rev-parse HEAD` and compare to check if the graph is stale.
- Run `graphify update .` after code changes (no API cost).

## Community Hubs (Navigation)
- json.hpp
- half
- push_back
- TreeBuilder.py
- tinyformat.h
- ArgumentParser
- string
- RangeSelectorTableModel
- BoardSelectorTableModel
- MainWindow
- functions
- TableStrategyModel
- .create
- QSolverJob
- half-1-12-0.h
- runtime_error
- lexer
- T
- GameTreeNode
- BasicJsonType
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
- Deck
- primitive_iterator_t
- wide_string_input_adapter
- BestResponse
- Dic5Compairer
- TreeModel
- CfrPlusTrainable
- mainwindow.h
- diyfp
- GameTreeBuildingSettings
- RemoteBackend
- sax_parse
- boardselector
- PrivateCards
- serializer
- progressbar
- remote_backend.cpp
- binary_reader
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
- size_t
- TerminalNode
- SettingEditor
- .size
- QString
- DropTextEdit
- json_ref
- htmltableview.h
- get_impl_ptr
- SolveRequest
- .cast_impl
- ServerProcess
- main.cpp
- lookup8.cpp
- .fmax
- .operator()
- build-AppImage.sh
- pre-commit
- replace.sh
- StrategyItemDelegate
- README.md
- ChanceNode
- ISolverBackend
- DetailItemDelegate
- PrivateCardsManager
- string_split
- qsolverjob.cpp
- SolverServer
- json
- FormatArg
- ostream
- TreeNodeDto
- StrategyGridDto
- worditemdelegate.cpp
- QSTreeView
- .dispatch
- Usage
- strategyexplorer.h
- main
- is_convertible
- htmltablerangeview.cpp
- library.cpp
- RemoteBackend::RemoteBackend

## God Nodes (most connected - your core abstractions)
1. `string` - 127 edges
2. `half` - 120 edges
3. `expr` - 117 edges
4. `MainWindow` - 71 edges
5. `functions` - 68 edges
6. `TableStrategyModel` - 64 edges
7. `GameTreeNode` - 59 edges
8. `PCfrSolver` - 59 edges
9. `Card` - 55 edges
10. `PrivateCards` - 49 edges

## Surprising Connections (you probably didn't know these)
- `main()` --references--> `MainWindow`  [INFERRED]
  main.cpp → mainwindow.h
- `StrategyExplorer::StrategyExplorer()` --calls--> `itemMouseChange`  [INFERRED]
  strategyexplorer.cpp → htmltableview.h
- `LocalBackend::getTreeChildren()` --calls--> `findNode`  [INFERRED]
  src/api/local_backend.cpp → include/api/local_backend.h
- `LocalBackend::toDto()` --calls--> `registerNode`  [INFERRED]
  src/api/local_backend.cpp → include/api/local_backend.h
- `LocalBackend::stop()` --calls--> `emitProgress`  [INFERRED]
  src/api/local_backend.cpp → include/api/local_backend.h

## Import Cycles
- None detected.

## Communities (144 total, 6 thin omitted)

### Community 0 - "json.hpp"
Cohesion: 0.02
Nodes (114): array, B1, CompatibleNumberIntegerType, CompatibleType, conditional<bool(B1::value), conjunction<Bn...>, B1>::type, ConstructibleArrayType, ConstructibleObjectType, ConstructibleStringType (+106 more)

### Community 1 - "half"
Cohesion: 0.05
Nodes (54): float_round_style, abs(), acos(), acosh(), asin(), asinh(), atan(), atan2() (+46 more)

### Community 2 - "push_back"
Cohesion: 0.14
Nodes (43): const_iterator, const_reference, at(), BasicJsonType* handle_value(Value&& v)(), begin(), cbegin(), cend(), contains() (+35 more)

### Community 3 - "TreeBuilder.py"
Cohesion: 0.06
Nodes (16): ActionNode, BoardcardChanceNode, ChanceNode, check_number_this_round(), DealCardNode, FiveCardTexasTreeBuilder, HolecardChanceNode, Node (+8 more)

### Community 4 - "tinyformat.h"
Cohesion: 0.10
Nodes (16): convertToInt, convertToInt<T,true>, FormatListN<0>, formatValueAsType, formatValueAsType<T,fmtT,true>, formatZeroIntegerWorkaround, formatZeroIntegerWorkaround<T,true>, is_wchar (+8 more)

### Community 5 - "ArgumentParser"
Cohesion: 0.07
Nodes (27): AnyVector, ArgumentVector, Any, content, Argument, fixed, name, optional (+19 more)

### Community 6 - "string"
Cohesion: 0.13
Nodes (11): QString, map, mutex, vector, SessionInfo, board, deck, ranks (+3 more)

### Community 7 - "RangeSelectorTableModel"
Cohesion: 0.06
Nodes (46): QModelIndex, QObject, QPainter, QStringList, QStyleOptionViewItem, Q_OBJECT, QStringList, RangeSelectorTableDelegate (+38 more)

### Community 8 - "BoardSelectorTableModel"
Cohesion: 0.06
Nodes (45): BoardSelectorTableDelegate, BoardSelectorTableDelegate::BoardSelectorTableDelegate(), boardSelectorTableModel, paint, public, rank_list, QModelIndex, QObject (+37 more)

### Community 9 - "MainWindow"
Cohesion: 0.06
Nodes (38): QModelIndex, getParams(), Q_OBJECT, MainWindow, backend_, boardSelector, clear_all_params, getPriUi (+30 more)

### Community 10 - "functions"
Cohesion: 0.06
Nodes (14): HALF_CONSTEXPR, binary_t, functions, HALF_CONSTEXPR half_float::half lowest(), half_float::half denorm_min(), half_float::half epsilon(), half_float::half infinity(), half_float::half max() (+6 more)

### Community 11 - "TableStrategyModel"
Cohesion: 0.07
Nodes (46): Orientation, pair, QModelIndex, QVariant, vector, map, pair, Q_OBJECT (+38 more)

### Community 12 - ".create"
Cohesion: 0.33
Nodes (6): get_bson_string(), get_number(), get_string(), json_sax_acceptor, sax_parse_internal(), input_format_t

### Community 13 - "QSolverJob"
Cohesion: 0.07
Nodes (30): MissionType, Q_OBJECT, QString, QThread, shared_ptr, QSolverJob, accuracy, allin_threshold (+22 more)

### Community 14 - "half-1-12-0.h"
Cohesion: 0.06
Nodes (39): conditional<B,T,F>, digits, binary_specialized<half,half>, bits, bits<const T>, bits<const volatile T>, bits<double>, bits<float> (+31 more)

### Community 15 - "runtime_error"
Cohesion: 0.14
Nodes (38): BetType, ifstream, runtime_error, ActionNode, ChanceNode, GameRound, json, shared_ptr (+30 more)

### Community 16 - "lexer"
Cohesion: 0.08
Nodes (18): lexer, current, decimal_point_char, error_message, ia, JSON_HEDLEY_CONST, next_unget, position (+10 more)

### Community 17 - "T"
Cohesion: 0.09
Nodes (29): builtin_isnan(), builtin_signbit(), fabs(), float2half(), float2half_impl(), fmod(), frexp(), false_type (+21 more)

### Community 18 - "GameTreeNode"
Cohesion: 0.15
Nodes (19): GameRound, shared_ptr, GameTreeNode, depth, GameTreeNode::GameTreeNode(), getParent, getPot, getRound (+11 more)

### Community 19 - "BasicJsonType"
Cohesion: 0.09
Nodes (23): BasicJsonType, CharType, binary_writer, is_little_endian, oa, dump_integer(), basic_ostream, output_adapter_protocol (+15 more)

### Community 20 - "RangeSelector"
Cohesion: 0.09
Nodes (33): QModelIndex, QString, Q_OBJECT, QDialog, QFileSystemModel, QStringList, QTextEdit, RangeSelector (+25 more)

### Community 21 - "CommandLineTool"
Cohesion: 0.07
Nodes (31): EXPORT, api(), main_backup(), CommandLineTool, accuracy, allin_threshold, big_blind, board (+23 more)

### Community 22 - "PCfrSolver"
Cohesion: 0.06
Nodes (36): GameRound, MonteCarolAlg, shared_ptr, vector, PCfrSolver, accuracy, collecting_statics, color_iso_offset (+28 more)

### Community 23 - "from_json"
Cohesion: 0.08
Nodes (34): Allocator, ArithmeticType, Compare, CompatibleArrayType, CompatibleNumberUnsignedType, CompatibleObjectType, CompatibleString, CompatibleStringType (+26 more)

### Community 24 - "DetailWindowSetting"
Cohesion: 0.15
Nodes (12): QStringList, DetailWindowMode, QStyledItemDelegate, QObject, DetailItemDelegate::DetailItemDelegate(), DetailWindowSetting, grid_i, grid_j (+4 more)

### Community 25 - "StrategyExplorer"
Cohesion: 0.10
Nodes (35): itemMouseChange, QModelIndex, QWidget, Q_OBJECT, QDialog, QTimer, vector, StrategyExplorer (+27 more)

### Community 26 - "Card"
Cohesion: 0.15
Nodes (28): Card, boardCard2long, boardCards2html, boardCards2long, boardInt2long, boardInts2long, card, card2int (+20 more)

### Community 27 - "DiscountedCfrTrainableHF"
Cohesion: 0.09
Nodes (32): ActionNode, json, shared_ptr, TrainableType, vector, DiscountedCfrTrainableHF, action_number, alpha (+24 more)

### Community 28 - "PCfrSolver.cpp"
Cohesion: 0.18
Nodes (27): gameRound2int, toString, ActionNode, ChanceNode, json, MonteCarolAlg, shared_ptr, ShowdownNode (+19 more)

### Community 29 - "PokerSolver"
Cohesion: 0.18
Nodes (9): shared_ptr, vector, PokerSolver, compairer, deck, game_tree, player1Range, player2Range (+1 more)

### Community 30 - "TCfrSolver"
Cohesion: 0.06
Nodes (31): MonteCarolAlg, shared_ptr, vector, TCfrSolver, actionUtility, cfr, chanceUtility, compairer (+23 more)

### Community 31 - "DiscountedCfrTrainable"
Cohesion: 0.11
Nodes (30): ActionNode, json, shared_ptr, TrainableType, vector, DiscountedCfrTrainable, action_number, alpha (+22 more)

### Community 32 - "DiscountedCfrTrainableSF"
Cohesion: 0.09
Nodes (32): ActionNode, json, shared_ptr, TrainableType, vector, DiscountedCfrTrainableSF, action_number, alpha (+24 more)

### Community 33 - "RiverRangeManager"
Cohesion: 0.10
Nodes (16): Compairer, compair, dic_dir, get_rank, lines, shared_ptr, mutex, shared_ptr (+8 more)

### Community 34 - "Deck"
Cohesion: 0.08
Nodes (23): vector, Deck, cards, cards_str, Deck::Deck(), ranks, suits, vector (+15 more)

### Community 35 - "primitive_iterator_t"
Cohesion: 0.10
Nodes (16): base_iterator, difference_type, internal_iterator, array_iterator, object_iterator, primitive_iterator, iter_impl, m_it (+8 more)

### Community 36 - "wide_string_input_adapter"
Cohesion: 0.09
Nodes (18): FILE, clear(), file_input_adapter, m_file, int_type, input_adapter_protocol, get_character, input_buffer_adapter (+10 more)

### Community 37 - "BestResponse"
Cohesion: 0.15
Nodes (25): BestResponse, actionBestResponse, bestResponse, chanceBestReponse, color_iso_offset, debug, getBestReponseEv, nthreads (+17 more)

### Community 38 - "Dic5Compairer"
Cohesion: 0.15
Nodes (22): CompairResult, printf(), unordered_map, vector, Dic5Compairer, cardslong2rank, compair, compairRanks (+14 more)

### Community 39 - "TreeModel"
Cohesion: 0.17
Nodes (21): ItemFlags, GameRoundDto, Orientation, QModelIndex, QVariant, Q_OBJECT, QAbstractItemModel, TreeModel (+13 more)

### Community 40 - "CfrPlusTrainable"
Cohesion: 0.12
Nodes (24): CfrPlusTrainable, action_node, action_number, card_number, CfrPlusTrainable::CfrPlusTrainable(), cum_r_plus, cum_r_plus_sum, dump_strategy (+16 more)

### Community 41 - "mainwindow.h"
Cohesion: 0.16
Nodes (8): QDialog, QTextEdit, QObject, QFileSystemModel, QThread, QMainWindow, QPointer, QTimer

### Community 42 - "diyfp"
Cohesion: 0.10
Nodes (20): FloatType, boundaries, minus, plus, w, cached_power, e, f (+12 more)

### Community 43 - "GameTreeBuildingSettings"
Cohesion: 0.10
Nodes (16): GameTreeBuildingSettings, flop_ip, flop_oop, GameTreeBuildingSettings::GameTreeBuildingSettings(), river_ip, river_oop, turn_ip, turn_oop (+8 more)

### Community 44 - "RemoteBackend"
Cohesion: 0.07
Nodes (27): mutex, ProgressCallback, QString, quint16, RemoteBackend, buildTree, estimateMemory, getBoard (+19 more)

### Community 45 - "sax_parse"
Cohesion: 0.13
Nodes (24): A1, A2, ContiguousContainer, accept(), from_bson(), from_cbor(), from_msgpack(), from_ubjson() (+16 more)

### Community 46 - "boardselector"
Cohesion: 0.14
Nodes (19): boardselector, boardEdit, boardselector::boardselector(), boardSelectorTableDelegate, boardSelectorTableModel, on_boardEdit_textChanged, on_boardEdit_textEdited, on_boardSelectorTable_clicked (+11 more)

### Community 47 - "PrivateCards"
Cohesion: 0.09
Nodes (21): vector, PrivateCards, board_long, card1, card2, card_vec, hashCode, relative_prob (+13 more)

### Community 48 - "serializer"
Cohesion: 0.09
Nodes (26): error_handler_t, basic_json, vector, output_adapter, oa, serializer, decimal_point, error_handler (+18 more)

### Community 49 - "progressbar"
Cohesion: 0.11
Nodes (13): progressbar, closing_bracket_char, do_show_bar, done_char, last_perc, n_cycles, opening_bracket_char, progress (+5 more)

### Community 50 - "remote_backend.cpp"
Cohesion: 0.15
Nodes (22): call, invalidateInfo, startJobAndWait, json, ProgressCallback, RemoteBackend::buildTree(), RemoteBackend::call(), RemoteBackend::estimateMemory() (+14 more)

### Community 51 - "binary_reader"
Cohesion: 0.15
Nodes (7): binary_reader, chars_read, current, ia, is_little_endian, sax, json_sax_t

### Community 52 - "ActionNode"
Cohesion: 0.16
Nodes (19): ActionNode, ActionNode::~ActionNode(), actions, childrens, getPlayer, getTrainable, getType, player (+11 more)

### Community 53 - "LocalBackend"
Cohesion: 0.06
Nodes (37): ProgressCallback, Q_OBJECT, weak_ptr, LocalBackend, actionLabel, buildTree, estimateMemory, findNode (+29 more)

### Community 54 - "DetailViewerModel"
Cohesion: 0.17
Nodes (19): Orientation, QModelIndex, QObject, QVariant, DetailViewerModel, clicked_event, columnCount, columns (+11 more)

### Community 55 - "htmltableview.cpp"
Cohesion: 0.18
Nodes (15): QMouseEvent, QPoint, QString, QWidget, anchorAt, HtmlTableView::HtmlTableView(), mouseMoveEvent, mousePressEvent (+7 more)

### Community 56 - ".tgamma"
Cohesion: 0.13
Nodes (11): builtin_isinf(), erf(), erfc(), exp(), exp2(), expm1(), isinf(), lgamma() (+3 more)

### Community 57 - "RoughStrategyViewerModel"
Cohesion: 0.19
Nodes (17): Orientation, QModelIndex, QObject, QVariant, Q_OBJECT, QAbstractItemModel, RoughStrategyViewerModel, columnCount (+9 more)

### Community 58 - "Combinations"
Cohesion: 0.17
Nodes (13): Combinations, comb, count, generate, M, N, out, partial (+5 more)

### Community 59 - "ActionLabelDto"
Cohesion: 0.17
Nodes (19): ActionLabelDto, amount, label, QModelIndex, QObject, QPainter, QStyleOptionViewItem, Q_OBJECT (+11 more)

### Community 60 - "GameActions"
Cohesion: 0.27
Nodes (10): PokerActions, GameActions, action, amount, GameActions::GameActions(), getAction, getAmount, pokerActionToString (+2 more)

### Community 61 - "iteration_proxy_value"
Cohesion: 0.13
Nodes (14): get(), ValueType, int_to_string(), iteration_proxy, container, iteration_proxy_value, anchor, array_index (+6 more)

### Community 62 - "local_backend.cpp"
Cohesion: 0.07
Nodes (29): clearNodeMap, emitProgress, toTreeSettings, GameRound, GameRoundDto, GameTreeNodeType, NodeType, PokerActions (+21 more)

### Community 63 - "operator+="
Cohesion: 0.13
Nodes (9): basic_ostream, charT, isgreater(), isgreaterequal(), isless(), islessequal(), isnan(), operator+=() (+1 more)

### Community 64 - "HtmlTableRangeView"
Cohesion: 0.18
Nodes (12): QTextEdit, QWidget, grid_pressed, grid_release, RangeSelector::RangeSelector(), Q_OBJECT, HtmlTableRangeView, item_release (+4 more)

### Community 65 - "ShowdownNode"
Cohesion: 0.18
Nodes (12): ShowDownResult, GameRound, GameTreeNodeType, shared_ptr, vector, vector, ShowdownNode, get_payoffs (+4 more)

### Community 66 - "Solver"
Cohesion: 0.16
Nodes (11): shared_ptr, shared_ptr, Solver, dumps, get_evs, get_strategy, getTree, Solver::Solver() (+3 more)

### Community 67 - "QDebugStream"
Cohesion: 0.15
Nodes (10): basic_streambuf<char>, streambuf, int_type, ostream, streambuf, QDebugStream, log_window, m_old_buf (+2 more)

### Community 68 - "Trainable"
Cohesion: 0.13
Nodes (10): ActionNode, Trainable, copyStrategy, dump_evs, dump_strategy, get_type, getAverageStrategy, getcurrentStrategy (+2 more)

### Community 69 - "QSTextEdit"
Cohesion: 0.18
Nodes (12): get_logwindow, getLogArea, QString, QWidget, Q_OBJECT, QTextEdit, QSTextEdit, log_with_signal (+4 more)

### Community 70 - "HtmlTableView"
Cohesion: 0.15
Nodes (13): QEvent, QObject, Q_OBJECT, HtmlTableView, eventFilter, last_bearing_i, last_bearing_j, _lastHoveredAnchor (+5 more)

### Community 71 - "TreeItem"
Cohesion: 0.17
Nodes (17): QList, QVariant, TreeItem, child, childCount, columnCount, data, insertChild (+9 more)

### Community 72 - "json_sax"
Cohesion: 0.15
Nodes (12): json_sax, boolean, end_array, end_object, key, null, number_float, number_integer (+4 more)

### Community 73 - "size_t"
Cohesion: 0.12
Nodes (14): exception, id, m, invalid_iterator, other_error, out_of_range, parse_error, byte (+6 more)

### Community 74 - "TerminalNode"
Cohesion: 0.20
Nodes (11): GameRound, GameTreeNodeType, shared_ptr, vector, vector, TerminalNode, get_payoffs, getType (+3 more)

### Community 75 - "SettingEditor"
Cohesion: 0.22
Nodes (10): QWidget, Q_OBJECT, QDialog, SettingEditor, initized, on_confirmBox_accepted, on_languageBox_currentIndexChanged, public (+2 more)

### Community 76 - ".size"
Cohesion: 0.07
Nodes (24): back(), escape(), flatten(), handle_value(), is_discarded(), json_sax_dom_callback_parser, allow_exceptions, callback (+16 more)

### Community 77 - "QString"
Cohesion: 0.15
Nodes (20): function, QString, QWidget, vector, iso_corh(), connectBackend, fillRequestFromUi, isBackendBusy (+12 more)

### Community 79 - "DropTextEdit"
Cohesion: 0.22
Nodes (8): QDropEvent, QWidget, DropTextEdit, dropEvent, DropTextEdit::DropTextEdit(), public, Q_OBJECT, QTextEdit

### Community 80 - "json_ref"
Cohesion: 0.31
Nodes (6): json_ref, is_rvalue, owned_value, value_ref, initializer_list, value_type

### Community 81 - "htmltableview.h"
Cohesion: 0.29
Nodes (5): QTableView, MouseTracker, pressed_i, pressed_j, tracking

### Community 82 - "get_impl_ptr"
Cohesion: 0.18
Nodes (12): array_t, boolean_t, get_impl_ptr(), get_ptr(), is_boolean(), is_number(), is_number_float(), is_number_integer() (+4 more)

### Community 83 - "SolveRequest"
Cohesion: 0.10
Nodes (20): SolveRequest, accuracy, allin_threshold, big_blind, board, current_round, dump_rounds, ip_commit (+12 more)

### Community 84 - ".cast_impl"
Cohesion: 0.33
Nodes (4): half_cast(), half_caster<half,half,R>, half_caster<half,U,R>, U

### Community 85 - "ServerProcess"
Cohesion: 0.12
Nodes (14): quint16, ServerProcess, executablePath, isRunning, port_, process_, start, stop (+6 more)

### Community 86 - "main.cpp"
Cohesion: 0.33
Nodes (5): QMessageLogContext, QString, QtMsgType, main(), myMessageOutput()

### Community 87 - "lookup8.cpp"
Cohesion: 0.53
Nodes (5): hash1(), hash2(), hash3(), ub1, ub8

### Community 88 - ".fmax"
Cohesion: 0.40
Nodes (3): binary_specialized, fmax(), fmin()

### Community 89 - ".operator()"
Cohesion: 0.50
Nodes (3): argument_type, hash<half_float::half>, result_type

### Community 93 - "StrategyItemDelegate"
Cohesion: 0.25
Nodes (16): normalization_tanh(), QModelIndex, QPainter, QStyleOptionViewItem, Q_OBJECT, isBetOrRaiseLabel(), isCheckOrCallLabel(), isFoldLabel() (+8 more)

### Community 118 - "README.md"
Cohesion: 0.10
Nodes (19): Console version Usage, CPP Texas Solver, Email, GUI version Usage, Install, Introduction, License, Q & As (+11 more)

### Community 119 - "ChanceNode"
Cohesion: 0.15
Nodes (16): ChanceNode, ChanceNode::ChanceNode(), children, donk, getChildren, getPlayer, getType, isDonk (+8 more)

### Community 120 - "ISolverBackend"
Cohesion: 0.08
Nodes (23): ISolverBackend, buildTree, estimateMemory, getBoard, getDeckCards, getRanks, getStack, getStrategyAt (+15 more)

### Community 121 - "DetailItemDelegate"
Cohesion: 0.32
Nodes (17): QModelIndex, QPainter, QStyleOptionViewItem, DetailItemDelegate, detailWindowSetting, paint, paint_evs, paint_evs_only (+9 more)

### Community 124 - "PrivateCardsManager"
Cohesion: 0.16
Nodes (12): hash_code, vector, vector, PrivateCardsManager, card_player_index, getInitialReachProb, indPlayer2Player, initialboard (+4 more)

### Community 125 - "string_split"
Cohesion: 0.14
Nodes (18): Dic5Compairer::Dic5Compairer(), vector, string_split(), QString, vector, noDuplicateRange(), build_game_tree, dump_strategy (+10 more)

### Community 126 - "qsolverjob.cpp"
Cohesion: 0.25
Nodes (10): QString, build_tree, estimate_tree_memory, get_solver, loading, run, saving, setContext (+2 more)

### Community 127 - "SolverServer"
Cohesion: 0.10
Nodes (21): atomic, deque, ProgressEvent, exploitability, iteration, message, QTcpServer, QTcpSocket (+13 more)

### Community 128 - "json"
Cohesion: 0.50
Nodes (3): hash<nlohmann::json>, json, operator "" _json()

### Community 129 - "FormatArg"
Cohesion: 0.18
Nodes (10): FormatArg, m_value, FormatList, m_args, m_N, FormatListN, m_formatterStore, parseIntAndAdvance() (+2 more)

### Community 130 - "ostream"
Cohesion: 0.29
Nodes (10): FormatListRef, format(), formatImpl(), formatTruncated(), formatValue(), ostream, T, printFormatStringLiteral() (+2 more)

### Community 131 - "TreeNodeDto"
Cohesion: 0.17
Nodes (13): toDto, GameRoundDto, NodeType, TreeNodeDto, has_children, id, label, node_type (+5 more)

### Community 132 - "StrategyGridDto"
Cohesion: 0.07
Nodes (37): pair, vector, StrategyGridDto, actions, card_int_to_str, current_player, evs, node_round (+29 more)

### Community 133 - "worditemdelegate.cpp"
Cohesion: 0.21
Nodes (11): QSize, QModelIndex, QObject, QPainter, QPoint, QString, QStyleOptionViewItem, anchorAt (+3 more)

### Community 134 - "QSTreeView"
Cohesion: 0.24
Nodes (7): QWidget, QSTreeView, backend_, QSTreeView::QSTreeView(), setTreeData, tree_model, QTreeView

### Community 135 - ".dispatch"
Cohesion: 0.27
Nodes (6): vector, RemoteBackend::getBoard(), RemoteBackend::getDeckCards(), RemoteBackend::getRanks(), RemoteBackend::getStack(), json

### Community 138 - "main"
Cohesion: 0.28
Nodes (7): QHostAddress, QMessageLogContext, QString, QtMsgType, quint16, main(), messageHandler()

### Community 139 - "is_convertible"
Cohesion: 0.29
Nodes (7): fail, dummy, is_convertible, tryConvert, value, succeed, dummy

### Community 140 - "htmltablerangeview.cpp"
Cohesion: 0.33
Nodes (5): QEvent, QObject, QWidget, eventFilter, HtmlTableRangeView::HtmlTableRangeView()

### Community 142 - "RemoteBackend::RemoteBackend"
Cohesion: 0.67
Nodes (3): QString, quint16, RemoteBackend::RemoteBackend()

## Knowledge Gaps
- **725 isolated node(s):** `public`, `ui`, `boardEdit`, `rank_list`, `boardSelectorTableModel` (+720 more)
  These have ≤1 connection - possible missing edges or undocumented components. (Counts symbols only; 1118 node(s) total have ≤1 connection when file, concept and rationale nodes are included.)
- **6 thin communities (<3 nodes) omitted from report** — run `graphify query` to explore isolated nodes.

## Suggested Questions
_Questions this graph is uniquely positioned to answer:_

- **Why does `string` connect `string` to `json.hpp`, `push_back`, `TreeNodeDto`, `StrategyGridDto`, `ostream`, `.dispatch`, `.create`, `QSolverJob`, `runtime_error`, `GameTreeNode`, `CommandLineTool`, `PCfrSolver`, `Card`, `PCfrSolver.cpp`, `TCfrSolver`, `RiverRangeManager`, `Deck`, `mainwindow.h`, `progressbar`, `remote_backend.cpp`, `binary_reader`, `LocalBackend`, `ActionLabelDto`, `GameActions`, `local_backend.cpp`, `QDebugStream`, `json_sax`, `size_t`, `.size`, `SolveRequest`, `main.cpp`, `string_split`, `SolverServer`?**
  _High betweenness centrality (0.340) - this node is a cross-community bridge._
- **Why does `PrivateCards` connect `PrivateCards` to `DiscountedCfrTrainableSF`, `RiverRangeManager`, `Trainable`, `BestResponse`, `CfrPlusTrainable`, `PCfrSolver.cpp`, `DiscountedCfrTrainable`, `PokerSolver`, `ActionNode`, `PCfrSolver`, `DiscountedCfrTrainableHF`, `PrivateCardsManager`, `string_split`, `TCfrSolver`, `SolverServer`?**
  _High betweenness centrality (0.104) - this node is a cross-community bridge._
- **Why does `Card` connect `Card` to `Deck`, `Dic5Compairer`, `string`, `BoardSelectorTableModel`, `strategyexplorer.h`, `TableStrategyModel`, `QString`, `PrivateCardsManager`, `ChanceNode`, `DetailWindowSetting`, `StrategyExplorer`, `PCfrSolver.cpp`?**
  _High betweenness centrality (0.065) - this node is a cross-community bridge._
- **Are the 69 inferred relationships involving `runtime_error` (e.g. with `api()` and `LocalBackend::actionLabel()`) actually correct?**
  _`runtime_error` has 69 INFERRED edges - model-reasoned connections that need verification._
- **What connects `public`, `ui`, `boardEdit` to the rest of the system?**
  _725 weakly-connected nodes found - possible documentation gaps or missing edges._
- **Should `json.hpp` be split into smaller, more focused modules?**
  _Cohesion score 0.02231404958677686 - nodes in this community are weakly interconnected._
- **Should `half` be split into smaller, more focused modules?**
  _Cohesion score 0.052289815447710185 - nodes in this community are weakly interconnected._