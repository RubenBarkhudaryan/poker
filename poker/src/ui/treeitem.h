#ifndef TREEITEM_H
#define TREEITEM_H

#include <QList>
#include <QVariant>
#include "include/api/solver_dtos.h"

class TreeItem {
public:
    explicit TreeItem(const TreeNodeDto& node, TreeItem* parentItem = nullptr);
    ~TreeItem();

    bool insertChild(TreeItem* child, int i = -1);
    TreeItem* child(int row);
    int childCount() const;
    int columnCount() const;
    QVariant data() const;
    bool setParentItem(TreeItem* item);
    int row() const;
    TreeItem* parentItem();
    bool removeChild(int row);

    const TreeNodeDto& node() const;
    const std::string& nodeId() const;

private:
    TreeNodeDto m_node;
    QList<TreeItem*> m_childItems;
    TreeItem* m_parentItem;
};

#endif // TREEITEM_H
