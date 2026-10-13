#ifndef QSTREEVIEW_H
#define QSTREEVIEW_H

#include <QTreeView>
#include "include/api/isolver_backend.h"
#include "include/ui/treemodel.h"

class QSTreeView : public QTreeView {
public:
    explicit QSTreeView(QWidget* parent = nullptr);
    ~QSTreeView() override;
    void setTreeData(ISolverBackend* backend);
    TreeModel* tree_model = nullptr;

private:
    ISolverBackend* backend_ = nullptr;
};

#endif // QSTREEVIEW_H
