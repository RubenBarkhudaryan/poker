#ifndef TEXASSOLVER_API_REMOTE_BACKEND_H
#define TEXASSOLVER_API_REMOTE_BACKEND_H

#include "include/api/isolver_backend.h"
#include "include/api/protocol.h"

#include <QString>
#include <mutex>
#include <string>
#include <vector>

// ISolverBackend over TCP to a TexasSolverServer.
// Methods block until the server answers; long jobs (load/build/solve/save)
// block until the job finishes, forwarding server progress to the callback.
// Safe to call from several threads: each call opens its own connection.
class RemoteBackend : public ISolverBackend
{
    public:
        explicit RemoteBackend(QString host = "127.0.0.1", quint16 port = 8080);

        void setProgressCallback(ProgressCallback callback) override;
        void load() override;
        void buildTree(const SolveRequest& request) override;
        long long estimateMemory(const SolveRequest& request) override;
        void solve(const SolveRequest& request) override;
        void stop() override;
        void saveStrategy(const std::string& path, int dump_rounds) override;
        std::vector<std::string> getRanks() const override;
        std::vector<std::string> getDeckCards() const override;
        std::string getBoard() const override;
        float getStack() const override;
        TreeNodeDto getTreeRoot() const override;
        std::vector<TreeNodeDto> getTreeChildren(const std::string& node_id) const override;
        StrategyGridDto getStrategyAt(const std::string& node_id, int turn_card_int, int river_card_int) const override;
        bool hasSolver() const override;
        bool hasGameTree() const override;

        // Polls until the server accepts connections or timeout_ms elapses.
        bool waitUntilReachable(int timeout_ms) const;
        void shutdownServer();

        QString host() const { return host_; }
        quint16 port() const { return port_; }

    private:
        struct SessionInfo
        {
            std::vector<std::string> ranks;
            std::vector<std::string> deck;
            std::string board;
            float stack = 0.0f;
        };

        json call(const std::string& cmd, const json& args = json::object()) const;
        void startJobAndWait(const std::string& cmd, const json& args);
        SessionInfo info() const;
        void invalidateInfo();

        QString host_;
        quint16 port_;

        mutable std::mutex mutex_;
        ProgressCallback progress_;
        // ranks/deck are read once per table cell on every repaint, so they must not hit the network.
        mutable bool info_valid_ = false;
        mutable SessionInfo info_;
};

#endif // TEXASSOLVER_API_REMOTE_BACKEND_H
