#include "mainwindow.h"
#include "qspinbox.h"
#include "ui_mainwindow.h"
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
    auto pushButtons = this->findChildren<QPushButton *>();
    for(auto* table : this->findChildren<QTableView *>()){
        if(table->objectName().endsWith("Graph")){
            auto name = table->objectName().replace("table_", "").replace("_Graph", "");

            if(name.startsWith("Matrix")) {
                graphMatrixViews.append(table);
            }
            else if (name.startsWith("List")) {
                graphListViews.append(table);
                if(name.endsWith("Edges")){
                    table->setModel(new QStandardItemModel(0, 0));
                }
            }

            for(auto *spinBox : spinBoxes){
                if(spinBox->objectName().startsWith("spin_"+name))
                {
                    if(spinBox->objectName().endsWith("NodesCount")){
                        if(name.startsWith("Matrix")){
                            connect(spinBox,
                                    &QSpinBox::valueChanged,
                                    this,
                                    std::bind(&MainWindow::setNodesAmountMatrix,
                                              this, table, std::placeholders::_1));
                        }
                        graphCountSpins.insert(table->objectName(), spinBox);
                        break;
                    }
                }
            }

            for(auto *button : pushButtons){
                if(button->objectName().startsWith("button_"+name) &&
                    button->objectName().endsWith("_Apply"))
                {
                    if (name.startsWith("Matrix"))
                        connect(button,
                                &QPushButton::clicked,
                                this,
                                std::bind(&MainWindow::applyGraphMatrix, this, table));
                    else if (name == "ListEdges") {
                        connect(button,
                                &QPushButton::clicked,
                                this,
                                std::bind(&MainWindow::applyEdgesList, this, table));
                    }
                }
            }
        }
        if(table->model()==nullptr)
            table->setModel(new QStandardItemModel(0,0));
    }

    connect(ui->actionBandwidth, &QAction::triggered, this, [this](bool checked){
        if(checked)
            this->graph.setFlag(GraphFlags::ShowBandwidth);
        else
            this->graph.unsetFlag(GraphFlags::ShowBandwidth);
        graph.graphView->scene()->update();
    });

    connect(ui->actionWeights, &QAction::triggered, this, [this](bool checked){
        if(checked)
            this->graph.setFlag(GraphFlags::ShowWeights);
        else
            this->graph.unsetFlag(GraphFlags::ShowWeights);
        graph.graphView->scene()->update();
    });

    connect(ui->actionFlow, &QAction::triggered, this, [this](bool checked){
        if(checked)
            this->graph.setFlag(GraphFlags::ShowFlow);
        else
            this->graph.unsetFlag(GraphFlags::ShowFlow);
        graph.graphView->scene()->update();
    });

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
    ui->centralwidget->layout()->removeWidget(ui->graphView);
    ui->centralwidget->layout()->addWidget(graph.graphView);
    connect(ui->actionCopy, &QAction::triggered,
            this, std::bind(&MainWindow::myCopy, this));
    connect(ui->actionPaste, &QAction::triggered,
            this, std::bind(&MainWindow::myPaste, this));

    connect(ui->actionRefreshTables,
            &QAction::triggered,
            this,
            std::bind(&MainWindow::updateTables, this));

    nodeMovementGroup = new QActionGroup(this);
    nodeMovementGroup->addAction(ui->actionAutomatic);
    nodeMovementGroup->addAction(ui->actionManual);
    connect(ui->actionManual, &QAction::triggered, this,
            [this](bool checked){if (checked) graph.setFlag(GraphFlags::ManualMode); graph.graphView->scene()->update();});
    connect(ui->actionAutomatic, &QAction::triggered, this,
            [this](bool checked){if (checked) {graph.unsetFlag(GraphFlags::ManualMode); graph.graphView->runTimer();}});
    ui->actionAutomatic->setChecked(true);
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
    static QRegularExpression reSplit("\t|(?=\n)");
    static QRegularExpression reValid("([0-9]+(\\.[0-9]+)?(\t|\n))+");
    QString text = QApplication::clipboard()->text();
    if(!reValid.match(text).hasMatch()) return;
    QStandardItemModel * model = static_cast<QStandardItemModel*>(dest->model());
    QItemSelectionModel * selection = dest->selectionModel();
    QModelIndexList indexes = selection->selectedIndexes();
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
    auto data = text.split(reSplit);
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

void MainWindow::setNodesAmountMatrix(QTableView *table, int newAmount)
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

void MainWindow::applyGraphMatrix(QTableView *table)
{
    QStandardItemModel* model = static_cast<QStandardItemModel*>(table->model());
    int oldAmount = model->rowCount();
    int newAmount = graphCountSpins[table->objectName()]->value();
    for(auto [key, value] : graphCountSpins.asKeyValueRange()){
        if (key != table->objectName()){
            value->setValue(newAmount);
        }
    }
    if(oldAmount > newAmount){
        model->setColumnCount(newAmount);
        model->setRowCount(newAmount);
    }
    Matrix2D m(newAmount);
    for(int i = 0; i < newAmount; i++){
        m[i].resize(newAmount);
        for(int j = 0; j < newAmount; j++){
            m[i][j] = model->item(i, j)->text().toDouble();
        }
    }
    if(table->objectName().contains("Adj"))
        graph.setMatrixAdjacent(m);
    else if(table->objectName().contains("Flow"))
        graph.setMatrixFlow(m);
    else if(table->objectName().contains("Bandwidth"))
        graph.setMatrixBandwidth(m);
    graph.graphView->initScene();
    graph.graphView->scene()->update();
    updateTables();
}

