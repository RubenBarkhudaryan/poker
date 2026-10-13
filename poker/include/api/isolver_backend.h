#ifndef TEXASSOLVER_API_ISOLVER_BACKEND_H
#define TEXASSOLVER_API_ISOLVER_BACKEND_H

#include "include/api/solver_dtos.h"
#include <functional>
#include <string>
#include <vector>

class ISolverBackend
{
    public:
        virtual ~ISolverBackend() = default;

        // Optional log/progress sink. LocalBackend calls this instead of qDebug into the UI.
        using ProgressCallback = std::function<void(const ProgressEvent&)>;
        virtual void setProgressCallback(ProgressCallback callback) = 0;

        // QSolverJob::loading()
        virtual void load() = 0;

        // QSolverJob::build_tree() using fields from SolveRequest + tree_settings
        virtual void buildTree(const SolveRequest& request) = 0;

        // QSolverJob::estimate_tree_memory(range1, range2, board)
        virtual long long estimateMemory(const SolveRequest& request) = 0;

        // QSolverJob::solving() / stop()
        virtual void solve(const SolveRequest& request) = 0;
        virtual void stop() = 0;

        // QSolverJob::saving() / dump_strategy
        virtual void saveStrategy(const std::string& path, int dump_rounds) = 0;

        // Deck ranks for the 13x13 grid
        virtual std::vector<std::string> getRanks() const = 0;

        // Cards in deck as "Ah"-style strings, for turn/river combo boxes
        virtual std::vector<std::string> getDeckCards() const = 0;

        // Current board string from last SolveRequest / session
        virtual std::string getBoard() const = 0;

        // Stack from last SolveRequest (for EV normalization in UI)
        virtual float getStack() const = 0;

        // Tree navigation (replaces TreeItem::m_treedata + TreeModel walking)
        virtual TreeNodeDto getTreeRoot() const = 0;
        virtual std::vector<TreeNodeDto> getTreeChildren(const std::string& node_id) const = 0;

        // Strategy at an action node.
        // turn_card_int / river_card_int: Card::getCardInt(), or -1 if unused
        // (same role as TableStrategyModel::turn_card / river_card)
        virtual StrategyGridDto getStrategyAt(const std::string& node_id,
                                            int turn_card_int,
                                            int river_card_int) const = 0;

        virtual bool hasSolver() const = 0;
        virtual bool hasGameTree() const = 0;
};

#endif // TEXASSOLVER_API_ISOLVER_BACKEND_H
