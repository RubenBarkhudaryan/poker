#include "qstreeview.h"
#include <QStyleFactory>

QSTreeView::QSTreeView(QWidget* parent) : QTreeView(parent) {
    this->setStyle(QStyleFactory::create("windows"));
}

QSTreeView::~QSTreeView() {
    if (this->tree_model != nullptr) {
        delete this->tree_model;
        this->tree_model = nullptr;
    }
}

void QSTreeView::setTreeData(ISolverBackend* backend) {
    this->backend_ = backend;
    if (this->tree_model != nullptr) {
        delete this->tree_model;
        this->tree_model = nullptr;
    }
    this->tree_model = new TreeModel(backend);
    this->setModel(this->tree_model);
}