void MainWindow::applyEdgesList(QTableView *table)
{
    auto model = static_cast<QStandardItemModel *>(table->model());
    int rowCount = model->rowCount();
    int amount = graph.getAmount();
    int u, v;
    double weight, bandwidth, flow;
    for (int i = 0; i != rowCount; ++i) {
        u = model->item(i, 0)->text().toInt();
        v = model->item(i, 1)->text().toInt();
        weight = model->item(i, 2)->text().toDouble();
        if ((u < amount) && (v < amount) && weight) {
            bandwidth = model->item(i, 3)->text().toDouble();
            flow = model->item(i, 4)->text().toDouble();
            graph.setEdgeWeight(u, v, weight);
            graph.setEdgeBandwidth(u, v, bandwidth);
            graph.setEdgeFlow(u, v, flow);
        } else {
            graph.removeEdge(u, v);
        }
    }
    graph.graphView->scene()->update();
    updateTables();
}

void MainWindow::updateEdgesList(QTableView *list)
{
    auto edges = graph.getListEdges();
    auto model = static_cast<QStandardItemModel *>(list->model());
    if(edges.empty()) {
        model->setRowCount(0);
        model->setColumnCount(0);
        return;
    }
    model->setColumnCount(edges[0].count());
    model->setHeaderData(0, Qt::Horizontal, "U");
    model->setHeaderData(1, Qt::Horizontal, "V");
    model->setHeaderData(2, Qt::Horizontal, "Weight");
    model->setHeaderData(3, Qt::Horizontal, "Band");
    model->setHeaderData(4, Qt::Horizontal, "Flow");
    int count = edges.count();
    int oldRowCount = model->rowCount();
    int colCount = edges[0].count();
    ui->label_ListEdges_Count->setText(QString::number(count));
    model->setRowCount(count);
    for(int i = 0; i < count; i++){
        list->setRowHeight(i, 20);
        if(i >= oldRowCount)
            for(int j = 0; j < colCount; j++){
                model->setItem(i,j,new QStandardItem(edges[i][j].toString()));
            }
        else {
            for(int j = 0; j < colCount; j++){
                model->item(i, j)->setText(edges[i][j].toString());
                model->item(i, j)->setData(edges[i][j].toString());
            }
        }
        auto item = model->item(i, 0);
        item->setFlags(item->flags()&(~Qt::ItemIsEditable));
        item = model->item(i, 1);
        item->setFlags(item->flags()&(~Qt::ItemIsEditable));
    }
    for(int i = colCount; i--;){
        list->setColumnWidth(i, 20);
    }
    emit model->dataChanged(model->item(0,0)->index(),model->item(count-1, colCount-1)->index());
}

void MainWindow::updateTables()
{
    static QRegularExpression nameRe("(table_(Matrix|List))|(_Graph)");
    unsigned int amount = graph.getAmount();
    for (auto &spin : graphCountSpins) {
        spin->setValue(amount);
    }
    QString tableName;
    for (auto &table : graphMatrixViews) {
        tableName = table->objectName().replace(nameRe, "");
        if (tableName == "Adj") {
            setTableFromMatrix(table, graph.getMatrixAdjacent(), amount, amount);
        } else if (tableName == "Flow") {
            setTableFromMatrix(table, graph.getMatrixFlow(), amount, amount);
        } else if (tableName == "Bandwidth") {
            setTableFromMatrix(table, graph.getMatrixBandwidth(), amount, amount);
        }
    }
    for (auto &list : graphListViews) {
        tableName = list->objectName().replace(nameRe, "");
        if (tableName == "Edges") {
            updateEdgesList(list);
        }
    }
    graph.unsavedChanges = false;
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

template<typename T>
void MainWindow::setTableFromMatrix(QTableView *table, T &matrix, int height, int width)
{
    if ((height == -1) || (height > matrix.size()))
        height = matrix.size();
    if (height && ((width == -1) || (width > matrix[0].size())))
        width = matrix[0].size();
    if (!height || !width)
        return;
    auto *model = static_cast<QStandardItemModel *>(table->model());
    int colCount = model->columnCount();
    int rowCount = model->rowCount();

    if (height > rowCount)
        model->insertRows(rowCount, height - rowCount);
    if (width > colCount)
        model->insertColumns(colCount, width - colCount);

    for (int i = height; i--;) {
        model->setHeaderData(i, Qt::Horizontal, i);
        model->setHeaderData(i, Qt::Vertical, i);
        table->setColumnWidth(i, 20);
        table->setRowHeight(i, 20);
        for (int j = width; j--;) {
            if ((i > rowCount) || (j > colCount))
                model->setItem(i, j, new QStandardItem(QString::number(matrix[i][j])));
            else {
                model->item(i, j)->setData(QString::number(matrix[i][j]));
                model->item(i, j)->setText(QString::number(matrix[i][j]));
            }
        }
    }
    emit model->dataChanged(model->item(0, 0)->index(), model->item(height - 1, width - 1)->index());
}
