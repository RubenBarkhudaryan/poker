#include "include/ui/roughstrategyitemdelegate.h"
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

RoughStrategyItemDelegate::RoughStrategyItemDelegate(DetailWindowSetting* detailWindowSetting, QObject* parent)
    : WordItemDelegate(parent) {
    this->detailWindowSetting = detailWindowSetting;
}

void RoughStrategyItemDelegate::paint_strategy(QPainter* painter, const QStyleOptionViewItem& option,
                                               const QModelIndex& index) const {
    auto options = option;
    initStyleOption(&options, index);

    const RoughStrategyViewerModel* roughStrategyViewerModel =
        qobject_cast<const RoughStrategyViewerModel*>(index.model());

    options.text = "";
    if (roughStrategyViewerModel->tableStrategyModel->treeItem != NULL &&
        roughStrategyViewerModel->tableStrategyModel->treeItem->node().node_type == NodeType::Action) {
        if (index.column() >= static_cast<int>(roughStrategyViewerModel->tableStrategyModel->total_strategy.size()))
            return;

        pair<ActionLabelDto, pair<float, float>> one_strategy =
            roughStrategyViewerModel->tableStrategyModel->total_strategy[index.column()];

        QBrush brush(Qt::gray);
        int bet_raise_num = 0;
        for (int i = 0; i <= index.column(); i++) {
            ActionLabelDto one_action = roughStrategyViewerModel->tableStrategyModel->total_strategy[i].first;
            if (isFoldLabel(one_action)) {
                brush = QBrush(QColor(0, 191, 255));
            } else if (isCheckOrCallLabel(one_action)) {
                brush = QBrush(Qt::green);
            } else if (isBetOrRaiseLabel(one_action)) {
                int color_base = max(128 - 32 * bet_raise_num - 1, 0);
                brush = QBrush(QColor(255, color_base, color_base));
                bet_raise_num += 1;
            } else {
                brush = QBrush(Qt::blue);
            }
        }
        QRect rect(option.rect.left(), option.rect.top(), option.rect.width(), option.rect.height());
        painter->fillRect(rect, brush);

        options.text = "";
        ActionLabelDto one_action = one_strategy.first;
        float one_strategy_float = one_strategy.second.second * 100;
        float one_combo = one_strategy.second.first;
        if (isFoldLabel(one_action)) {
            options.text += QString(" <h3> %1 <\/h3> <h4>%2\%<\/h4> <h4>%3 %4<\/h4>")
                                .arg(tr("FOLD"), QString::number(one_strategy_float, 'f', 1),
                                     QString::number(one_combo, 'f', 1), tr("combos"));
        } else if (one_action.label == "CALL") {
            options.text += QString(" <h3> %1 <\/h3> <h4>%2\%<\/h4> <h4>%3 %4<\/h4>")
                                .arg(tr("CALL"), QString::number(one_strategy_float, 'f', 1),
                                     QString::number(one_combo, 'f', 1), tr("combos"));
        } else if (one_action.label == "CHECK") {
            options.text += QString(" <h3> %1 <\/h3> <h4>%2\%<\/h4> <h4>%3 %4<\/h4>")
                                .arg(tr("CHECK"), QString::number(one_strategy_float, 'f', 1),
                                     QString::number(one_combo, 'f', 1), tr("combos"));
        } else if (isBetLabel(one_action)) {
            options.text +=
                QString(" <h3> %1 %2 <\/h3> <h4>%3\%<\/h4> <h4>%4 %5<\/h4>")
                    .arg(tr("BET"), QString::number(one_action.amount, 'f', 1),
                         QString::number(one_strategy_float, 'f', 1), QString::number(one_combo, 'f', 1), tr("combos"));
        } else if (isRaiseLabel(one_action)) {
            options.text +=
                QString(" <h3> %1 %2 <\/h3> <h4>%3\%<\/h4> <h4>%4 %5<\/h4>")
                    .arg(tr("RAISE"), QString::number(one_action.amount, 'f', 1),
                         QString::number(one_strategy_float, 'f', 1), QString::number(one_combo, 'f', 1), tr("combos"));
        }
    }
    QTextDocument doc;
    doc.setHtml(options.text);

    options.text = "";

    painter->translate(options.rect.left(), options.rect.top());
    QRect clip(0, 0, options.rect.width(), options.rect.height());
    doc.drawContents(painter, clip);
}

void RoughStrategyItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                                      const QModelIndex& index) const {
    painter->save();

    QRect rect(option.rect.left(), option.rect.top(), option.rect.width(), option.rect.height());
    QBrush brush(Qt::gray);
    painter->fillRect(rect, brush);

    this->paint_strategy(painter, option, index);

    painter->restore();
}
