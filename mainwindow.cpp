#include "mainwindow.h"
#include "qspinbox.h"
#include "ui_mainwindow.h"
#include "graphwidget.h"
#include "graph.h"


#include <QStandardItemModel>
#include <QMessageBox>
#include <QToolTip>
#include <QClipboard>
#include <set>
#include <QShortcut>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);    
    auto spinBoxes = this->findChildren<QSpinBox *>();
    for(auto* table : this->findChildren<QTableView *>()){
        if(table->objectName().endsWith("Graph")){
            auto name = table->objectName().replace("table_", "").replace("_Graph", "");
            for(auto *spinBox : spinBoxes){
                if(spinBox->objectName().startsWith("spin_"+name))
                {
                    if(spinBox->objectName().endsWith("NodesCount")){
                        if(name.startsWith("Matrix"))
                            connect(spinBox,
                                    &QSpinBox::valueChanged,
                                    this,
                                    std::bind(&MainWindow::setNodesAmountSet,
                                              this, table, std::placeholders::_1));
                        break;
                    }
                }
            }
        }
        table->setModel(new QStandardItemModel(0,0));
    }
    auto view = ui->menuView_mode;
    auto docks = this->findChildren<QDockWidget *>();
    if(!docks.empty()){
        auto dockStack = this->findChild<QDockWidget *>("dock_PlaceHolder");
        QDockWidget *dockTop = nullptr;
        for(auto* dock : docks){
            if(dock!=dockStack){
                auto act = new QAction(dock->windowTitle());
                act->setCheckable(true);
                act->setChecked(true);
                this->tabifyDockWidget(dockStack, dock);
                if(!dockTop)
                    dockTop = dock;
                docksViewMode.insert(dock->windowTitle(), dock);
                connect(act, &QAction::triggered,
                        this, std::bind(&MainWindow::viewModeChecked, this, std::placeholders::_1));
                view->addAction(act);
            }
        }
        dockStack->setDisabled(true);
        dockTop->raise();
        auto tabBar = this->findChild<QTabBar *>();
        for(int i = 0; i < tabBar->count(); i++){
            if(tabBar->tabText(i)==""){
                for(int j = i+1; j < tabBar->count(); j++)
                    tabBar->moveTab(i++, j);
                break;
            }

        }
        tabBar->setTabEnabled(tabBar->count()-1, false);
        tabBar->setMovable(false);
        tabBar->setObjectName("tabBar_Docks");

    }

    connect(ui->actionCopy, &QAction::triggered,
            this, std::bind(&MainWindow::myCopy, this));
    connect(ui->actionPaste, &QAction::triggered,
            this, std::bind(&MainWindow::myPaste, this));


    nodeMovementGroup = new QActionGroup(this);
    nodeMovementGroup->addAction(ui->actionAutomatic);
    nodeMovementGroup->addAction(ui->actionManual);
    ui->actionManual->setChecked(true);
    for(auto& i: ui->menubar->children()){
        ((QMenu*)i)->setWindowFlag(Qt::FramelessWindowHint);
        ((QMenu*)i)->setWindowFlag(Qt::NoDropShadowWindowHint);
        ((QMenu*)i)->setAttribute(Qt::WA_TranslucentBackground);
    }
}

MainWindow::~MainWindow()
{
    delete nodeMovementGroup;
    delete ui;
}

void MainWindow::buttonClearConsoleClicked()
{
    ui->textEdit_Console->clear();
}

void MainWindow::pasteClipboardToTable(QTableView *dest)
{
    if(!dest->hasFocus()) return;
    static QRegularExpression re("\t|(?=\n)");
    QStandardItemModel * model = static_cast<QStandardItemModel*>(dest->model());
    QItemSelectionModel * selection = dest->selectionModel();
    QModelIndexList indexes = selection->selectedIndexes();
    QString text = QApplication::clipboard()->text();
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

void MainWindow::viewModeChecked(bool checked)
{
    auto act = qobject_cast<QAction*>(sender());
    auto *dock = docksViewMode[act->text()];
    dock->setVisible(checked);
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
}

void MainWindow::copyTableToClipboard(QTableView *src){
    if(!src->hasFocus()) return;
    QAbstractItemModel * model = src->model();
    QItemSelectionModel * selection = src->selectionModel();
    QModelIndexList indexes = selection->selectedIndexes();
    std::sort(indexes.begin(), indexes.end());
    QString selected_text;
    int indexes_count = indexes.count();
    if(!indexes_count) return;
    QModelIndex *current, *previous = &indexes.first();
    std::set<int> rows{previous->row()}, columns{previous->column()};
    bool canCopy = true;
    QString text = model->data(*previous).toString();
    selected_text.append(text);
    for(int i = 1; (i<indexes_count); i++)
    {
        rows.insert(indexes[i].row());
        columns.insert(indexes[i].column());
        if((int)(columns.size()*rows.size()) > indexes_count){
            canCopy = false;
            break;
        }
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
    if(canCopy){
        QApplication::clipboard()->setText(selected_text+'\n');
    }
    else
        QMessageBox::warning(src,
                             this->windowTitle(),
                             "This action won't work on multiple selections.",
                             QMessageBox::StandardButton::Ok,
                             QMessageBox::StandardButton::Ok);

}

void MainWindow::myCopy()
{
    auto widget = QApplication::focusWidget();
    auto table = qobject_cast<QTableView *>(widget);
    if( table != nullptr ){
        copyTableToClipboard(table);
    }
}
void MainWindow::myPaste()
{
    auto widget = QApplication::focusWidget();
    auto table = qobject_cast<QTableView *>(widget);
    if( table != nullptr ){
        pasteClipboardToTable(table);
    }
}
