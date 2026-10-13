#include "include/ui/treeitem.h"
#include <QObject>

TreeItem::TreeItem(const TreeNodeDto& node, TreeItem* parentItem)
    : m_node(node), m_parentItem(parentItem) {}

TreeItem::~TreeItem() {
    qDeleteAll(m_childItems);
}

bool TreeItem::insertChild(TreeItem* child, int i) {
    if (i >= m_childItems.count() || i < 0)
        m_childItems.append(child);
    else
        m_childItems.insert(i, child);
    return true;
}

TreeItem* TreeItem::child(int row) {
    return m_childItems.at(row);
}

int TreeItem::childCount() const {
    return m_childItems.count();
}

int TreeItem::columnCount() const {
    return 1;
}

QVariant TreeItem::data() const {
    return QString::fromStdString(m_node.label);
}

bool TreeItem::setParentItem(TreeItem* item) {
    m_parentItem = item;
    return true;
}

int TreeItem::row() const {
    if (m_parentItem)
        return m_parentItem->m_childItems.indexOf(const_cast<TreeItem*>(this));
    return 0;
}

TreeItem* TreeItem::parentItem() {
    return m_parentItem;
}

bool TreeItem::removeChild(int row) {
    m_childItems.removeAt(row);
    return true;
}

const TreeNodeDto& TreeItem::node() const {
    return m_node;
}

const std::string& TreeItem::nodeId() const {
    return m_node.id;
}
