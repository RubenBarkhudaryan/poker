#include "include/ui/strategyitemdelegate.h"
#include <QPainter>
#include <QTextDocument>
#include <QAbstractTextDocumentLayout>
#include <QRect>
#include <QBrush>

namespace {
bool isFoldLabel(const ActionLabelDto& action) {
    return action.label == "FOLD";
}
bool isCheckOrCallLabel(const ActionLabelDto& action) {
    return action.label == "CHECK" || action.label == "CALL";
}
bool isBetOrRaiseLabel(const ActionLabelDto& action) {
    return action.label.rfind("BET", 0) == 0 || action.label.rfind("RAISE", 0) == 0;
}
} // namespace

StrategyItemDelegate::StrategyItemDelegate(ISolverBackend* backend, DetailWindowSetting* detailWindowSetting,
                                           QObject* parent)
    : WordItemDelegate(parent) {
    this->detailWindowSetting = detailWindowSetting;
    this->backend_ = backend;
}

void StrategyItemDelegate::paint_strategy(QPainter* painter, const QStyleOptionViewItem& option,
                                          const QModelIndex& index, bool withEVs) const {
    auto options = option;
    initStyleOption(&options, index);
    const TableStrategyModel* tableStrategyModel = qobject_cast<const TableStrategyModel*>(index.model());
    if (tableStrategyModel->treeItem != NULL &&
        tableStrategyModel->treeItem->node().node_type == NodeType::Action) {
        const TreeNodeDto& node = tableStrategyModel->treeItem->node();
        vector<pair<ActionLabelDto, float>> strategy = tableStrategyModel->get_strategy(index.row(), index.column());
        vector<float> evs = tableStrategyModel->get_strategies_evs(index.row(), index.column());
        if (!strategy.empty()) {
            float fold_prob = 0;
            vector<float> strategy_without_fold;
            vector<float> evs_without_fold;
            float strategy_without_fold_sum = 0;
            for (std::size_t i = 0; i < strategy.size(); i++) {
                const ActionLabelDto& one_action = strategy[i].first;
                if (isFoldLabel(one_action)) {
                    fold_prob = strategy[i].second;
                } else {
                    strategy_without_fold.push_back(strategy[i].second);
                    strategy_without_fold_sum += strategy[i].second;
                    if (i < evs.size())
                        evs_without_fold.push_back(evs[i]);
                }
            }
            if (strategy_without_fold_sum > 0.) {
                for (std::size_t i = 0; i < strategy_without_fold.size(); i++) {
                    strategy_without_fold[i] = strategy_without_fold[i] / strategy_without_fold_sum;
                }
            }

            vector<pair<int, int>> card_cords;
            card_cords = tableStrategyModel->ui_strategy_table[index.row()][index.column()];
            float range_number = 0;
            if (!card_cords.empty()) {
                for (auto one_cord : card_cords) {
                    if (0 == tableStrategyModel->current_player) {
                        range_number += tableStrategyModel->p1_range[one_cord.first][one_cord.second];
                    } else {
                        range_number += tableStrategyModel->p2_range[one_cord.first][one_cord.second];
                    }
                }
                range_number = range_number / card_cords.size();
                if (range_number < 0 || range_number > 1)
                    throw runtime_error("range number incorrect in strategyitemdeletage");
            }
            float not_in_range = (this->detailWindowSetting->grid_i == index.row()) &&
                                         (this->detailWindowSetting->grid_j == index.column())
                                     ? 0
                                     : 1 - range_number;
            int niR_height = (int)(not_in_range * option.rect.height() * 0.95);

            int disable_height = (int)(0.5 + fold_prob * (option.rect.height() - niR_height));
            int remain_height = option.rect.height() - niR_height - disable_height;

            if (disable_height > 0) {
                QRect rect(option.rect.left(), option.rect.top() + niR_height, option.rect.width(), disable_height);
                QBrush brush(QColor(0, 191, 255));
                painter->fillRect(rect, brush);
            }

            if (remain_height > 0) {
                int ind = 0;
                float last_prob = 0;
                int bet_raise_num = 0;
                for (std::size_t i = 0; i < strategy.size(); i++) {
                    const ActionLabelDto& one_action = strategy[i].first;
                    float normalized_ev =
                        withEVs && ind < static_cast<int>(evs_without_fold.size())
                            ? normalization_tanh(node.pot * 3, evs_without_fold[ind])
                            : 1.;
                    QBrush brush(Qt::gray);
                    if (!isFoldLabel(one_action)) {
                        if (isCheckOrCallLabel(one_action)) {
                            int green = 255;
                            int blue = max((int)(225 - normalized_ev * 175), 55);
                            int red = 55;
                            brush = QBrush(QColor(red, green, blue));
                        } else if (isBetOrRaiseLabel(one_action)) {
                            int color_base = max(128 - 32 * bet_raise_num - 1, 0);
                            int blue = max((int)(255 - normalized_ev * (255 - color_base)), color_base);
                            int red = color_base + min((int)(normalized_ev * (255 - color_base)), 255 - color_base);
                            int green = color_base;
                            brush = QBrush(QColor(red, green, blue));
                            bet_raise_num += 1;
                        } else {
                            brush = QBrush(Qt::blue);
                        }

                        int delta_x = (int)(option.rect.width() * last_prob);
                        int delta_width = (int)(option.rect.width() * (last_prob + strategy_without_fold[ind])) -
                                          (int)(option.rect.width() * last_prob);

                        QRect rect(option.rect.left() + delta_x, option.rect.top() + niR_height + disable_height,
                                   delta_width, remain_height);
                        painter->fillRect(rect, brush);

                        last_prob += strategy_without_fold[ind];
                        ind += 1;
                    }
                }
            }
        }
    }
    QTextDocument doc;
    doc.setHtml(options.text);

    options.text = "";

    painter->translate(options.rect.left(), options.rect.top());
    QRect clip(0, 0, options.rect.width(), options.rect.height());
    doc.drawContents(painter, clip);
}

