#include "include/ui/tablestrategymodel.h"
#include <stdexcept>
#include <algorithm>

using std::pair;
using std::runtime_error;
using std::string;
using std::vector;

TableStrategyModel::TableStrategyModel(ISolverBackend* backend, QObject* parent)
    : QAbstractItemModel(parent), backend_(backend) {
    setupModelData();
}

TableStrategyModel::~TableStrategyModel() {}

QModelIndex TableStrategyModel::index(int row, int column, const QModelIndex& parent) const {
    if (!hasIndex(row, column, parent))
        return QModelIndex();
    return createIndex(row, column, nullptr);
}

QVariant TableStrategyModel::headerData(int section, Qt::Orientation orientation, int role) const {
    Q_UNUSED(section);
    Q_UNUSED(orientation);
    Q_UNUSED(role);
    return QString();
}

QModelIndex TableStrategyModel::parent(const QModelIndex& child) const {
    Q_UNUSED(child);
    return QModelIndex();
}

int TableStrategyModel::columnCount(const QModelIndex& parent) const {
    Q_UNUSED(parent);
    if (backend_ == nullptr)
        return 0;
    return static_cast<int>(backend_->getRanks().size());
}

int TableStrategyModel::rowCount(const QModelIndex& parent) const {
    Q_UNUSED(parent);
    if (backend_ == nullptr)
        return 0;
    return static_cast<int>(backend_->getRanks().size());
}

QVariant TableStrategyModel::data(const QModelIndex& index, int role) const {
    Q_UNUSED(role);
    vector<string> ranks = backend_->getRanks();
    int row = index.row();
    int col = index.column();
    int larger = row > col ? row : col;
    int smaller = row > col ? col : row;
    QString retval = QString("<h4>%1%2<b>%3</b></h4>")
                         .arg(QString::fromStdString(ranks[ranks.size() - 1 - smaller]))
                         .arg(QString::fromStdString(ranks[ranks.size() - 1 - larger]));
    if (row > col)
        retval = retval.arg(tr("o"));
    else if (row < col)
        retval = retval.arg(tr("s"));
    else
        retval = retval.arg(tr(" "));
    return retval;
}

void TableStrategyModel::setupModelData() {
    if (backend_ == nullptr)
        return;

    cardint2card.clear();
    vector<string> ranks = backend_->getRanks();
    vector<string> deck_cards = backend_->getDeckCards();
    for (const string& card_str : deck_cards) {
        Card one_card(card_str);
        cardint2card.insert(pair<int, Card>(one_card.getCardInt(), one_card));
    }

    const std::size_t n = ranks.size();
    ui_strategy_table.assign(n, vector<vector<pair<int, int>>>(n));
    ui_p1_range.assign(n, vector<vector<pair<int, int>>>(n));
    ui_p2_range.assign(n, vector<vector<pair<int, int>>>(n));
    p1_range.assign(52, vector<float>(52, 0.0f));
    p2_range.assign(52, vector<float>(52, 0.0f));
    total_strategy.clear();
    actions.clear();
}

void TableStrategyModel::addToGrid(int card_int1, int card_int2, vector<vector<vector<pair<int, int>>>>& table) {
    auto it1 = cardint2card.find(card_int1);
    auto it2 = cardint2card.find(card_int2);
    if (it1 == cardint2card.end() || it2 == cardint2card.end())
        return;
    const int n = static_cast<int>(table.size());
    int rank1 = it1->second.getCardInt() / 4;
    int suit1 = it1->second.getCardInt() - rank1 * 4;
    int index1 = 12 - rank1;
    int rank2 = it2->second.getCardInt() / 4;
    int suit2 = it2->second.getCardInt() - rank2 * 4;
    int index2 = 12 - rank2;
    if (index1 < 0 || index2 < 0 || index1 >= n || index2 >= n)
        return;
    if (index1 == index2)
        table[index1][index2].push_back(pair<int, int>(card_int1, card_int2));
    else if (suit1 == suit2)
        table[std::min(index1, index2)][std::max(index1, index2)].push_back(pair<int, int>(card_int1, card_int2));
    else
        table[std::max(index1, index2)][std::min(index1, index2)].push_back(pair<int, int>(card_int1, card_int2));
}

