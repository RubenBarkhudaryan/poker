#include "include/ui/treemodel.h"
#include <stdexcept>

TreeModel::TreeModel(ISolverBackend* backend, QObject* parent)
    : QAbstractItemModel(parent), backend_(backend), rootItem_(nullptr) {
    setupModelData();
}

TreeModel::~TreeModel() {
    delete rootItem_;
}

int TreeModel::columnCount(const QModelIndex& parent) const {
    if (parent.isValid())
        return static_cast<TreeItem*>(parent.internalPointer())->columnCount();
    if (rootItem_ == nullptr)
        return 1;
    return rootItem_->columnCount();
}

QVariant TreeModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid())
        return QVariant();
    if (role != Qt::DisplayRole)
        return QVariant();
    TreeItem* item = static_cast<TreeItem*>(index.internalPointer());
    return item->data();
}

Qt::ItemFlags TreeModel::flags(const QModelIndex& index) const {
    if (!index.isValid())
        return Qt::NoItemFlags;
    return QAbstractItemModel::flags(index);
}

QVariant TreeModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole && rootItem_ != nullptr)
        return rootItem_->data();
    return QVariant();
}

QModelIndex TreeModel::index(int row, int column, const QModelIndex& parent) const {
    if (rootItem_ == nullptr || !hasIndex(row, column, parent))
        return QModelIndex();

    TreeItem* parentItem;
    if (!parent.isValid())
        parentItem = rootItem_;
    else
        parentItem = static_cast<TreeItem*>(parent.internalPointer());

    TreeItem* childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    return QModelIndex();
}

QModelIndex TreeModel::parent(const QModelIndex& index) const {
    if (!index.isValid() || rootItem_ == nullptr)
        return QModelIndex();

    TreeItem* childItem = static_cast<TreeItem*>(index.internalPointer());
    TreeItem* parentItem = childItem->parentItem();

    if (parentItem == rootItem_)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int TreeModel::rowCount(const QModelIndex& parent) const {
    if (rootItem_ == nullptr)
        return 0;
    if (parent.column() > 0)
        return 0;

    TreeItem* parentItem;
    if (!parent.isValid())
        parentItem = rootItem_;
    else
        parentItem = static_cast<TreeItem*>(parent.internalPointer());

    return parentItem->childCount();
}

void TreeModel::reGenerateTreeItem(GameRoundDto round, TreeItem* node_to_process) {
    if (backend_ == nullptr || node_to_process == nullptr)
        return;

    const std::vector<TreeNodeDto> children = backend_->getTreeChildren(node_to_process->nodeId());
    for (const TreeNodeDto& child_dto : children) {
        TreeItem* child_node = new TreeItem(child_dto, node_to_process);
        node_to_process->insertChild(child_node);
        if (child_dto.round != round)
            continue;
        reGenerateTreeItem(round, child_node);
    }
}

void TreeModel::setupModelData() {
    if (backend_ == nullptr || !backend_->hasGameTree())
        return;

    const TreeNodeDto root = backend_->getTreeRoot();
    rootItem_ = new TreeItem(root);
    TreeItem* visible_root = new TreeItem(root, rootItem_);
    rootItem_->insertChild(visible_root);
    reGenerateTreeItem(root.round, visible_root);
}

void TreeModel::clicked_event(const QModelIndex& index) {
    Q_UNUSED(index);
}
