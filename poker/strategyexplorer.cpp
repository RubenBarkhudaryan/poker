#include "strategyexplorer.h"
#include "ui_strategyexplorer.h"
#include "qstandarditemmodel.h"
#include <QBrush>
#include <QApplication>
#include <QComboBox>
#include <QColor>
#include "include/Card.h"

StrategyExplorer::StrategyExplorer(QWidget* parent, ISolverBackend* backend)
    : QDialog(parent), ui(new Ui::StrategyExplorer) {
    this->backend_ = backend;
    this->detailWindowSetting = DetailWindowSetting();
    ui->setupUi(this);
    /*
    QStandardItemModel* model = new QStandardItemModel();
    for (int row = 0; row < 4; ++row) {
         QStandardItem *item = new QStandardItem(QString("%1").arg(row) );
         model->appendRow( item );
    }
    this->ui->gameTreeView->setModel(model);
    */
    // Initial Game Tree preview panel
    this->ui->gameTreeView->setTreeData(backend_);
    connect(this->ui->gameTreeView, SIGNAL(expanded(const QModelIndex&)), this,
            SLOT(item_expanded(const QModelIndex&)));
    connect(this->ui->gameTreeView, SIGNAL(clicked(const QModelIndex&)), this, SLOT(item_clicked(const QModelIndex&)));

    // Initize strategy(rough) table
    this->tableStrategyModel = new TableStrategyModel(this->backend_, this);
    this->ui->strategyTableView->setModel(this->tableStrategyModel);
    this->delegate_strategy = new StrategyItemDelegate(this->backend_, &(this->detailWindowSetting), this);
    this->ui->strategyTableView->setItemDelegate(this->delegate_strategy);

    int index = 0;
    QString board_qstring = QString::fromStdString(this->backend_->getBoard());
    for (const std::string& card_str : this->backend_->getDeckCards()) {
        Card one_card(card_str);
        if (board_qstring.contains(QString::fromStdString(one_card.toString())))
            continue;
        QString card_str_formatted = QString::fromStdString(one_card.toFormattedString());
        this->ui->turnCardBox->addItem(card_str_formatted);
        this->ui->riverCardBox->addItem(card_str_formatted);

        if (card_str_formatted.contains(QString::fromLocal8Bit("♦️")) ||
            card_str_formatted.contains(QString::fromLocal8Bit("♥️️"))) {
            this->ui->turnCardBox->setItemData(0, QBrush(Qt::red), Qt::ForegroundRole);
            this->ui->riverCardBox->setItemData(0, QBrush(Qt::red), Qt::ForegroundRole);
        } else {
            this->ui->turnCardBox->setItemData(0, QBrush(Qt::black), Qt::ForegroundRole);
            this->ui->riverCardBox->setItemData(0, QBrush(Qt::black), Qt::ForegroundRole);
        }

        this->cards.push_back(one_card);
        index += 1;
    }
    TreeNodeDto root = this->backend_->getTreeRoot();
    if (root.round == GameRoundDto::Flop) {
        this->tableStrategyModel->setTrunCard(this->cards[0]);
        this->tableStrategyModel->setRiverCard(this->cards[1]);
        this->ui->riverCardBox->setCurrentIndex(1);
    } else if (root.round == GameRoundDto::Turn) {
        this->tableStrategyModel->setRiverCard(this->cards[0]);
        this->ui->turnCardBox->clear();
    } else if (root.round == GameRoundDto::River) {
        this->ui->turnCardBox->clear();
        this->ui->riverCardBox->clear();
    }

    // Initize timer for strategy auto update
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(update_second()));
    timer->start(1000);

    // On mouse event of strategy table
    connect(this->ui->strategyTableView, SIGNAL(itemMouseChange(int, int)), this, SLOT(onMouseMoveEvent(int, int)));

    // Initize Detail Viewer window
    this->detailViewerModel = new DetailViewerModel(this->tableStrategyModel, this);
    this->ui->detailView->setModel(this->detailViewerModel);
    this->detailItemItemDelegate = new DetailItemDelegate(&(this->detailWindowSetting), this);
    this->ui->detailView->setItemDelegate(this->detailItemItemDelegate);

    // Initize Rough Strategy Viewer
    this->roughStrategyViewerModel = new RoughStrategyViewerModel(this->tableStrategyModel, this);
    this->ui->roughStrategyView->setModel(this->roughStrategyViewerModel);
    this->roughStrategyItemDelegate = new RoughStrategyItemDelegate(&(this->detailWindowSetting), this);
    this->ui->roughStrategyView->setItemDelegate(this->roughStrategyItemDelegate);
}

