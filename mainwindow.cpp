#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QStandardItemModel>
#include <QToolTip>
#include <QClipboard>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QWidget *tab;
    QAction *act;
    auto view = ui->menuView_mode;
    for(int i =ui->tabWidget->count(); i--;){
        tab =ui->tabWidget->widget(i);
        tabs[ui->tabWidget->tabText(i)] = tab;
        act = new QAction(ui->tabWidget->tabText(i));
        act->setCheckable(true);
        act->setChecked(true);
        connect(act, &QAction::toggled, this, &MainWindow::viewModeChecked);
        view->addAction(act);
        for(auto button: tab->findChildren<QPushButton *>()){
            if(button->objectName().startsWith("buttonPin")){
                pins[tab] = button;
                connect(button, &QPushButton::clicked, this, &MainWindow::buttonPinToggled);
            }
        }
        auto spinBoxes = tab->findChildren<QSpinBox *>();
        auto tables = tab->findChildren<QTableView *>();
        for(auto spinBox: spinBoxes){
            if(spinBox->objectName().startsWith("nodesCount")){
                for(int j = tables.count(); j--;){
                    if(tables[j]->objectName().startsWith("tableGraphMatrix")){
                        connect(spinBox,
                                &QSpinBox::valueChanged,
                                this,
                                std::bind(&MainWindow::setNodesAmountSet,
                                          this, tables[j], std::placeholders::_1));
                        tables[j]->setModel(new QStandardItemModel(0,0));
                        tables.remove(j);
                    }
                }
            }
        }
    }

    nodeMovementGroup = new QActionGroup(this);
    nodeMovementGroup->addAction(ui->actionAutomatic);
    nodeMovementGroup->addAction(ui->actionManual);
    ui->actionManual->setChecked(true);
    for(auto& i: ui->menubar->children()){
        ((QMenu*)i)->setWindowFlag(Qt::FramelessWindowHint);
        ((QMenu*)i)->setWindowFlag(Qt::NoDropShadowWindowHint);
        ((QMenu*)i)->setAttribute(Qt::WA_TranslucentBackground);
    }
    connect(ui->actionCopy,
            &QAction::triggered,
            this,
            std::bind(&MainWindow::copyTableToClipboard,
                      this, ui->tableGraphMatrixInc));
    connect(ui->actionPaste,
            &QAction::triggered,
            this,
            std::bind(&MainWindow::pasteClipboardToTable,
                      this, ui->tableGraphMatrixInc));
}

MainWindow::~MainWindow()
{
    delete nodeMovementGroup;
    delete ui;
}

void MainWindow::buttonClearConsoleClicked()
{
    ui->console->clear();
}

void MainWindow::unpinTab(int index = -1){
    if(index==-1){
        index = ui->tabWidget->currentIndex();
    }

    QWidget *unpinnedTab = ui->tabWidget->widget(index);
    if(unpinnedTab->objectName()=="graphView")
        return;
    unpinnedTab->setWindowTitle(ui->tabWidget->tabText(index));
    unpinnedTab->hide();
    pins[unpinnedTab]->setChecked(true);
    unpinnedTab->setParent(nullptr);
    unpinnedTab->show();
}
void MainWindow::pinTab(){
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    auto key = button->nativeParentWidget();


    ui->tabWidget->addTab(key, key->windowTitle());
    for(auto act: ui->menuView_mode->actions()){
        if(key->windowTitle()==act->text()){
            ui->tabWidget->setTabVisible(ui->tabWidget->count()-1, act->isChecked());
            break;
        }
    }
}

