#include "include/ui/detailitemdelegate.h"
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
bool isBetLabel(const ActionLabelDto& action) {
    return action.label.rfind("BET", 0) == 0;
}
bool isRaiseLabel(const ActionLabelDto& action) {
    return action.label.rfind("RAISE", 0) == 0;
}
} // namespace

DetailItemDelegate::DetailItemDelegate(DetailWindowSetting* detailWindowSetting, QObject* parent)
    : WordItemDelegate(parent) {
    this->detailWindowSetting = detailWindowSetting;
}

void DetailItemDelegate::paint_strategy(QPainter* painter, const QStyleOptionViewItem& option,
                                        const QModelIndex& index) const {
    auto options = option;
    initStyleOption(&options, index);

    const DetailViewerModel* detailViewerModel = qobject_cast<const DetailViewerModel*>(index.model());

    options.text = "";
    if (detailViewerModel->tableStrategyModel->treeItem != NULL &&
        detailViewerModel->tableStrategyModel->treeItem->node().node_type == NodeType::Action) {
        int strategy_number = 0;
        if (this->detailWindowSetting->grid_i >= 0 && this->detailWindowSetting->grid_j >= 0) {
            strategy_number =
                detailViewerModel->tableStrategyModel
                    ->ui_strategy_table[this->detailWindowSetting->grid_i][this->detailWindowSetting->grid_j]
                    .size();
        }
        int ind = index.row() * detailViewerModel->columns + index.column();

        if (ind < strategy_number) {
            pair<int, int> strategy_ui_table =
                detailViewerModel->tableStrategyModel
                    ->ui_strategy_table[this->detailWindowSetting->grid_i][this->detailWindowSetting->grid_j][ind];
            int card1 = strategy_ui_table.first;
            int card2 = strategy_ui_table.second;
            vector<float> strategy = detailViewerModel->tableStrategyModel->current_strategy[card1][card2];
            const vector<ActionLabelDto>& gameActions = detailViewerModel->tableStrategyModel->actions;
            if (gameActions.size() != strategy.size())
                throw runtime_error("size mismatch in DetailItemItemDelegate paint");
            float fold_prob = 0;
            vector<float> strategy_without_fold;
            float strategy_without_fold_sum = 0;
            for (std::size_t i = 0; i < strategy.size(); i++) {
                const ActionLabelDto& one_action = gameActions[i];
                if (isFoldLabel(one_action)) {
                    fold_prob = strategy[i];
                } else {
                    strategy_without_fold.push_back(strategy[i]);
                    strategy_without_fold_sum += strategy[i];
                }
            }

            if (strategy_without_fold_sum > 0.) {
                for (std::size_t i = 0; i < strategy_without_fold.size(); i++) {
                    strategy_without_fold[i] = strategy_without_fold[i] / strategy_without_fold_sum;
                }
            }

            float range_number;
            if (0 == detailViewerModel->tableStrategyModel->current_player) {
                range_number = detailViewerModel->tableStrategyModel->p1_range[card1][card2];
            } else {
                range_number = detailViewerModel->tableStrategyModel->p2_range[card1][card2];
            }
            if (range_number < 0 || range_number > 1)
                throw runtime_error("range number incorrect in strategyitemdeletage");

            float not_in_range = 1 - range_number;
            int niR_height = (int)(not_in_range * option.rect.height() * 0.975);

            int disable_height = (int)(0.5 + fold_prob * (option.rect.height() - niR_height));
            int remain_height = option.rect.height() - niR_height - disable_height;

            if (disable_height > 0) {
                QRect rect(option.rect.left(), option.rect.top() + niR_height, option.rect.width(), disable_height);
                QBrush brush(QColor(0, 191, 255));
                painter->fillRect(rect, brush);
            }
            if (remain_height > 0) {
                int action_ind = 0;
                float last_prob = 0;
                int bet_raise_num = 0;
                for (std::size_t i = 0; i < strategy.size(); i++) {
                    const ActionLabelDto& one_action = gameActions[i];
                    QBrush brush(Qt::gray);
                    if (!isFoldLabel(one_action)) {
                        if (isCheckOrCallLabel(one_action)) {
                            brush = QBrush(Qt::green);
                        } else if (isBetOrRaiseLabel(one_action)) {
                            int color_base = max(128 - 32 * bet_raise_num - 1, 0);
                            brush = QBrush(QColor(255, color_base, color_base));
                            bet_raise_num += 1;
                        } else {
                            brush = QBrush(Qt::blue);
                        }

                        int delta_x = (int)(option.rect.width() * last_prob);
                        int delta_width = (int)(option.rect.width() * (last_prob + strategy_without_fold[action_ind])) -
                                          (int)(option.rect.width() * last_prob);

                        QRect rect(option.rect.left() + delta_x, option.rect.top() + niR_height + disable_height,
                                   delta_width, remain_height);
                        painter->fillRect(rect, brush);

                        last_prob += strategy_without_fold[action_ind];
                        action_ind += 1;
                    }
                }
            }
            options.text = "";
            options.text += detailViewerModel->tableStrategyModel->cardint2card[card1].toFormattedHtml();
            options.text += detailViewerModel->tableStrategyModel->cardint2card[card2].toFormattedHtml();
            options.text = "<h2 >" + options.text + "<\/h2>";
            for (std::size_t i = 0; i < strategy.size(); i++) {
                const ActionLabelDto& one_action = gameActions[i];
                float one_strategy = strategy[i] * 100;
                if (isFoldLabel(one_action)) {
                    options.text +=
                        QString(" <h5> %1 : %2\%<\/h5>").arg(tr("FOLD"), QString::number(one_strategy, 'f', 1));
                } else if (one_action.label == "CALL") {
                    options.text +=
                        QString(" <h5> %1 : %2\%<\/h5>").arg(tr("CALL"), QString::number(one_strategy, 'f', 1));
                } else if (one_action.label == "CHECK") {
                    options.text +=
                        QString(" <h5> %1 : %2\%<\/h5>").arg(tr("CHECK"), QString::number(one_strategy, 'f', 1));
                } else if (isBetLabel(one_action)) {
                    options.text += QString(" <h5> %1 %2 : %3\%<\/h5>")
                                        .arg(tr("BET"), QString::number(one_action.amount),
                                             QString::number(one_strategy, 'f', 1));
                } else if (isRaiseLabel(one_action)) {
                    options.text += QString(" <h5> %1 %2 : %3\%<\/h5>")
                                        .arg(tr("RAISE"), QString::number(one_action.amount),
                                             QString::number(one_strategy, 'f', 1));
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

void DetailItemDelegate::paint_range(QPainter* painter, const QStyleOptionViewItem& option,
                                     const QModelIndex& index) const {
    auto options = option;
    initStyleOption(&options, index);

    const DetailViewerModel* detailViewerModel = qobject_cast<const DetailViewerModel*>(index.model());
    options.text = "";

    if (detailViewerModel->tableStrategyModel->treeItem != NULL) {
        vector<pair<int, int>> card_cords;
        if (this->detailWindowSetting->mode == DetailWindowSetting::DetailWindowMode::RANGE_IP) {
            card_cords = detailViewerModel->tableStrategyModel
                             ->ui_p1_range[this->detailWindowSetting->grid_i][this->detailWindowSetting->grid_j];
        } else {
            card_cords = detailViewerModel->tableStrategyModel
                             ->ui_p2_range[this->detailWindowSetting->grid_i][this->detailWindowSetting->grid_j];
        }

        int ind = index.row() * detailViewerModel->columns + index.column();
        if (ind < static_cast<int>(card_cords.size())) {
            pair<int, int> cord = card_cords[ind];
            float range_number;
            if (this->detailWindowSetting->mode == DetailWindowSetting::DetailWindowMode::RANGE_IP) {
                range_number = detailViewerModel->tableStrategyModel->p1_range[cord.first][cord.second];
            } else {
                range_number = detailViewerModel->tableStrategyModel->p2_range[cord.first][cord.second];
            }

            if (range_number < 0 || range_number > 1)
                throw runtime_error("range number incorrect in strategyitemdeletage");

            float fold_prob = 1 - range_number;
            int disable_height = (int)(fold_prob * option.rect.height());
            int remain_height = option.rect.height() - disable_height;

            QRect rect(option.rect.left(), option.rect.top() + disable_height, option.rect.width(), remain_height);
            QBrush brush(Qt::yellow);
            painter->fillRect(rect, brush);

            options.text = "";
            options.text += detailViewerModel->tableStrategyModel->cardint2card[cord.first].toFormattedHtml();
            options.text += detailViewerModel->tableStrategyModel->cardint2card[cord.second].toFormattedHtml();
            options.text = "<h2>" + options.text + "<\/h2>";

            options.text += QString(" <h2>%1<\/h2>").arg(QString::number(range_number, 'f', 3));
        }
    }

    QTextDocument doc;
    doc.setHtml(options.text);

    painter->translate(options.rect.left(), options.rect.top());
    QRect clip(0, 0, options.rect.width(), options.rect.height());
    doc.drawContents(painter, clip);
}

void DetailItemDelegate::paint_evs(QPainter* painter, const QStyleOptionViewItem& option,
                                   const QModelIndex& index) const {
    auto options = option;
    initStyleOption(&options, index);

    const DetailViewerModel* detailViewerModel = qobject_cast<const DetailViewerModel*>(index.model());

    options.text = "";
    if (detailViewerModel->tableStrategyModel->treeItem != NULL &&
        detailViewerModel->tableStrategyModel->treeItem->node().node_type == NodeType::Action) {
        const TreeNodeDto& node = detailViewerModel->tableStrategyModel->treeItem->node();
        int strategy_number = 0;
        if (this->detailWindowSetting->grid_i >= 0 && this->detailWindowSetting->grid_j >= 0) {
            strategy_number =
                detailViewerModel->tableStrategyModel
                    ->ui_strategy_table[this->detailWindowSetting->grid_i][this->detailWindowSetting->grid_j]
                    .size();
        }
        int ind = index.row() * detailViewerModel->columns + index.column();

        if (ind < strategy_number) {
            pair<int, int> strategy_ui_table =
                detailViewerModel->tableStrategyModel
                    ->ui_strategy_table[this->detailWindowSetting->grid_i][this->detailWindowSetting->grid_j][ind];
            int card1 = strategy_ui_table.first;
            int card2 = strategy_ui_table.second;
            const vector<ActionLabelDto>& gameActions = detailViewerModel->tableStrategyModel->actions;
            vector<float> evs = detailViewerModel->tableStrategyModel->current_evs.empty()
                                    ? vector<float>(gameActions.size(), -1.0)
                                    : detailViewerModel->tableStrategyModel->current_evs[card1][card2];
            if (gameActions.size() != evs.size())
                throw runtime_error("evs size mismatch in DetailItemItemDelegate paint");

            vector<float> strategy = detailViewerModel->tableStrategyModel->current_strategy[card1][card2];
            if (gameActions.size() != strategy.size())
                throw runtime_error("strategy size mismatch in DetailItemItemDelegate paint");

            float fold_prob = 0;
            vector<float> strategy_without_fold;
            float strategy_without_fold_sum = 0;
            for (std::size_t i = 0; i < strategy.size(); i++) {
                const ActionLabelDto& one_action = gameActions[i];
                if (isFoldLabel(one_action)) {
                    fold_prob = strategy[i];
                } else {
                    strategy_without_fold.push_back(strategy[i]);
                    strategy_without_fold_sum += strategy[i];
                }
            }

            if (strategy_without_fold_sum > 0.) {
                for (std::size_t i = 0; i < strategy_without_fold.size(); i++) {
                    strategy_without_fold[i] = strategy_without_fold[i] / strategy_without_fold_sum;
                }
            }

            float range_number;
            if (0 == detailViewerModel->tableStrategyModel->current_player) {
                range_number = detailViewerModel->tableStrategyModel->p1_range[card1][card2];
            } else {
                range_number = detailViewerModel->tableStrategyModel->p2_range[card1][card2];
            }

            if (range_number < 0 || range_number > 1)
                throw runtime_error("range number incorrect in strategyitemdeletage");

            float not_in_range = 1 - range_number;
            int niR_height = (int)(not_in_range * option.rect.height() * 0.975);

            int disable_height = (int)(0.5 + fold_prob * (option.rect.height() - niR_height));
            int remain_height = option.rect.height() - niR_height - disable_height;

            if (disable_height > 0) {
                QRect rect(option.rect.left(), option.rect.top() + niR_height, option.rect.width(), disable_height);
                QBrush brush(QColor(0, 191, 255));
                painter->fillRect(rect, brush);
            }
            if (remain_height > 0) {
                int action_ind = 0;
                float last_prob = 0;
                int bet_raise_num = 0;
                for (std::size_t i = 0; i < strategy.size(); i++) {
                    const ActionLabelDto& one_action = gameActions[i];
                    float normalized_ev = normalization_tanh(node.pot * 3, evs[i]);
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
                        int delta_width = (int)(option.rect.width() * (last_prob + strategy_without_fold[action_ind])) -
                                          (int)(option.rect.width() * last_prob);

                        QRect rect(option.rect.left() + delta_x, option.rect.top() + niR_height + disable_height,
                                   delta_width, remain_height);
                        painter->fillRect(rect, brush);

                        last_prob += strategy_without_fold[action_ind];
                        action_ind += 1;
                    }
                }
            }

            options.text = "";
            options.text += detailViewerModel->tableStrategyModel->cardint2card[card1].toFormattedHtml();
            options.text += detailViewerModel->tableStrategyModel->cardint2card[card2].toFormattedHtml();
            options.text = "<h2>" + options.text + "<\/h2>";
            for (std::size_t i = 0; i < evs.size(); i++) {
                const ActionLabelDto& one_action = gameActions[i];
                QString one_ev = evs[i] != evs[i] ? tr("Can't calculate") : QString::number(evs[i], 'f', 1);
                QString ev_str = tr("EV");
                if (isFoldLabel(one_action)) {
                    options.text += QString(" <h5> %1 %2: %3<\/h5>").arg(tr("FOLD"), ev_str, one_ev);
                } else if (one_action.label == "CALL") {
                    options.text += QString(" <h5> %1 %2: %3<\/h5>").arg(tr("CALL"), ev_str, one_ev);
                } else if (one_action.label == "CHECK") {
                    options.text += QString(" <h5> %1 %2: %3<\/h5>").arg(tr("CHECK"), ev_str, one_ev);
                } else if (isBetLabel(one_action)) {
                    options.text += QString(" <h5> %1 %2 %3: %4<\/h5>")
                                        .arg(tr("BET"), QString::number(one_action.amount), ev_str, one_ev);
                } else if (isRaiseLabel(one_action)) {
                    options.text += QString(" <h5> %1 %2 %3: %4<\/h5>")
                                        .arg(tr("RAISE"), QString::number(one_action.amount), ev_str, one_ev);
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

void DetailItemDelegate::paint_evs_only(QPainter* painter, const QStyleOptionViewItem& option,
                                        const QModelIndex& index) const {
    auto options = option;
    initStyleOption(&options, index);

    const DetailViewerModel* detailViewerModel = qobject_cast<const DetailViewerModel*>(index.model());
    options.text = "";

    if (detailViewerModel->tableStrategyModel->treeItem != NULL &&
        detailViewerModel->tableStrategyModel->treeItem->node().node_type == NodeType::Action) {
        int strategy_number = 0;
        if (this->detailWindowSetting->grid_i >= 0 && this->detailWindowSetting->grid_j >= 0) {
            strategy_number =
                detailViewerModel->tableStrategyModel
                    ->ui_strategy_table[this->detailWindowSetting->grid_i][this->detailWindowSetting->grid_j]
                    .size();
        }

        vector<float> evs = detailViewerModel->tableStrategyModel->get_ev_grid(this->detailWindowSetting->grid_i,
                                                                               this->detailWindowSetting->grid_j);
        std::size_t ind = index.row() * detailViewerModel->columns + index.column();

        if (ind < evs.size() && ind < static_cast<std::size_t>(strategy_number)) {
            float one_ev = evs[ind];
            float stack = detailViewerModel->tableStrategyModel->get_backend() != nullptr
                              ? detailViewerModel->tableStrategyModel->get_backend()->getStack()
                              : 1.0f;
            float normalized_ev = normalization_tanh(stack, one_ev);

            pair<int, int> strategy_ui_table =
                detailViewerModel->tableStrategyModel
                    ->ui_strategy_table[this->detailWindowSetting->grid_i][this->detailWindowSetting->grid_j][ind];
            int card1 = strategy_ui_table.first;
            int card2 = strategy_ui_table.second;

            int red = max((int)(255 - normalized_ev * 255), 0);
            int green = min((int)(normalized_ev * 255), 255);
            int blue = min(red, green);
            QBrush brush = QBrush(QColor(red, green, blue));

            QRect rect(option.rect.left(), option.rect.top(), option.rect.width(), option.rect.height());
            painter->fillRect(rect, brush);

            options.text = "";
            options.text += detailViewerModel->tableStrategyModel->cardint2card[card1].toFormattedHtml();
            options.text += detailViewerModel->tableStrategyModel->cardint2card[card2].toFormattedHtml();
            options.text = "<h2>" + options.text + "<\/h2>";

            options.text += QString(" <h2>%1<\/h2>").arg(QString::number(one_ev, 'f', 3));
        }
    }

    QTextDocument doc;
    doc.setHtml(options.text);

    painter->translate(options.rect.left(), options.rect.top());
    QRect clip(0, 0, options.rect.width(), options.rect.height());
    doc.drawContents(painter, clip);
}

void DetailItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const {
    painter->save();

    QRect rect(option.rect.left(), option.rect.top(), option.rect.width(), option.rect.height());
    QBrush brush(Qt::gray);
    painter->fillRect(rect, brush);

    if (this->detailWindowSetting->mode == DetailWindowSetting::DetailWindowMode::STRATEGY) {
        this->paint_strategy(painter, option, index);
    } else if (this->detailWindowSetting->mode == DetailWindowSetting::DetailWindowMode::RANGE_IP ||
               this->detailWindowSetting->mode == DetailWindowSetting::DetailWindowMode::RANGE_OOP) {
        this->paint_range(painter, option, index);
    } else if (this->detailWindowSetting->mode == DetailWindowSetting::DetailWindowMode::EV) {
        this->paint_evs(painter, option, index);
    } else if (this->detailWindowSetting->mode == DetailWindowSetting::DetailWindowMode::EV_ONLY) {
        this->paint_evs_only(painter, option, index);
    }

    painter->restore();
}