StrategyExplorer::~StrategyExplorer() {
    delete ui;
    delete this->delegate_strategy;
    delete this->tableStrategyModel;
    delete this->detailViewerModel;
    delete this->roughStrategyViewerModel;
    delete this->timer;
}

void StrategyExplorer::item_expanded(const QModelIndex& index) {
    TreeItem* item = static_cast<TreeItem*>(index.internalPointer());
    int num_child = item->childCount();
    try {
        for (int i = 0; i < num_child; i++) {
            TreeItem* one_child = item->child(i);
            if (one_child->childCount() != 0)
                continue;
            this->ui->gameTreeView->tree_model->reGenerateTreeItem(one_child->node().round, one_child);
        }
    } catch (const std::exception& error) {
        logBackendError(error);
    }
}

void StrategyExplorer::logBackendError(const std::exception& error) {
    qDebug().noquote() << tr("Encountering error:");
    qDebug().noquote() << error.what() << "\n";
}

void StrategyExplorer::refreshStrategy() {
    try {
        this->tableStrategyModel->updateStrategyData();
    } catch (const std::exception& error) {
        logBackendError(error);
    }
}
void StrategyExplorer::process_board(TreeItem* treeitem) {
    std::string board;
    try {
        board = this->backend_->getBoard();
    } catch (const std::exception& error) {
        logBackendError(error);
        return;
    }
    vector<string> board_str_arr = string_split(board, ',');
    vector<Card> cards;
    for (string one_board_str : board_str_arr) {
        cards.push_back(Card(one_board_str));
    }
    if (treeitem != NULL) {
        if (treeitem->node().round == GameRoundDto::Turn &&
            !this->tableStrategyModel->getTrunCard().empty()) {
            cards.push_back(Card(this->tableStrategyModel->getTrunCard()));
        } else if (treeitem->node().round == GameRoundDto::River) {
            if (!this->tableStrategyModel->getTrunCard().empty())
                cards.push_back(Card(this->tableStrategyModel->getTrunCard()));
            if (!this->tableStrategyModel->getRiverCard().empty())
                cards.push_back(Card(this->tableStrategyModel->getRiverCard()));
        }
    }
    this->ui->boardLabel->setText(QString("<b>%1: </b>").arg(tr("board")) + Card::boardCards2html(cards));
}
void StrategyExplorer::process_treeclick(TreeItem* treeitem) {
    const TreeNodeDto& treenode = treeitem->node();
    if (treenode.node_type == NodeType::Action) {
        QString action_str =
            QString("<b>%1 %2</b>").arg(treenode.player == 0 ? tr("IP") : tr("OOP"), tr(" decision node"));
        this->ui->nodeDisplayLabel->setText(action_str);
    } else if (treenode.node_type == NodeType::Chance) {
        this->ui->nodeDisplayLabel->setText(tr("<b>Chance node</b>"));
    } else if (treenode.node_type == NodeType::Terminal) {
        this->ui->nodeDisplayLabel->setText(tr("<b>Terminal node</b>"));
    } else if (treenode.node_type == NodeType::Showdown) {
        this->ui->nodeDisplayLabel->setText(tr("<b>Showdown node</b>"));
    }
}

void StrategyExplorer::item_clicked(const QModelIndex& index) {
    try {
        TreeItem* treeNode = static_cast<TreeItem*>(index.internalPointer());
        this->process_treeclick(treeNode);
        this->process_board(treeNode);
        this->tableStrategyModel->setGameTreeNode(treeNode);
        this->tableStrategyModel->updateStrategyData();
        this->ui->strategyTableView->viewport()->update();
        this->roughStrategyViewerModel->onchanged();
        this->ui->roughStrategyView->triger_resize();
        this->ui->roughStrategyView->viewport()->update();
    } catch (const std::exception& error) {
        logBackendError(error);
    }
}

