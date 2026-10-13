#ifndef ROUGHSTRATEGYVIEWERMODEL_H
#define ROUGHSTRATEGYVIEWERMODEL_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>
#include "include/ui/treeitem.h"
#include "include/ui/tablestrategymodel.h"
#include "include/ui/detailwindowsetting.h"
#include <map>

class RoughStrategyViewerModel : public QAbstractItemModel {
    Q_OBJECT

public:
    explicit RoughStrategyViewerModel(TableStrategyModel* tableStrategyModel, QObject* parent = nullptr);
    ~RoughStrategyViewerModel();

    QVariant data(const QModelIndex& index, int role) const override;
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole);
    QModelIndex parent(const QModelIndex& child) const;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    void onchanged();
    TableStrategyModel* tableStrategyModel = NULL;
};

#endif // ROUGHSTRATEGYVIEWERMODEL_H
