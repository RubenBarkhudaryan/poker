#ifndef TABLESTRATEGYMODEL_H
#define TABLESTRATEGYMODEL_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>
#include "include/api/isolver_backend.h"
#include "include/api/solver_dtos.h"
#include "include/ui/treeitem.h"
#include "include/Card.h"
#include <map>
#include <string>
#include <utility>
#include <vector>

class TableStrategyModel : public QAbstractItemModel {
    Q_OBJECT

public:
    explicit TableStrategyModel(ISolverBackend* backend, QObject* parent = nullptr);
    ~TableStrategyModel() override;

    QVariant data(const QModelIndex& index, int role) const override;
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QModelIndex parent(const QModelIndex& child) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    void setGameTreeNode(TreeItem* treeItem);
    void setTrunCard(Card turn_card);
    void setRiverCard(Card river_card);

    Card getTrunCard() { return turn_card; }
    Card getRiverCard() { return river_card; }

    void updateStrategyData();

    const std::vector<std::pair<ActionLabelDto, float>> get_strategy(int i, int j) const;
    const std::vector<std::pair<ActionLabelDto, std::pair<float, float>>> get_total_strategy() const;
    const std::vector<float> get_ev_grid(int i, int j) const;
    const std::vector<float> get_strategies_evs(int i, int j) const;

    std::vector<std::vector<std::vector<float>>> current_strategy;
    std::vector<std::vector<std::vector<float>>> current_evs;
    std::vector<std::vector<float>> p1_range;
    std::vector<std::vector<float>> p2_range;
    std::vector<std::vector<std::vector<std::pair<int, int>>>> ui_strategy_table;
    std::vector<std::vector<std::vector<std::pair<int, int>>>> ui_p1_range;
    std::vector<std::vector<std::vector<std::pair<int, int>>>> ui_p2_range;
    std::vector<std::pair<ActionLabelDto, std::pair<float, float>>> total_strategy;
    std::map<int, Card> cardint2card;
    TreeItem* treeItem = nullptr;
    std::vector<ActionLabelDto> actions;
    int current_player = 0;

    ISolverBackend* get_backend() { return backend_; }

private:
    ISolverBackend* backend_;
    void setupModelData();
    void rebuildUiStrategyTable();
    void rebuildUiRangeTable(const std::vector<std::pair<int, int>>& combos,
                             std::vector<std::vector<std::vector<std::pair<int, int>>>>& table);
    void addToGrid(int card_int1, int card_int2,
                   std::vector<std::vector<std::vector<std::pair<int, int>>>>& table);
    Card turn_card;
    Card river_card;

public slots:
    void clicked_event(const QModelIndex& index);
};

#endif