void StrategyExplorer::selection_changed(const QItemSelection& selected, const QItemSelection& deselected) {}

void StrategyExplorer::on_turnCardBox_currentIndexChanged(int index) {
    if (this->cards.size() > 0 && index < this->cards.size()) {
        this->tableStrategyModel->setTrunCard(this->cards[index]);
        refreshStrategy();
        // TODO this somehow cause bugs, crashes, why?
        //this->roughStrategyViewerModel->onchanged();
        //this->ui->roughStrategyView->viewport()->update();
        this->process_board(this->tableStrategyModel->treeItem);
    }
    this->ui->strategyTableView->viewport()->update();
    this->ui->detailView->viewport()->update();
}

void StrategyExplorer::on_riverCardBox_currentIndexChanged(int index) {
    if (this->cards.size() > 0 && index < this->cards.size()) {
        this->tableStrategyModel->setRiverCard(this->cards[index]);
        refreshStrategy();
        //this->roughStrategyViewerModel->onchanged();
        //this->ui->roughStrategyView->viewport()->update();
        this->process_board(this->tableStrategyModel->treeItem);
    }
    this->ui->strategyTableView->viewport()->update();
    this->ui->detailView->viewport()->update();
}

void StrategyExplorer::update_second() {
    if (this->cards.size() > 0) {
        refreshStrategy();
        this->roughStrategyViewerModel->onchanged();
        this->ui->roughStrategyView->viewport()->update();
        this->process_board(this->tableStrategyModel->treeItem);
    }
    this->ui->strategyTableView->viewport()->update();
    this->ui->detailView->viewport()->update();
}

void StrategyExplorer::onMouseMoveEvent(int i, int j) {
    this->detailWindowSetting.grid_i = i;
    this->detailWindowSetting.grid_j = j;
    this->ui->detailView->viewport()->update();
    this->ui->strategyTableView->viewport()->update();
}

void StrategyExplorer::on_strategyModeButtom_clicked() {
    this->detailWindowSetting.mode = DetailWindowSetting::DetailWindowMode::STRATEGY;
    this->ui->strategyTableView->viewport()->update();
    this->ui->detailView->viewport()->update();
    this->roughStrategyViewerModel->onchanged();
    this->ui->roughStrategyView->viewport()->update();
}

void StrategyExplorer::on_ipRangeButtom_clicked() {
    this->detailWindowSetting.mode = DetailWindowSetting::DetailWindowMode::RANGE_IP;
    this->ui->strategyTableView->viewport()->update();
    this->ui->detailView->viewport()->update();
    this->roughStrategyViewerModel->onchanged();
    this->ui->roughStrategyView->viewport()->update();
}

void StrategyExplorer::on_oopRangeButtom_clicked() {
    this->detailWindowSetting.mode = DetailWindowSetting::DetailWindowMode::RANGE_OOP;
    this->ui->strategyTableView->viewport()->update();
    this->ui->detailView->viewport()->update();
    this->roughStrategyViewerModel->onchanged();
    this->ui->roughStrategyView->viewport()->update();
}

void StrategyExplorer::on_evModeButtom_clicked() {
    this->detailWindowSetting.mode = DetailWindowSetting::DetailWindowMode::EV;
    this->ui->strategyTableView->viewport()->update();
    this->ui->detailView->viewport()->update();
    this->ui->roughStrategyView->viewport()->update();
}

void StrategyExplorer::on_evOnlyModeButtom_clicked() {
    this->detailWindowSetting.mode = DetailWindowSetting::DetailWindowMode::EV_ONLY;
    this->ui->strategyTableView->viewport()->update();
    this->ui->detailView->viewport()->update();
    this->roughStrategyViewerModel->onchanged();
    this->ui->roughStrategyView->viewport()->update();
}
