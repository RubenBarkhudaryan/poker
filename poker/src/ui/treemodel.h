#ifndef TREEMODEL_H
#define TREEMODEL_H

#include "include/ui/treeitem.h"
#include "include/api/isolver_backend.h"
#include "include/api/solver_dtos.h"
#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>

class TreeModel : public QAbstractItemModel {
    Q_OBJECT

public:
    explicit TreeModel(ISolverBackend* backend, QObject* parent = nullptr);
    ~TreeModel() override;

    QVariant data(const QModelIndex& index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    void reGenerateTreeItem(GameRoundDto round, TreeItem* node_to_process);

private:
    ISolverBackend* backend_;
    TreeItem* rootItem_;
    void setupModelData();

public slots:
    void clicked_event(const QModelIndex& index);
};

#endif // TREEMODEL_H
