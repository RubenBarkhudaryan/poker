const replyWrite = `encoded = encodeMessage({{"ok", true}, {"result", result}});
socket->write(encoded);

QByteArray encodeMessage(const json& message)
{
    // dump() escapes control characters, so '\\n' only ever appears as the terminator.
    return QByteArray::fromStdString(message.dump()) + '\\n';
}`;

const statusFn = `json status()
{
    std::lock_guard<std::mutex> lock(mutex_);
    json result = {
        {"busy", busy_.load()},
        {"job", job_},
        {"loaded", loaded_.load()},
        {"error", last_error_},
        {"events", events_},
    };
    events_.clear();
    return result;
}`;

const stepSource = {
  startup: {
    1: { file: "mainwindow.cpp  ·  MainWindow::connectBackend", code:
`QSettings setting("TexasSolver", "Setting");
setting.beginGroup("solver");
const bool remote_mode = setting.value("backend_mode", "local").toString() == "remote";
const QString host = remote_mode ? setting.value("server_host", "127.0.0.1").toString() : QString("127.0.0.1");
const quint16 port = static_cast<quint16>(setting.value("server_port", 8080).toInt());

backend_ = std::make_unique<RemoteBackend>(host, port);` },
    2: { file: "src/api/server_process.cpp  ·  ServerProcess::start", code:
`process_.setProgram(program);
process_.setArguments({QString::number(port)});
process_.setWorkingDirectory(QCoreApplication::applicationDirPath());
process_.setStandardOutputFile(QProcess::nullDevice());
process_.setProcessChannelMode(QProcess::ForwardedErrorChannel);
#ifdef Q_OS_WIN
process_.setCreateProcessArgumentsModifier(
    [](QProcess::CreateProcessArguments* args) { args->flags |= CREATE_NO_WINDOW; });
#endif
process_.start();` },
    3: { file: "src/server/main.cpp  ·  main", code:
`const quint16 port = argc > 1 ? QString(argv[1]).toUShort() : 8080;
const QHostAddress address = argc > 2 ? QHostAddress(QString(argv[2])) : QHostAddress(QHostAddress::LocalHost);

qInstallMessageHandler(messageHandler);
if (!server.listen(address, port)) {
    std::cerr << "listen failed: " << server.errorString().toStdString() << std::endl;
    return 1;
}
return app.exec();` },
    4: { file: "src/api/remote_backend.cpp  ·  RemoteBackend::waitUntilReachable", code:
`bool RemoteBackend::waitUntilReachable(int timeout_ms) const
{
    QElapsedTimer timer;
    timer.start();
    for (;;) {
        try {
            call("status");
            return true;
        } catch (const std::exception&) {
            if (timer.elapsed() >= timeout_ms)
                return false;
            QThread::msleep(200);
        }
    }
}` },
    5: { file: "src/server/main.cpp  ·  SolverServer::status", code: statusFn },
    6: { file: "src/server/main.cpp  ·  onReadyRead", code: replyWrite },
    7: { file: "src/api/remote_backend.cpp  ·  RemoteBackend::load", code:
`void RemoteBackend::load()
{
    invalidateInfo();
    startJobAndWait("load", json::object());
}` },
    8: { file: "src/server/main.cpp  ·  dispatch load", code:
`if (cmd == "load") {
    if (loaded_)
        return nullptr;
    startJob("load", [this]() {
        backend_.load();
        loaded_ = true;
    });
    return nullptr;
}` },
    9: { file: "src/server/main.cpp  ·  onReadyRead", code: replyWrite },
    10: { file: "src/api/local_backend.cpp  ·  LocalBackend::load", code:
`emitProgress("Loading holdem comparing file");
solver_ = PokerSolver("2,3,4,5,6,7,8,9,T,J,Q,K,A", suits,
                      resource_dir + "/compairer/card5_dic_sorted.txt", 2598961,
                      resource_dir + "/compairer/card5_dic_zipped.bin");

emitProgress("Loading finished. Good to go.");` },
    11: { file: "src/api/remote_backend.cpp  ·  RemoteBackend::startJobAndWait", code:
`void RemoteBackend::startJobAndWait(const std::string& cmd, const json& args)
{
    call(cmd, args);
    for (;;) {
        const json status = call("status");
        if (progress) {
            for (const json& event : status.at("events"))
                progress(event.get<ProgressEvent>());
        }
        if (!status.at("busy").get<bool>()) {
            const std::string error = status.value("error", std::string());
            if (!error.empty())
                throw std::runtime_error(error);
            return;
        }
        QThread::msleep(kStatusPollMs);
    }
}` },
    12: { file: "src/server/main.cpp  ·  SolverServer::status", code: statusFn }
  },
  build: {
    1: { file: "mainwindow.cpp  ·  MainWindow::fillRequestFromUi", code:
`request.range_ip = this->ui->ipRangeText->toPlainText().toStdString();
request.range_oop = this->ui->oopRangeText->toPlainText().toStdString();
request.board = this->ui->boardText->toPlainText().toStdString();
request.tree_settings.flop_ip =
    streetFromUi(ui->flop_ip_bet->text(), ui->flop_ip_raise->text(), QString(), ui->flop_ip_allin->isChecked());
request.tree_settings.river_oop = streetFromUi(ui->river_oop_bet->text(), ui->river_oop_raise->text(),
                                               ui->river_oop_donk->text(), ui->river_oop_allin->isChecked());` },
    2: { file: "mainwindow.cpp  ·  MainWindow::runInBackground", code:
`void MainWindow::runInBackground(std::function<void()> task) {
    if (isBackendBusy()) {
        this->ui->logOutput->log_with_signal(tr("Solver is busy, wait for the current job or press stop."));
        return;
    }
    worker_ = QThread::create([this, task]() {
        try {
            task();
        } catch (const std::exception& error) {
            logError(QString::fromStdString(error.what()));
        }
    });
    worker_->start();
}` },
    3: { file: "src/api/remote_backend.cpp  ·  RemoteBackend::buildTree", code:
`void RemoteBackend::buildTree(const SolveRequest& request)
{
    invalidateInfo();
    startJobAndWait("build", json(request));
}` },
    4: { file: "src/server/main.cpp  ·  SolverServer::onReadyRead", code:
`while (socket->canReadLine()) {
    const QByteArray line = socket->readLine().trimmed();
    const json request = json::parse(line.toStdString());
    const json args = request.value("args", json::object());
    const json result = dispatch(request.at("cmd").get<std::string>(), args);
    encoded = encodeMessage({{"ok", true}, {"result", result}});
    socket->write(encoded);
}` },
    5: { file: "src/server/main.cpp  ·  SolverServer::dispatch", code:
`const bool tree_changing = busy_ && (job_ == "build" || job_ == "load");
if (tree_changing)
    throw std::runtime_error("busy: " + job_ + " is running");

if (cmd != "load")
    requireLoaded();` },
    6: { file: "src/api/local_backend.cpp  ·  LocalBackend::buildTree", code:
`void LocalBackend::buildTree(const SolveRequest& request) {
    last_request_ = request;
    clearNodeMap();
    GameTreeBuildingSettings settings = toTreeSettings(request.tree_settings);
    solver_.build_game_tree(request.oop_commit, request.ip_commit, request.current_round,
                            request.raise_limit, request.small_blind, request.big_blind, request.stack,
                            settings, request.allin_threshold);
}` },
    7: { file: "src/server/main.cpp  ·  onReadyRead", code: replyWrite },
    8: { file: "src/api/remote_backend.cpp  ·  RemoteBackend::startJobAndWait", code:
`call(cmd, args);
for (;;) {
    const json status = call("status");
    if (!status.at("busy").get<bool>()) {
        const std::string error = status.value("error", std::string());
        if (!error.empty())
            throw std::runtime_error(error);
        return;
    }
    QThread::msleep(kStatusPollMs);
}` },
    9: { file: "src/server/main.cpp  ·  SolverServer::startJob", code:
`worker_ = std::thread([this, task]() {
    try {
        task();
    } catch (const std::exception& error) {
        std::lock_guard<std::mutex> lock(mutex_);
        last_error_ = error.what();
    }
    busy_ = false;
});` },
    10: { file: "src/server/main.cpp  ·  SolverServer::status", code: statusFn }
  },
  solve: {
    1: { file: "mainwindow.cpp  ·  MainWindow::on_buttomSolve_clicked", code:
`void MainWindow::on_buttomSolve_clicked() {
    last_request_.max_iteration = ui->iterationText->text().toInt();
    last_request_.accuracy = ui->exploitabilityText->text().toFloat();
    last_request_.print_interval = ui->logIntervalText->text().toInt();
    last_request_.thread_number = ui->threadsText->text().toInt();
    SolveRequest request = last_request_;
    runInBackground([this, request]() { backend_->solve(request); });
}` },
    2: { file: "src/api/remote_backend.cpp  ·  RemoteBackend::solve", code:
`void RemoteBackend::solve(const SolveRequest& request)
{
    invalidateInfo();
    startJobAndWait("solve", json(request));
}` },
    3: { file: "src/server/main.cpp  ·  SolverServer::dispatch", code:
`// build/load replace the game tree, so nothing may read it meanwhile.
// solve/save only touch strategy data, which the explorer is allowed to poll live.
const bool tree_changing = busy_ && (job_ == "build" || job_ == "load");
if (tree_changing)
    throw std::runtime_error("busy: " + job_ + " is running");

if (cmd == "strategy")
    return backend_.getStrategyAt(args.at("id").get<std::string>(), args.value("turn", -1), args.value("river", -1));

if (busy_)
    throw std::runtime_error("busy: " + job_ + " is running");` },
    4: { file: "src/api/local_backend.cpp  ·  LocalBackend::solve", code:
`void LocalBackend::solve(const SolveRequest& request) {
    last_request_ = request;
    solver_.train(request.range_ip, request.range_oop, request.board, "", request.max_iteration,
                  request.print_interval, "discounted_cfr", -1, request.accuracy,
                  request.use_isomorphism != 0, request.use_halffloats, request.thread_number);
}` },
    5: { file: "src/server/main.cpp  ·  onReadyRead", code: replyWrite },
    6: { file: "src/api/remote_backend.cpp  ·  progress callback", code:
`if (progress) {
    for (const json& event : status.at("events"))
        progress(event.get<ProgressEvent>());
}

// MainWindow installs this callback:
backend_->setProgressCallback([](const ProgressEvent& e) {
    if (MainWindow::s_textEdit != nullptr)
        MainWindow::s_textEdit->log_with_signal(QString::fromStdString(e.message));
});` },
    7: { file: "src/server/main.cpp  ·  SolverServer::status", code: statusFn },
    8: { file: "src/server/main.cpp  ·  SolverServer::dispatch", code:
`if (cmd == "root")
    return backend_.getTreeRoot();
if (cmd == "children")
    return backend_.getTreeChildren(args.at("id").get<std::string>());
if (cmd == "strategy")
    return backend_.getStrategyAt(args.at("id").get<std::string>(), args.value("turn", -1), args.value("river", -1));

if (busy_)
    throw std::runtime_error("busy: " + job_ + " is running");` },
    9: { file: "src/server/main.cpp  ·  SolverServer::status", code: statusFn }
  },
  stop: {
    1: { file: "mainwindow.cpp  ·  MainWindow::on_stopSolvingButton_clicked", code:
`void MainWindow::on_stopSolvingButton_clicked() {
    if (!backend_)
        return;
    try {
        backend_->stop();
    } catch (const std::exception& error) {
        logError(QString::fromStdString(error.what()));
    }
}` },
    2: { file: "src/api/remote_backend.cpp  ·  RemoteBackend::stop", code:
`void RemoteBackend::stop()
{
    call("stop");
}` },
    3: { file: "src/server/main.cpp  ·  SolverServer::dispatch", code:
`if (cmd == "stop") {
    if (busy_ && job_ == "solve")
        backend_.stop();
    return nullptr;
}` },
    4: { file: "src/runtime/PokerSolver.cpp  ·  PokerSolver::stop", code:
`void PokerSolver::stop() {
    if (this->solver != nullptr) {
        this->solver->stop();
    }
}

void PCfrSolver::stop() {
    this->nowstop = true;
}` },
    5: { file: "src/server/main.cpp  ·  onReadyRead", code: replyWrite }
  },
  strategy: {
    1: { file: "mainwindow.cpp  ·  MainWindow::on_showResultButton_clicked", code:
`if (!backend_->hasGameTree()) {
    this->ui->logOutput->log_with_signal(tr("Please build tree first."));
    return;
}
this->strategyExplorer = new StrategyExplorer(this, this->backend_.get());` },
    2: { file: "src/server/main.cpp  ·  dispatch has_tree", code:
`if (cmd == "has_tree")
    return loaded_ && backend_.hasGameTree();` },
    3: { file: "src/api/remote_backend.cpp  ·  RemoteBackend::getStrategyAt", code:
`StrategyGridDto RemoteBackend::getStrategyAt(const std::string& node_id, int turn_card_int, int river_card_int) const
{
    return call("strategy", {{"id", node_id}, {"turn", turn_card_int}, {"river", river_card_int}})
        .get<StrategyGridDto>();
}` },
    4: { file: "src/api/local_backend.cpp  ·  LocalBackend::getStrategyAt", code:
`dto.strategy = cfr->get_strategy(action_node, deal_cards);
dto.evs = cfr->get_evs(action_node, deal_cards);
dto.p1_range[one_private.card1][one_private.card2] = one_private.weight;
dto.p1_combos.emplace_back(one_private.card1, one_private.card2);` },
    5: { file: "src/api/protocol.cpp  ·  to_json(StrategyGridDto)", code:
`void to_json(json& j, const StrategyGridDto& v)
{
    TO(ranks);
    TO(actions);
    TO(current_player);
    TO(root_round);
    TO(node_round);
    TO(strategy);
    TO(evs);
    TO(p1_range);
    TO(p2_range);
    TO(p1_combos);
    TO(p2_combos);
    TO(card_int_to_str);
}` },
    6: { file: "src/ui/tablestrategymodel.cpp  ·  TableStrategyModel::updateStrategyData", code:
`StrategyGridDto grid = backend_->getStrategyAt(treeItem->nodeId(), turn_int, river_int);
current_strategy = grid.strategy;
current_evs = grid.evs;
p1_range = grid.p1_range;
p2_range = grid.p2_range;
actions = grid.actions;
current_player = grid.current_player;` }
  },
  tree: {
    1: { file: "src/ui/treemodel.cpp  ·  TreeModel::setupModelData", code:
`const TreeNodeDto root = backend_->getTreeRoot();
rootItem_ = new TreeItem(root);
TreeItem* visible_root = new TreeItem(root, rootItem_);
rootItem_->insertChild(visible_root);
reGenerateTreeItem(root.round, visible_root);` },
    2: { file: "src/api/local_backend.cpp  ·  LocalBackend::getTreeRoot", code:
`TreeNodeDto LocalBackend::getTreeRoot() const {
    if (!hasGameTree())
        throw runtime_error("no game tree loaded");
    shared_ptr<GameTreeNode> root = solver_.get_game_tree()->getRoot();
    return toDto(root, "0");
}` },
    3: { file: "src/server/main.cpp  ·  onReadyRead", code: replyWrite },
    4: { file: "src/ui/treemodel.cpp  ·  TreeModel::reGenerateTreeItem", code:
`const std::vector<TreeNodeDto> children = backend_->getTreeChildren(node_to_process->nodeId());
for (const TreeNodeDto& child_dto : children) {
    TreeItem* child_node = new TreeItem(child_dto, node_to_process);
    node_to_process->insertChild(child_node);
    if (child_dto.round != round)
        continue;
    reGenerateTreeItem(round, child_node);
}` },
    5: { file: "src/api/local_backend.cpp  ·  LocalBackend::getTreeChildren", code:
`if (node->getType() == GameTreeNode::GameTreeNodeType::ACTION) {
    const vector<shared_ptr<GameTreeNode>>& kids = action->getChildrens();
    for (std::size_t i = 0; i < kids.size(); i++) {
        string child_id = node_id + "/" + std::to_string(i);
        children.push_back(toDto(kids[i], child_id));
    }
}` },
    6: { file: "src/server/main.cpp  ·  onReadyRead", code: replyWrite }
  },
  estimate: {
    1: { file: "mainwindow.cpp  ·  MainWindow::on_estimateMemoryButtom_clicked", code:
`estimate_request.range_ip = this->ui->ipRangeText->toPlainText().toStdString();
estimate_request.range_oop = this->ui->oopRangeText->toPlainText().toStdString();
estimate_request.board = this->ui->boardText->toPlainText().toStdString();
memory_float = backend_->estimateMemory(estimate_request);` },
    2: { file: "src/api/remote_backend.cpp  ·  RemoteBackend::estimateMemory", code:
`long long RemoteBackend::estimateMemory(const SolveRequest& request)
{
    return call("estimate", json(request)).get<long long>();
}` },
    3: { file: "src/server/main.cpp  ·  SolverServer::dispatch", code:
`if (busy_)
    throw std::runtime_error("busy: " + job_ + " is running");
if (cmd == "estimate")
    return backend_.estimateMemory(args.get<SolveRequest>());` },
    4: { file: "src/api/local_backend.cpp  ·  LocalBackend::estimateMemory", code:
`long long LocalBackend::estimateMemory(const SolveRequest& request) {
    return solver_.estimate_tree_memory(QString::fromStdString(request.range_ip),
                                        QString::fromStdString(request.range_oop),
                                        QString::fromStdString(request.board));
}` },
    5: { file: "src/server/main.cpp  ·  onReadyRead", code: replyWrite }
  },
  save: {
    1: { file: "mainwindow.cpp  ·  MainWindow::on_actionjson_triggered", code:
`int dump_rounds = setting.value("dump_round", last_request_.dump_rounds).toInt();
std::string path = fileName.toStdString();
runInBackground([this, path, dump_rounds]() { backend_->saveStrategy(path, dump_rounds); });` },
    2: { file: "src/api/remote_backend.cpp  ·  RemoteBackend::saveStrategy", code:
`void RemoteBackend::saveStrategy(const std::string& path, int dump_rounds)
{
    startJobAndWait("save", {{"path", path}, {"dump_rounds", dump_rounds}});
}` },
    3: { file: "src/api/local_backend.cpp  ·  LocalBackend::saveStrategy", code:
`void LocalBackend::saveStrategy(const string& path, int dump_rounds) {
    solver_.dump_strategy(QString::fromStdString(path), dump_rounds);
}` },
    4: { file: "src/server/main.cpp  ·  onReadyRead", code: replyWrite },
    5: { file: "src/api/remote_backend.cpp  ·  RemoteBackend::startJobAndWait", code:
`if (!status.at("busy").get<bool>()) {
    const std::string error = status.value("error", std::string());
    if (!error.empty())
        throw std::runtime_error(error);
    return;
}
QThread::msleep(kStatusPollMs);` },
    6: { file: "src/server/main.cpp  ·  SolverServer::status", code: statusFn }
  },
  cache: {
    1: { file: "src/ui/tablestrategymodel.cpp  ·  TableStrategyModel::rowCount", code:
`int TableStrategyModel::rowCount(const QModelIndex& parent) const {
    if (backend_ == nullptr)
        return 0;
    return static_cast<int>(backend_->getRanks().size());
}` },
    2: { file: "src/api/remote_backend.cpp  ·  RemoteBackend::info", code:
`RemoteBackend::SessionInfo RemoteBackend::info() const
{
    if (info_valid_)
        return info_;
    const json result = call("info");
    result.at("ranks").get_to(fresh.ranks);
    result.at("deck").get_to(fresh.deck);
    result.at("board").get_to(fresh.board);
    result.at("stack").get_to(fresh.stack);
    info_valid_ = true;
    return info_;
}` },
    3: { file: "src/server/main.cpp  ·  SolverServer::info", code:
`json info() const
{
    return {
        {"ranks", backend_.getRanks()},
        {"deck", backend_.getDeckCards()},
        {"board", backend_.getBoard()},
        {"stack", backend_.getStack()},
    };
}` },
    4: { file: "src/server/main.cpp  ·  onReadyRead", code: replyWrite },
    5: { file: "src/api/remote_backend.cpp  ·  RemoteBackend::invalidateInfo", code:
`void RemoteBackend::invalidateInfo()
{
    std::lock_guard<std::mutex> lock(mutex_);
    info_valid_ = false;
}

void RemoteBackend::load()
{
    invalidateInfo();
    startJobAndWait("load", json::object());
}` }
  }
};