void TableStrategyModel::rebuildUiStrategyTable() {
    const std::size_t n = backend_->getRanks().size();
    ui_strategy_table.assign(n, vector<vector<pair<int, int>>>(n));

    for (int i = 0; i < static_cast<int>(current_strategy.size()); i++) {
        for (int j = 0; j < static_cast<int>(current_strategy[i].size()); j++) {
            if (!current_strategy[i][j].empty())
                addToGrid(i, j, ui_strategy_table);
        }
    }
}

void TableStrategyModel::rebuildUiRangeTable(const vector<pair<int, int>>& combos,
                                             vector<vector<vector<pair<int, int>>>>& table) {
    const std::size_t n = backend_->getRanks().size();
    table.assign(n, vector<vector<pair<int, int>>>(n));
    for (const pair<int, int>& combo : combos)
        addToGrid(combo.first, combo.second, table);
}

void TableStrategyModel::clicked_event(const QModelIndex& index) {
    Q_UNUSED(index);
}

void TableStrategyModel::setGameTreeNode(TreeItem* treeNode) {
    this->treeItem = treeNode;
}

void TableStrategyModel::setTrunCard(Card turn_card) {
    this->turn_card = turn_card;
}

void TableStrategyModel::setRiverCard(Card river_card) {
    this->river_card = river_card;
}

void TableStrategyModel::updateStrategyData() {
    if (treeItem == nullptr || backend_ == nullptr)
        return;

    setupModelData();

    const int turn_int = turn_card.empty() ? -1 : turn_card.getCardInt();
    const int river_int = river_card.empty() ? -1 : river_card.getCardInt();
    StrategyGridDto grid = backend_->getStrategyAt(treeItem->nodeId(), turn_int, river_int);

    current_strategy = grid.strategy;
    current_evs = grid.evs;
    if (!grid.p1_range.empty())
        p1_range = grid.p1_range;
    if (!grid.p2_range.empty())
        p2_range = grid.p2_range;
    actions = grid.actions;
    current_player = grid.current_player;

    for (std::size_t i = 0; i < grid.card_int_to_str.size(); i++) {
        if (!grid.card_int_to_str[i].empty())
            cardint2card[static_cast<int>(i)] = Card(grid.card_int_to_str[i]);
    }

    rebuildUiStrategyTable();
    rebuildUiRangeTable(grid.p1_combos, ui_p1_range);
    rebuildUiRangeTable(grid.p2_combos, ui_p2_range);
    total_strategy = get_total_strategy();
}

const vector<pair<ActionLabelDto, pair<float, float>>> TableStrategyModel::get_total_strategy() const {
    vector<pair<ActionLabelDto, pair<float, float>>> ret_strategy;
    if (treeItem == nullptr || actions.empty() || current_strategy.empty())
        return ret_strategy;

    vector<float> combos(actions.size(), 0.0f);
    vector<float> avg_strategy(actions.size(), 0.0f);
    float sum_strategy = 0.0f;
    const vector<vector<float>>& range = current_player == 0 ? p1_range : p2_range;

    for (std::size_t index1 = 0; index1 < current_strategy.size(); index1++) {
        for (std::size_t index2 = 0; index2 < current_strategy[index1].size(); index2++) {
            const vector<float>& one_strategy = current_strategy[index1][index2];
            if (one_strategy.empty() || one_strategy.size() != actions.size())
                continue;
            if (range.size() <= index1 || range[index1].size() <= index2)
                continue;
            const float one_range = range[index1][index2];
            for (std::size_t i = 0; i < one_strategy.size(); i++) {
                combos[i] += one_strategy[i] * one_range;
                avg_strategy[i] += one_strategy[i] * one_range;
                sum_strategy += one_strategy[i] * one_range;
            }
        }
    }

    if (sum_strategy <= 0.0f)
        return ret_strategy;

    for (std::size_t i = 0; i < actions.size(); i++) {
        avg_strategy[i] /= sum_strategy;
        ret_strategy.push_back(pair<ActionLabelDto, pair<float, float>>(
            actions[i], pair<float, float>(combos[i], avg_strategy[i])));
    }
    return ret_strategy;
}