void MainWindow::pasteClipboardToTable(QTableView *dest)
{
    static QRegularExpression re("\t|(?=\n)");
    QStandardItemModel * model = static_cast<QStandardItemModel*>(dest->model());
    QItemSelectionModel * selection = dest->selectionModel();
    QModelIndexList indexes = selection->selectedIndexes();
    QString text = QApplication::clipboard()->text();
    // You need a pair of indexes to find the row changes
    if(indexes.empty()) return;
    int maxIndexCol=0, maxIndexRow=0;
    if (indexes.count() == 1){
        maxIndexCol = model->columnCount();
        maxIndexRow = model->rowCount();
    } else {
        for(auto & index : selection->selectedIndexes()){
            if((index.column())>maxIndexCol)
                maxIndexCol = index.column();
            if((index.row())>maxIndexRow)
                maxIndexRow = index.row();
        }
        maxIndexCol++;
        maxIndexRow++;
    }
    int row=indexes.first().row();
    int col=indexes.first().column();
    int i = 0, j = 0, dt = 0;
    auto data = text.split(re);
    int data_len = data.count();
    QString textToCell;
    while(((row+i < maxIndexRow)) && (dt < data_len))
    {
        if(data[dt].startsWith('\n')){
            i++;
            if ((dt < data_len - 1) &&
                (row+i < maxIndexRow) &&
                (model->item(row+i, col)->flags()!=Qt::NoItemFlags)){
                j = 0;
                textToCell = data[dt].replace("\n","");
                model->item(row+i, col)->setData(textToCell);
                model->item(row+i, col)->setText(textToCell);
                j++;
            }
            dt++;
        }else {
            if ((col+j < maxIndexCol) &&
                (model->item(row+i, col+j)->flags()!=Qt::NoItemFlags)){
                textToCell = data[dt];
                model->item(row+i, col+j)->setData(textToCell);
                model->item(row+i, col+j)->setText(textToCell);
                j++;
            }
            dt++;
        }
    }
    emit model->dataChanged(indexes.first(),model->item(row+i-1, col+j-1)->index());
}


void MainWindow::on_actionUnpin_current_tab_triggered()
{
    unpinTab();
}

void MainWindow::buttonPinToggled(bool checked)
{
    if(!checked){
        pinTab();
    }else{
        unpinTab();
    }
}

void MainWindow::viewModeChecked(bool checked)
{
    auto act = qobject_cast<QAction*>(sender());
    auto *tab = tabs[act->text()];
    ui->tabWidget->setTabVisible(ui->tabWidget->indexOf(tab), checked);
}

void MainWindow::setNodesAmountSet(QTableView *table, int newAmount)
{
    QStandardItemModel* model = static_cast<QStandardItemModel *>(table->model());
    int oldAmount = model->columnCount();
    int delta = abs(oldAmount-newAmount);
    auto flags = Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable;
    if(newAmount < 0) newAmount = 0;
    for(int i = ((oldAmount>newAmount)?(newAmount):(oldAmount)); i--;){
        if ((model->item(i, i)->flags() == Qt::NoItemFlags)){
            for(int j = i+1; j--;){
                model->item(i, j)->setFlags(flags);
                model->item(j, i)->setFlags(flags);
            }
        }
    }
    if(oldAmount > newAmount){
        for(int i = oldAmount; i--!=newAmount;){
            if ((model->item(i, i)->flags() != Qt::NoItemFlags)){
                for(int j = i+1; j--;){
                    model->item(i, j)->setFlags(Qt::NoItemFlags);
                    model->item(j, i)->setFlags(Qt::NoItemFlags);
                }
            }

        }
    } else if (oldAmount == newAmount){
        if (model->item(oldAmount-1, oldAmount-1)->flags() == Qt::NoItemFlags){
            int j, i = oldAmount-1;
            for (j=i+1; j--;){
                model->item(i, j)->setFlags(flags);
                model->item(j, i)->setFlags(flags);
            }
        }
    }else if (oldAmount < newAmount) {
        model->insertColumns(oldAmount, delta);
        model->insertRows(oldAmount, delta);
        for(int i = oldAmount; i!=newAmount; ++i){
            model->setHeaderData(i, Qt::Horizontal, i+1);
            model->setHeaderData(i, Qt::Vertical, i+1);
            table->setColumnWidth(i, 20);
            table->setRowHeight(i, 20);
            for(int j = newAmount; j--;){
                model->setItem(i, j, new QStandardItem("0"));
                if(i!=j)
                    model->setItem(j, i, new QStandardItem("0"));
            }
        }
    }
    table->show();
    ui->statusbar->showMessage(QString::number(model->columnCount()));
}

void MainWindow::copyTableToClipboard(QTableView *src){
    QAbstractItemModel * model = src->model();
    QItemSelectionModel * selection = src->selectionModel();
    QModelIndexList indexes = selection->selectedIndexes();
    QString selected_text;
    int indexes_count = indexes.count();
    if(indexes_count){
        QModelIndex *current, *previous = &indexes.first();
        QString text = model->data(*previous).toString();
        selected_text.append(text);
        for(int i = 1; i<indexes_count; i++)
        {
            current = &indexes[i];
            text = model->data(*current).toString();
            if (current->row() != previous->row())
            {
                selected_text.append('\n');
            }
            else
            {
                selected_text.append('\t');
            }
            selected_text.append(text);
            previous = current;
        }
        QApplication::clipboard()->setText(selected_text+'\n');
    }
}