void StrategyItemDelegate::paint_range(QPainter* painter, const QStyleOptionViewItem& option,
                                       const QModelIndex& index) const {
    auto options = option;
    initStyleOption(&options, index);
    const TableStrategyModel* tableStrategyModel = qobject_cast<const TableStrategyModel*>(index.model());

    vector<pair<int, int>> card_cords;
    if (tableStrategyModel == NULL || tableStrategyModel->ui_p1_range.size() <= index.row() ||
        tableStrategyModel->ui_p1_range.size() <= index.column() ||
        tableStrategyModel->ui_p2_range.size() <= index.row() ||
        tableStrategyModel->ui_p2_range.size() <= index.column()) {
        return;
    }
    if (this->detailWindowSetting->mode == DetailWindowSetting::DetailWindowMode::RANGE_IP) {
        card_cords = tableStrategyModel->ui_p1_range[index.row()][index.column()];
    } else {
        card_cords = tableStrategyModel->ui_p2_range[index.row()][index.column()];
    }

    if (tableStrategyModel->p1_range.empty() || tableStrategyModel->p2_range.empty()) {
        return;
    }

    if (!card_cords.empty()) {
        float range_number = 0;
        for (auto one_cord : card_cords) {
            if (this->detailWindowSetting->mode == DetailWindowSetting::DetailWindowMode::RANGE_IP) {
                range_number += tableStrategyModel->p1_range[one_cord.first][one_cord.second];
            } else {
                range_number += tableStrategyModel->p2_range[one_cord.first][one_cord.second];
            }
        }
        range_number = range_number / card_cords.size();

        if (range_number < 0 || range_number > 1)
            throw runtime_error("range number incorrect in strategyitemdeletage");

        float fold_prob = 1 - range_number;
        int disable_height = (int)(fold_prob * option.rect.height());
        int remain_height = option.rect.height() - disable_height;

        QRect rect(option.rect.left(), option.rect.top() + disable_height, option.rect.width(), remain_height);
        QBrush brush(Qt::yellow);
        painter->fillRect(rect, brush);
    }
    QTextDocument doc;
    doc.setHtml(options.text);

    options.text = "";

    painter->translate(options.rect.left(), options.rect.top());
    QRect clip(0, 0, options.rect.width(), options.rect.height());
    doc.drawContents(painter, clip);
}

void StrategyItemDelegate::paint_evs(QPainter* painter, const QStyleOptionViewItem& option,
                                     const QModelIndex& index) const {
    auto options = option;
    initStyleOption(&options, index);
    const TableStrategyModel* tableStrategyModel = qobject_cast<const TableStrategyModel*>(index.model());
    vector<float> evs = tableStrategyModel->get_ev_grid(index.row(), index.column());
    sort(evs.begin(), evs.end());

    float stack = backend_ != nullptr ? backend_->getStack() : 1.0f;
    int last_left = 0;
    for (std::size_t i = 0; i < evs.size(); i++) {
        float one_ev = evs[evs.size() - i - 1];
        float normalized_ev = normalization_tanh(stack, one_ev);

        int red = max((int)(255 - normalized_ev * 255), 0);
        int green = min((int)(normalized_ev * 255), 255);
        int blue = min(red, green);

        QBrush brush = QBrush(QColor(red, green, blue));

        int this_width = (int)((float(i + 1) / evs.size()) * options.rect.width()) - last_left;

        QRect rect(option.rect.left() + last_left, option.rect.top(), this_width, (int)(options.rect.height()));
        painter->fillRect(rect, brush);
        last_left += this_width;
    }
    QTextDocument doc;
    doc.setHtml(options.text);
    options.text = "";

    painter->translate(options.rect.left(), options.rect.top());
    QRect clip(0, 0, options.rect.width(), options.rect.height());
    doc.drawContents(painter, clip);
}

void StrategyItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                                 const QModelIndex& index) const {
    painter->save();

    QRect rect(option.rect.left(), option.rect.top(), option.rect.width(), option.rect.height());
    QBrush brush(Qt::gray);
    if (index.column() == index.row())
        brush = QBrush(Qt::darkGray);
    painter->fillRect(rect, brush);

    if (this->detailWindowSetting->mode == DetailWindowSetting::DetailWindowMode::STRATEGY) {
        this->paint_strategy(painter, option, index);
    } else if (this->detailWindowSetting->mode == DetailWindowSetting::DetailWindowMode::RANGE_IP ||
               this->detailWindowSetting->mode == DetailWindowSetting::DetailWindowMode::RANGE_OOP) {
        this->paint_range(painter, option, index);
    } else if (this->detailWindowSetting->mode == DetailWindowSetting::DetailWindowMode::EV) {
        this->paint_strategy(painter, option, index, true);
    } else if (this->detailWindowSetting->mode == DetailWindowSetting::DetailWindowMode::EV_ONLY) {
        this->paint_evs(painter, option, index);
    }

    painter->restore();
}
