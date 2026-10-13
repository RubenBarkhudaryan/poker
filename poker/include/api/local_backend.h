#ifndef TEXASSOLVER_API_LOCAL_BACKEND_H
#define TEXASSOLVER_API_LOCAL_BACKEND_H

#include "include/api/isolver_backend.h"

#include "include/runtime/PokerSolver.h"
#include "include/nodes/GameTreeNode.h"
#include "include/nodes/ActionNode.h"
#include "include/nodes/ChanceNode.h"
#include "include/nodes/GameActions.h"
#include "include/tools/GameTreeBuildingSettings.h"
#include "include/tools/StreetSetting.h"

#include <QObject>
#include <map>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <vector>

class LocalBackend : public QObject, public ISolverBackend
{
    private:
        Q_OBJECT

    public:
        explicit LocalBackend(QObject* parent = nullptr);
        ~LocalBackend() override = default;
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
        const SolveRequest& getLastRequest() const;
        TreeNodeDto getTreeRoot() const override;
        std::vector<TreeNodeDto> getTreeChildren(const std::string& node_id) const override;
        StrategyGridDto getStrategyAt(const std::string& node_id,
                                    int turn_card_int,
                                    int river_card_int) const override;
        bool hasSolver() const override;
        bool hasGameTree() const override;

    private:
        PokerSolver solver_;
        SolveRequest last_request_;
        ProgressCallback progress_;
        // buildTree() runs on a worker thread while the explorer reads from the UI thread.
        mutable std::mutex nodes_mutex_;
        mutable std::map<std::string, std::weak_ptr<GameTreeNode>> nodes_;
        std::shared_ptr<GameTreeNode> findNode(const std::string& id) const;
        TreeNodeDto toDto(const std::shared_ptr<GameTreeNode>& node, const std::string& id) const;
        void registerNode(const std::string& id, const std::shared_ptr<GameTreeNode>& node) const;
        void emitProgress(const std::string& message, int iter = 0, float expl = 0.0f) const;
        void clearNodeMap() const;
        static GameRoundDto toRoundDto(GameTreeNode::GameRound round);
        static NodeType toNodeType(GameTreeNode::GameTreeNodeType type);
        static StreetSetting toStreetSetting(const StreetSettingDto& dto);
        static GameTreeBuildingSettings toTreeSettings(const TreeBuildSettingsDto& dto);
        static std::string actionLabel(GameTreeNode::PokerActions action, float amount);
        static std::string nodeLabel(const std::shared_ptr<GameTreeNode>& node);
};


#endif // TEXASSOLVER_API_LOCAL_BACKEND_H