const vector<pair<ActionLabelDto, float>> TableStrategyModel::get_strategy(int i, int j) const {
    vector<pair<ActionLabelDto, float>> ret_strategy;
    if (treeItem == nullptr || actions.empty())
        return ret_strategy;
    if (i < 0 || j < 0 || i >= static_cast<int>(ui_strategy_table.size()) ||
        j >= static_cast<int>(ui_strategy_table[i].size()))
        return ret_strategy;

    const vector<pair<int, int>>& card_cords = ui_strategy_table[i][j];
    if (card_cords.empty())
        return ret_strategy;

    const vector<vector<float>>* current_range = (current_player == 0) ? &p1_range : &p2_range;
    if (p1_range.empty() || p2_range.empty())
        return ret_strategy;

    float range_number = 0.0f;
    for (const auto& cord : card_cords)
        range_number += (*current_range)[cord.first][cord.second];
    range_number /= static_cast<float>(card_cords.size());
    if (range_number < 0.0f || range_number > 1.0f)
        throw runtime_error("range number incorrect in tablestrategymodel");

    vector<float> strategies(actions.size(), 0.0f);
    const int strategy_number = static_cast<int>(card_cords.size());
    for (const auto& index : card_cords) {
        const vector<float>& one_strategy = current_strategy[index.first][index.second];
        if (one_strategy.size() != actions.size())
            throw runtime_error("size not match between actions and strategy");
        if (range_number > 0.0f) {
            for (std::size_t indi = 0; indi < one_strategy.size(); indi++) {
                strategies[indi] += one_strategy[indi] * (*current_range)[index.first][index.second] /
                                    range_number / static_cast<float>(strategy_number);
            }
        }
    }
    for (std::size_t indi = 0; indi < strategies.size(); indi++)
        ret_strategy.push_back(pair<ActionLabelDto, float>(actions[indi], strategies[indi]));
    return ret_strategy;
}

const vector<float> TableStrategyModel::get_ev_grid(int i, int j) const {
    vector<float> ret_evs;
    if (treeItem == nullptr || current_evs.empty())
        return ret_evs;
    if (i < 0 || j < 0 || i >= static_cast<int>(ui_strategy_table.size()) ||
        j >= static_cast<int>(ui_strategy_table[i].size()))
        return ret_evs;

    for (const auto& index : ui_strategy_table[i][j]) {
        const vector<float>& one_strategy = current_strategy[index.first][index.second];
        const vector<float>& one_ev = current_evs[index.first][index.second];
        if (one_ev.size() != one_strategy.size())
            return vector<float>();
        float one_ev_float = 0.0f;
        for (std::size_t indi = 0; indi < one_strategy.size(); indi++)
            one_ev_float += one_strategy[indi] * one_ev[indi];
        ret_evs.push_back(one_ev_float);
    }
    return ret_evs;
}

const vector<float> TableStrategyModel::get_strategies_evs(int i, int j) const {
    vector<float> ret_evs;
    if (treeItem == nullptr || current_evs.empty() || actions.empty())
        return ret_evs;
    if (i < 0 || j < 0 || i >= static_cast<int>(ui_strategy_table.size()) ||
        j >= static_cast<int>(ui_strategy_table[i].size()))
        return ret_evs;

    const vector<vector<float>>* current_range = (current_player == 0) ? &p1_range : &p2_range;
    if (p1_range.empty() || p2_range.empty())
        return ret_evs;

    ret_evs.assign(actions.size(), 0.0f);
    vector<float> strategy_p(actions.size(), 0.0f);
    for (const auto& index : ui_strategy_table[i][j]) {
        const vector<float>& one_strategy = current_strategy[index.first][index.second];
        const vector<float>& one_ev = current_evs[index.first][index.second];
        const float one_range = (*current_range)[index.first][index.second];
        if (one_strategy.size() != actions.size() || one_ev.size() != one_strategy.size())
            throw runtime_error("size not match between one_ev, actions and one_strategy");
        for (std::size_t indi = 0; indi < ret_evs.size(); indi++) {
            ret_evs[indi] += one_strategy[indi] * one_ev[indi] * one_range;
            strategy_p[indi] += one_strategy[indi] * one_range;
        }
    }
    for (std::size_t indi = 0; indi < ret_evs.size(); indi++) {
        if (strategy_p[indi] > 0.0f)
            ret_evs[indi] /= strategy_p[indi];
    }
    return ret_evs;
}
