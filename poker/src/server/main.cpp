#include "include/api/local_backend.h"
#include "include/api/protocol.h"

#include <QCoreApplication>
#include <QHostAddress>
#include <QTcpServer>
#include <QTcpSocket>

#include <atomic>
#include <deque>
#include <functional>
#include <iostream>
#include <mutex>
#include <thread>

class SolverServer
{
    public:
        SolverServer()
        {
            backend_.setProgressCallback([this](const ProgressEvent& event) { pushProgress(event); });
            QObject::connect(&server_, &QTcpServer::newConnection, [this]() { onNewConnection(); });
        }

        ~SolverServer()
        {
            if (worker_.joinable()) {
                if (job_ == "solve")
                    backend_.stop();
                worker_.join();
            }
        }

        bool listen(const QHostAddress& address, quint16 port)
        {
            return server_.listen(address, port);
        }

        QString errorString() const
        {
            return server_.errorString();
        }

        // Called from solver/OpenMP threads through the Qt message handler.
        void pushProgress(const ProgressEvent& event)
        {
            std::lock_guard<std::mutex> lock(mutex_);
            std::cout << event.message << std::endl;
            events_.push_back(event);
            if (events_.size() > 2000)
                events_.pop_front();
        }

    private:
        void onNewConnection()
        {
            while (QTcpSocket* socket = server_.nextPendingConnection()) {
                QObject::connect(socket, &QTcpSocket::readyRead, socket, [this, socket]() { onReadyRead(socket); });
                QObject::connect(socket, &QTcpSocket::disconnected, socket, &QObject::deleteLater);
            }
        }

        void onReadyRead(QTcpSocket* socket)
        {
            while (socket->canReadLine()) {
                const QByteArray line = socket->readLine().trimmed();
                if (line.isEmpty())
                    continue;

                QByteArray encoded;
                try {
                    const json request = json::parse(line.toStdString());
                    const json args = request.value("args", json::object());
                    const json result = dispatch(request.at("cmd").get<std::string>(), args);
                    encoded = encodeMessage({{"ok", true}, {"result", result}});
                } catch (const std::exception& error) {
                    encoded = encodeMessage({{"ok", false}, {"error", error.what()}});
                }
                socket->write(encoded);
            }
        }

        json dispatch(const std::string& cmd, const json& args)
        {
            if (cmd == "status")
                return status();
            if (cmd == "stop") {
                if (busy_ && job_ == "solve")
                    backend_.stop();
                return nullptr;
            }

            // build/load replace the game tree, so nothing may read it meanwhile.
            // solve/save only touch strategy data, which the explorer is allowed to poll live.
            const bool tree_changing = busy_ && (job_ == "build" || job_ == "load");
            if (tree_changing)
                throw std::runtime_error("busy: " + job_ + " is running");

            if (cmd == "has_solver")
                return loaded_ && backend_.hasSolver();
            if (cmd == "has_tree")
                return loaded_ && backend_.hasGameTree();
            if (cmd == "shutdown") {
                if (busy_)
                    throw std::runtime_error("busy: " + job_ + " is running");
                QCoreApplication::quit();
                return nullptr;
            }
            if (cmd != "load")
                requireLoaded();

            if (cmd == "ranks")
                return backend_.getRanks();
            if (cmd == "deck")
                return backend_.getDeckCards();
            if (cmd == "board")
                return backend_.getBoard();
            if (cmd == "stack")
                return backend_.getStack();
            if (cmd == "info")
                return info();
            if (cmd == "root")
                return backend_.getTreeRoot();
            if (cmd == "children")
                return backend_.getTreeChildren(args.at("id").get<std::string>());
            if (cmd == "strategy")
                return backend_.getStrategyAt(args.at("id").get<std::string>(), args.value("turn", -1),
                                              args.value("river", -1));

            if (busy_)
                throw std::runtime_error("busy: " + job_ + " is running");

            // Long-running jobs reply immediately; the client polls "status" until busy is false.
            if (cmd == "load") {
                if (loaded_)
                    return nullptr;
                startJob("load", [this]() {
                    backend_.load();
                    loaded_ = true;
                });
                return nullptr;
            }
            if (cmd == "build") {
                const SolveRequest request = args.get<SolveRequest>();
                startJob("build", [this, request]() { backend_.buildTree(request); });
                return nullptr;
            }
            if (cmd == "solve") {
                const SolveRequest request = args.get<SolveRequest>();
                startJob("solve", [this, request]() { backend_.solve(request); });
                return nullptr;
            }
            if (cmd == "save") {
                const std::string path = args.at("path").get<std::string>();
                const int dump_rounds = args.value("dump_rounds", 2);
                startJob("save", [this, path, dump_rounds]() { backend_.saveStrategy(path, dump_rounds); });
                return nullptr;
            }
            if (cmd == "estimate")
                return backend_.estimateMemory(args.get<SolveRequest>());

            throw std::runtime_error("unknown command: " + cmd);
        }

        void requireLoaded() const
        {
            if (!loaded_)
                throw std::runtime_error("solver resources are not loaded yet");
        }

        json info() const
        {
            return {
                {"ranks", backend_.getRanks()},
                {"deck", backend_.getDeckCards()},
                {"board", backend_.getBoard()},
                {"stack", backend_.getStack()},
            };
        }

        void startJob(const std::string& name, std::function<void()> task)
        {
            if (worker_.joinable())
                worker_.join();

            job_ = name;
            {
                std::lock_guard<std::mutex> lock(mutex_);
                last_error_.clear();
            }
            busy_ = true;

            worker_ = std::thread([this, task]() {
                try {
                    task();
                } catch (const std::exception& error) {
                    std::lock_guard<std::mutex> lock(mutex_);
                    last_error_ = error.what();
                }
                busy_ = false;
            });
        }

        json status()
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
        }

        LocalBackend backend_;
        QTcpServer server_;

        std::thread worker_;
        std::atomic<bool> busy_{false};
        std::atomic<bool> loaded_{false};
        // Written and read only on the event-loop thread.
        std::string job_;

        std::mutex mutex_;
        std::deque<ProgressEvent> events_;
        std::string last_error_;
};

static SolverServer* g_server = nullptr;

static void messageHandler(QtMsgType, const QMessageLogContext&, const QString& message)
{
    if (g_server != nullptr)
        g_server->pushProgress({0, 0.0f, message.toStdString()});
    else
        std::cerr << message.toStdString() << std::endl;
}

// Usage: TexasSolverServer [port] [bind-address]
// Defaults to 127.0.0.1:8080. Pass 0.0.0.0 to accept connections from other machines.
int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    const quint16 port = argc > 1 ? QString(argv[1]).toUShort() : 8080;
    const QHostAddress address = argc > 2 ? QHostAddress(QString(argv[2])) : QHostAddress(QHostAddress::LocalHost);

    SolverServer server;
    g_server = &server;
    qInstallMessageHandler(messageHandler);

    if (!server.listen(address, port)) {
        std::cerr << "listen failed: " << server.errorString().toStdString() << std::endl;
        return 1;
    }
    std::cout << "listening on " << address.toString().toStdString() << ":" << port << std::endl;

    const int code = app.exec();
    qInstallMessageHandler(nullptr);
    g_server = nullptr;
    return code;
}
