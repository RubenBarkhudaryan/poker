#include "include/api/remote_backend.h"

#include <QElapsedTimer>
#include <QTcpSocket>
#include <QThread>
#include <stdexcept>

namespace
{
    constexpr int kConnectTimeoutMs = 3000;
    constexpr int kReplyTimeoutMs = 60000;
    constexpr int kStatusPollMs = 300;
}

RemoteBackend::RemoteBackend(QString host, quint16 port) : host_(std::move(host)), port_(port) {}

json RemoteBackend::call(const std::string& cmd, const json& args) const
{
    QTcpSocket socket;
    socket.connectToHost(host_, port_);
    if (!socket.waitForConnected(kConnectTimeoutMs))
        throw std::runtime_error("cannot reach solver server at " + host_.toStdString() + ":" +
                                 std::to_string(port_) + ": " + socket.errorString().toStdString());

    socket.write(encodeMessage({{"cmd", cmd}, {"args", args}}));
    if (!socket.waitForBytesWritten(kConnectTimeoutMs))
        throw std::runtime_error("sending '" + cmd + "' failed: " + socket.errorString().toStdString());

    while (!socket.canReadLine()) {
        if (!socket.waitForReadyRead(kReplyTimeoutMs))
            throw std::runtime_error("no reply from solver server for '" + cmd + "'");
    }

    json reply;
    try {
        reply = json::parse(socket.readLine().toStdString());
    } catch (const json::exception& error) {
        throw std::runtime_error(std::string("malformed reply from solver server: ") + error.what());
    }
    socket.disconnectFromHost();

    if (!reply.value("ok", false))
        throw std::runtime_error(reply.value("error", std::string("unknown server error")));
    return reply.value("result", json());
}

void RemoteBackend::startJobAndWait(const std::string& cmd, const json& args)
{
    call(cmd, args);
    for (;;) {
        const json status = call("status");

        ProgressCallback progress;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            progress = progress_;
        }
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
}

bool RemoteBackend::waitUntilReachable(int timeout_ms) const
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
}

void RemoteBackend::shutdownServer()
{
    call("shutdown");
}

RemoteBackend::SessionInfo RemoteBackend::info() const
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (info_valid_)
            return info_;
    }

    const json result = call("info");
    SessionInfo fresh;
    result.at("ranks").get_to(fresh.ranks);
    result.at("deck").get_to(fresh.deck);
    result.at("board").get_to(fresh.board);
    result.at("stack").get_to(fresh.stack);

    std::lock_guard<std::mutex> lock(mutex_);
    info_ = fresh;
    info_valid_ = true;
    return info_;
}

void RemoteBackend::invalidateInfo()
{
    std::lock_guard<std::mutex> lock(mutex_);
    info_valid_ = false;
}

void RemoteBackend::setProgressCallback(ProgressCallback callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    progress_ = std::move(callback);
}

void RemoteBackend::load()
{
    invalidateInfo();
    startJobAndWait("load", json::object());
}

void RemoteBackend::buildTree(const SolveRequest& request)
{
    invalidateInfo();
    startJobAndWait("build", json(request));
}

void RemoteBackend::solve(const SolveRequest& request)
{
    invalidateInfo();
    startJobAndWait("solve", json(request));
}

void RemoteBackend::stop()
{
    call("stop");
}

void RemoteBackend::saveStrategy(const std::string& path, int dump_rounds)
{
    startJobAndWait("save", {{"path", path}, {"dump_rounds", dump_rounds}});
}

long long RemoteBackend::estimateMemory(const SolveRequest& request)
{
    return call("estimate", json(request)).get<long long>();
}

std::vector<std::string> RemoteBackend::getRanks() const
{
    return info().ranks;
}

std::vector<std::string> RemoteBackend::getDeckCards() const
{
    return info().deck;
}

std::string RemoteBackend::getBoard() const
{
    return info().board;
}

float RemoteBackend::getStack() const
{
    return info().stack;
}

bool RemoteBackend::hasSolver() const
{
    return call("has_solver").get<bool>();
}

bool RemoteBackend::hasGameTree() const
{
    return call("has_tree").get<bool>();
}

TreeNodeDto RemoteBackend::getTreeRoot() const
{
    return call("root").get<TreeNodeDto>();
}

std::vector<TreeNodeDto> RemoteBackend::getTreeChildren(const std::string& node_id) const
{
    return call("children", {{"id", node_id}}).get<std::vector<TreeNodeDto>>();
}

StrategyGridDto RemoteBackend::getStrategyAt(const std::string& node_id, int turn_card_int, int river_card_int) const
{
    return call("strategy", {{"id", node_id}, {"turn", turn_card_int}, {"river", river_card_int}})
        .get<StrategyGridDto>();
}
