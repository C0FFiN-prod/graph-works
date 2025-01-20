#include "mainwindow.h"
#include <QClipboard>
#include <QMessageBox>
#include <QQueue>
#include <QShortcut>
#include <QStandardItemModel>
#include <QThread>
#include <QToolTip>
#include "qspinbox.h"
#include "ui_mainwindow.h"
#include <set>

const QRegularExpression MainWindow::reValidDoubleLine("([0-9]+(\\.[0-9]+)?(\t|\n))+");
const QRegularExpression MainWindow::reValidDouble("[0-9]+(\\.[0-9]+)?");
const QRegularExpression MainWindow::reValidInt("[0-9]+");

template<typename Func, typename... Args>
auto timer(Func func, long long &ms, Args &&...args) -> decltype(func(std::forward<Args>(args)...))
{
    using namespace std::chrono;
    auto start = system_clock::now();
    if constexpr (std::is_void_v<decltype(func(std::forward<Args>(args)...))>) {
        func(std::forward<Args>(args)...);
        auto end = system_clock::now();
        ms = duration_cast<milliseconds>(end - start).count();
        return;
    } else {
        auto result = func(std::forward<Args>(args)...);
        auto end = system_clock::now();
        ms = duration_cast<milliseconds>(end - start).count();
        return result;
    }
}

MainWindow::MainWindow(const QString &title, QWidget *parent)
    : QMainWindow(parent)
    , sequencer(Sequencer(graph.graphView))
    , ui(new Ui::MainWindow)
    , title(title)
{
    ui->setupUi(this);

    //setting up extra widgets
    currentFrameLabel = new QLabel("0/0", this);
    ui->toolBar->insertWidget(ui->actionFirstFrame, currentFrameLabel);
    delayBetweenFramesSlider = new QSlider(Qt::Orientation::Horizontal, this);
    delayBetweenFramesSlider->setOrientation(Qt::Horizontal);
    delayBetweenFramesSlider->setMaximum(1000);
    delayBetweenFramesSlider->setMinimum(0);
    delayBetweenFramesSlider->setTickInterval(10);
    delayBetweenFramesSlider->setMaximumSize(100, 20);

    connect(delayBetweenFramesSlider, &QSlider::valueChanged, [this](int value) {
        QToolTip::showText(delayBetweenFramesSlider->mapToGlobal(
                               delayBetweenFramesSlider->rect().center()),
                           QString::number(value) + " ms",
                           delayBetweenFramesSlider);
    });

    // Скрываем tooltip, когда слайдер не активен
    connect(delayBetweenFramesSlider, &QSlider::sliderReleased, this, []() {
        QToolTip::hideText();
    });

    ui->toolBar->addWidget(delayBetweenFramesSlider);

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
                QStandardItemModel *model;
                if(name.endsWith("Edges")){
                    model = new QStandardItemModel(1, 5);
                    table->setModel(model);
                    model->setHeaderData(0, Qt::Horizontal, "U");
                    model->setHeaderData(1, Qt::Horizontal, "V");
                    model->setHeaderData(2, Qt::Horizontal, "Weight");
                    model->setHeaderData(3, Qt::Horizontal, "Band");
                    model->setHeaderData(4, Qt::Horizontal, "Flow");
                    for (int i = 5; i--;) {
                        model->setItem(0, i, new QStandardItem());
                        table->setColumnWidth(i, 20);
                    }
                }
                connect(model, &QStandardItemModel::dataChanged, this, &MainWindow::listDataChanged);
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

    connect(ui->actionBandwidth, &QAction::triggered, this, [this](bool checked) {
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
    connect(ui->actionAddNode, &QAction::triggered, this, &MainWindow::addNode);
    connect(ui->actionDeleteNode,
            &QAction::triggered,
            this,
            std::bind(&MainWindow::deleteSelectedObjects, this, DeleteOptions::Nodes));
    connect(ui->actionDeleteEdges,
            &QAction::triggered,
            this,
            std::bind(&MainWindow::deleteSelectedObjects, this, DeleteOptions::Edges));
    connect(ui->actionDeleteSelected,
            &QAction::triggered,
            this,
            std::bind(&MainWindow::deleteSelectedObjects,
                      this,
                      QFlags<DeleteOptions>(DeleteOptions::Nodes | DeleteOptions::Edges)));
    connect(ui->actionSelectSourceNode,
            &QAction::triggered,
            this,
            std::bind(&MainWindow::markSelectedAs, this, SelectOptions::Source));
    connect(ui->actionSelectDestinationNode,
            &QAction::triggered,
            this,
            std::bind(&MainWindow::markSelectedAs, this, SelectOptions::Destination));
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

    auto actGroup = new QActionGroup(this);
    actionGroups.append(actGroup);
    actGroup->addAction(ui->actionAutomatic);
    actGroup->addAction(ui->actionManual);
    connect(ui->actionManual, &QAction::triggered, this,
            [this](bool checked){if (checked) graph.setFlag(GraphFlags::ManualMode); graph.graphView->scene()->update();});
    connect(ui->actionAutomatic, &QAction::triggered, this,
            [this](bool checked){if (checked) {graph.unsetFlag(GraphFlags::ManualMode); graph.graphView->runTimer();}});
    ui->actionAutomatic->setChecked(true);

    actGroup = new QActionGroup(this);
    actionGroups.append(actGroup);
    actGroup->addAction(ui->actionDisplayIndex);
    actGroup->addAction(ui->actionDisplayName);
    connect(ui->actionDisplayIndex, &QAction::triggered, this, [this](bool checked) {
        if (checked) {
            graph.setFlag(GraphFlags::DisplayIndex);
            graph.updateNodes();
        }
    });
    connect(ui->actionDisplayName, &QAction::triggered, this, [this](bool checked) {
        if (checked) {
            graph.unsetFlag(GraphFlags::DisplayIndex);
            graph.updateNodes();
        }
    });
    ui->actionDisplayName->setChecked(true);

    for (auto &i : ui->menubar->children()) {
        ((QMenu*)i)->setWindowFlag(Qt::FramelessWindowHint);
        ((QMenu*)i)->setWindowFlag(Qt::NoDropShadowWindowHint);
        ((QMenu*)i)->setAttribute(Qt::WA_TranslucentBackground);
    }

    // Connecting algorithms
    connect(ui->actionFloydWarshall, &QAction::triggered, this, &MainWindow::algorithmFloYdWarshall);
    connect(ui->actionDijkstra, &QAction::triggered, this, &MainWindow::algorithmDijkstra);
    connect(ui->actionDinic, &QAction::triggered, this, &MainWindow::algorithmDinic);
    connect(ui->actionBellmanFord, &QAction::triggered, this, &MainWindow::algorithmBellmanFord);
    connect(ui->actionNetTransportProblem,
            &QAction::triggered,
            this,
            &MainWindow::algorithmNetTransportProblem);

    connect(ui->actionSave,
            &QAction::triggered,
            this,
            std::bind(&MainWindow::saveGraphToCSV, this, false));
    connect(ui->actionSaveAs,
            &QAction::triggered,
            this,
            std::bind(&MainWindow::saveGraphToCSV, this, true));
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::readGraphFromCSV);
    connect(ui->actionNew, &QAction::triggered, this, &MainWindow::newGraph);
    connect(ui->actionClearGraph, &QAction::triggered, this, &MainWindow::clearGraph);
    connect(ui->actionClearConsole, &QAction::triggered, this, &MainWindow::clearConsole);
    connect(ui->button_ClearConsole, &QPushButton::pressed, this, &MainWindow::clearConsole);

    updateFileStatus();

    // Connecting sequencer actions
    connect(ui->actionNextFrame, &QAction::triggered, this, [this]() {
        long long ms;
        timer([this]() { sequencer.next(); }, ms);
        qDebug() << "Invoked method next: took " << ms << " ms";
        handleSequencerFrameChange();
        QThread::msleep(qMax(delayBetweenFramesSlider->sliderPosition() - ms, 0));
    });
    connect(ui->actionPreviousFrame, &QAction::triggered, this, [this]() {
        long long ms;
        timer([this]() { sequencer.prev(); }, ms);
        qDebug() << "Invoked method prev: took " << ms << " ms";

        handleSequencerFrameChange();
        QThread::msleep(qMax(delayBetweenFramesSlider->sliderPosition() - ms, 0));
    });
    connect(ui->actionFirstFrame, &QAction::triggered, this, [this]() {
        sequencer.first();
        handleSequencerFrameChange();
    });
    connect(ui->actionLastFrame, &QAction::triggered, this, [this]() {
        sequencer.last();
        handleSequencerFrameChange();
    });
    connect(ui->actionClearSequence, &QAction::triggered, this, [this]() {
        sequencer.clear();
        handleSequencerFrameChange();
    });
    //disabling sequencer btns and controls
    ui->actionFirstFrame->setDisabled(true);
    ui->actionNextFrame->setDisabled(true);
    ui->actionPreviousFrame->setDisabled(true);
    ui->actionLastFrame->setDisabled(true);
    ui->actionClearSequence->setDisabled(true);
    delayBetweenFramesSlider->setDisabled(true);
}

MainWindow::~MainWindow()
{
    for (auto &act : actionGroups)
        delete act;
    delete ui;
}

void MainWindow::clearConsole()
{
    ui->textEdit_Console->clear();
}

void MainWindow::pasteClipboardToTable(QTableView *dest)
{
    if(!dest->hasFocus()) return;
    static QRegularExpression reSplit("\t|(?=\n)");
    QString text = QApplication::clipboard()->text();
    if (!reValidDoubleLine.match(text).hasMatch())
        return;
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

void MainWindow::handleSequencerFrameChange()
{
    int currentPosition = sequencer.getPosition();
    int maxPosition = sequencer.getFramesLength() - 1;
    currentFrameLabel->setText(QString::number(currentPosition + 1) + " / "
                               + QString::number(maxPosition + 1));

    ui->actionFirstFrame->setDisabled(false);
    ui->actionNextFrame->setDisabled(false);
    ui->actionPreviousFrame->setDisabled(false);
    ui->actionLastFrame->setDisabled(false);
    ui->actionClearSequence->setDisabled(false);

    if (currentPosition == maxPosition && currentPosition != -1) {
        ui->actionNextFrame->setDisabled(true);
        ui->actionLastFrame->setDisabled(true);
    } else if (currentPosition == maxPosition) {
        ui->actionFirstFrame->setDisabled(true);
        ui->actionNextFrame->setDisabled(true);
        ui->actionPreviousFrame->setDisabled(true);
        ui->actionLastFrame->setDisabled(true);
        ui->actionClearSequence->setDisabled(true);
        delayBetweenFramesSlider->setDisabled(true);
    } else if (currentPosition < 0) {
        ui->actionPreviousFrame->setDisabled(true);
        ui->actionFirstFrame->setDisabled(true);
    }
}

void MainWindow::initSequencer(bool isSequenceStateless)
{
    sequencer.isSequenceStateless = isSequenceStateless;
    if (this->sequencer.getFramesLength() == 0) {
        qDebug() << "Sequencer cannot be prepared: frames are empty";
        return;
    }
    delayBetweenFramesSlider->setDisabled(false);
    ui->actionLastFrame->setDisabled(false);
    ui->actionNextFrame->setDisabled(false);
    ui->actionClearSequence->setDisabled(false);
    currentFrameLabel->setText(QString::number(sequencer.getPosition() + 1) + "/"
                               + QString::number(sequencer.getFramesLength()));
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
    } else if (oldAmount == newAmount && oldAmount) {
        if (model->item(oldAmount-1, oldAmount-1)->flags() == Qt::NoItemFlags){
            int j, i = oldAmount-1;
            for (j=i+1; j--;){
                model->item(i, j)->setFlags(flags);
                model->item(j, i)->setFlags(flags);
            }
        }
    } else if (oldAmount < newAmount) {
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
    bool ok;
    for(int i = 0; i < newAmount; i++){
        m[i].resize(newAmount);
        for(int j = 0; j < newAmount; j++){
            m[i][j] = model->item(i, j)->text().toDouble(&ok);
            if (!ok) {
                QMessageBox::warning(this,
                                     title,
                                     QString("Cell (%1,%2) contains invalid data").arg(i).arg(j));
                return;
            }
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
    updateFileStatus();
    graph.graphView->stabilizingIteration = 0;
    if (!(graph.getFlags() & GraphFlags::ManualMode))
        graph.graphView->runTimer();
}

void MainWindow::applyEdgesList(QTableView *table)
{
    static QRegularExpression reNumInt("[0-9]+");
    auto model = static_cast<QStandardItemModel *>(table->model());
    int rowCount = model->rowCount() - 1;
    int amount = graph.getAmount();
    int u, v;
    QString su, sv;
    double weight, bandwidth, flow;
    QSet<std::pair<int, int>> addedEdges;
    QMap<QString, int> nodes;
    for (int i = 0; i != rowCount; ++i) {
        su = model->item(i, 0)->text();
        sv = model->item(i, 1)->text();
        if (!model->item(i, 0)->flags().testFlag(Qt::ItemIsEditable)) {
            u = model->item(i, 0)->data().toInt();
            nodes[su] = u;
        }
        if (!model->item(i, 1)->flags().testFlag(Qt::ItemIsEditable)) {
            v = model->item(i, 1)->data().toInt();
            nodes[sv] = v;
        }
        if (su.isEmpty() || sv.isEmpty())
            continue;
        if (nodes.contains(su))
            u = nodes[su];
        else {
            if (reNumInt.match(su).hasMatch())
                u = su.toInt();
            else {
                u = amount + 1;
            }
        }

        if (nodes.contains(sv))
            v = nodes[sv];
        else {
            if (reNumInt.match(sv).hasMatch())
                v = sv.toInt();
            else {
                v = amount + 1;
            }
        }

        weight = model->item(i, 2)->text().toDouble();
        if (weight) {
            if (addedEdges.contains(qMakePair(u, v))) {
                auto result = QMessageBox::warning(
                    table,
                    this->windowTitle(),
                    QString("Row %1: Edge %2 - %3 already exists").arg(i).arg(u).arg(v),
                    QMessageBox::StandardButton::Abort | QMessageBox::StandardButton::Ignore
                        | QMessageBox::StandardButton::Apply);
                if (result == QMessageBox::StandardButton::NoButton
                    || result == QMessageBox::StandardButton::Abort) {
                    return;
                } else if (result == QMessageBox::StandardButton::Ignore) {
                    continue;
                } else if (result == QMessageBox::StandardButton::Apply) {
                }
            }
            if (amount <= u) {
                graph.addNode(amount, su);
                nodes[su] = amount;
                u = amount++;
            }
            if (amount <= v) {
                graph.addNode(amount, sv);
                nodes[sv] = amount;
                v = amount++;
            }
            bandwidth = model->item(i, 3)->text().toDouble();
            flow = model->item(i, 4)->text().toDouble();
            if (graph.edgeExists(u, v))
                graph.setEdgeWeight(u, v, weight);
            else
                graph.setEdge(u, v, weight);
            graph.setEdgeBandwidth(u, v, bandwidth);
            graph.setEdgeFlow(u, v, flow);
            addedEdges.insert(qMakePair(u, v));
        } else {
            graph.removeEdge(u, v);
        }
    }
    graph.graphView->initScene();
    graph.graphView->scene()->update();
    updateTables();
    updateFileStatus();
}

void MainWindow::updateEdgesList(QTableView *list)
{
    auto edges = graph.getListEdges();
    int count = edges.count();
    ui->label_ListEdges_Count->setText(QString::number(count));
    auto model = static_cast<QStandardItemModel *>(list->model());
    if (!count) {
        model->setRowCount(0);
        model->setColumnCount(5);
        return;
    }
    model->setColumnCount(edges[0].count());

    int oldRowCount = model->rowCount();
    int colCount = edges[0].count();
    model->setRowCount(count);
    for(int i = 0; i < count; i++){
        list->setRowHeight(i, 20);

        for (int j = 0; j < colCount; j++) {
            if (i >= oldRowCount)
                model->setItem(i,
                               j,
                               new QStandardItem((j < 2) ? (graph.getNodeName(edges[i][j].toInt()))
                                                         : (edges[i][j].toString())));
            else
                model->item(i, j)->setText((j < 2) ? (graph.getNodeName(edges[i][j].toInt()))
                                                   : (edges[i][j].toString()));
            model->item(i, j)->setData(edges[i][j].toString());
        }
        auto item = model->item(i, 0);
        item->setFlags(item->flags() & (~Qt::ItemIsEditable));
        item = model->item(i, 1);
        item->setFlags(item->flags() & (~Qt::ItemIsEditable));
    }

    //emit model->dataChanged(model->item(0,0)->index(),model->item(count-1, colCount-1)->index());
}
void MainWindow::addNode()
{
    int amount = graph.getAmount();
    graph.addNode(amount, QString::number(amount));
    for (auto &spin : graphCountSpins) {
        spin->setValue(amount + 1);
    }
    graph.graphView->initScene();
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
        addRowToList(static_cast<QStandardItemModel *>(list->model()));
    }
}

void MainWindow::addRowToList(QStandardItemModel *model)
{
    int colCount = model->columnCount();
    int rowCount = model->rowCount();
    QList<QStandardItem *> items(colCount);
    for (int i = colCount; i--;)
        items[i] = new QStandardItem("");
    model->insertRow(rowCount, items);
}

void MainWindow::consoleLog(const QString &text)
{
    ui->textEdit_Console->appendPlainText(text);
}

void MainWindow::addDockWidget(const QList<QWidget *> &widgets,
                               const QString &title,
                               bool closable,
                               bool floating)
{
    QDockWidget *dock = new QDockWidget(this);
    auto container = new QWidget(this);
    auto layout = new QVBoxLayout(container);
    for (auto &widget : widgets)
        layout->addWidget(widget);
    dock->setWidget(container);
    dock->setFloating(floating);
    dock->setWindowTitle(title);
    if (closable)
        dock->setFeatures(QDockWidget::DockWidgetClosable);
    dock->show();
}
void MainWindow::deleteSelectedObjects(const QFlags<DeleteOptions> &options)
{
    //QHash<QGraphicsItem *, DeleteOptions> toDelete;
    QSet<QPair<QGraphicsItem *, DeleteOptions>> toDelete;
    for (auto &item : graph.graphView->items()) {
        if (item != nullptr) {
            if (item->isSelected()) {
                if (Node *node = qgraphicsitem_cast<Node *>(item)) {
                    if (options.testFlag(DeleteOptions::Nodes)) {
                        toDelete.insert(qMakePair(node, DeleteOptions::Nodes));
                    }
                } else if (Edge *edge = qgraphicsitem_cast<Edge *>(item)) {
                    if (options.testFlag(DeleteOptions::Edges)) {
                        toDelete.insert(qMakePair(edge, DeleteOptions::Edges));
                    }
                }
            }
        }
    }
    for (auto &i : toDelete) {
        auto deleteConnected = [&i](const QPair<QGraphicsItem *, DeleteOptions> &j) {
            return ((Node *) i.first)->edgeList.contains((Edge *) (j.first));
        };
        unsigned src;
        unsigned dst;
        switch (i.second) {
        case DeleteOptions::Nodes:
            toDelete.removeIf(deleteConnected);

            graph.removeNode(((Node *) i.first)->getIndex());
            break;
        case DeleteOptions::Edges:
            src = ((Edge *) i.first)->sourceNode()->getIndex();
            dst = ((Edge *) i.first)->destNode()->getIndex();
            graph.removeEdge(src, dst);
            if (((Edge *) i.first)->getEdgeType() == EdgeType::BiDirectionalSame)
                graph.removeEdge(dst, src);
            break;
        default:
            break;
        }
    }
    updateTables();
    updateFileStatus();
    graph.graphView->initScene();
}

void MainWindow::markSelectedAs(const QFlags<SelectOptions> &option)
{
    for (auto &item : graph.graphView->items()) {
        if (item != nullptr) {
            if (item->isSelected()) {
                if (Node *node = qgraphicsitem_cast<Node *>(item)) {
                    if (option == SelectOptions::Source) {
                        graph.setSourceIndex(node->getIndex());
                    } else if (option == SelectOptions::Destination) {
                        graph.setDestIndex(node->getIndex());
                    }
                }
            }
        }
    }
    graph.graphView->initScene();
}

void MainWindow::listDataChanged(const QModelIndex &topLeft,
                                 const QModelIndex &bottomRight,
                                 const QVector<int> &)
{
    auto model = static_cast<QStandardItemModel *>(sender());
    int lastRow = model->rowCount() - 1;
    if (topLeft.row() == lastRow && bottomRight.row() == lastRow) {
        addRowToList(model);
    }
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
void MainWindow::setTableFromMatrix(QTableView *table, const T &matrix, int height, int width)
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
        model->setHeaderData(i, Qt::Vertical, i);
        table->setColumnWidth(i, 20);
        for (int j = width; j--;) {
            model->setHeaderData(j, Qt::Horizontal, j);
            if ((i >= rowCount) || (j >= colCount))
                model->setItem(i, j, new QStandardItem(QString::number(matrix[i][j])));
            else {
                model->item(i, j)->setData(QString::number(matrix[i][j]));
                model->item(i, j)->setText(QString::number(matrix[i][j]));
            }
        }
    }
    emit model->dataChanged(model->item(0, 0)->index(), model->item(height - 1, width - 1)->index());
}

template void MainWindow::setTableFromMatrix(QTableView *table,
                                             const Matrix2D &matrix,
                                             int height,
                                             int width);
template void MainWindow::setTableFromMatrix(QTableView *table,
                                             const Matrix2I &matrix,
                                             int height,
                                             int width);

template<typename T>
QTableView *MainWindow::makeTableFromMatrix(
    const T &matrix, int height, int width, bool editable, int headerVSize, int headerHSize)
{
    auto table = new QTableView();
    table->setModel(new QStandardItemModel(0, 0));
    if (!editable)
        table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->verticalHeader()->setDefaultSectionSize(headerVSize);
    table->horizontalHeader()->setDefaultSectionSize(headerHSize);
    setTableFromMatrix(table, matrix, height, width);
    return table;
}

template QTableView *MainWindow::makeTableFromMatrix(
    const Matrix2D &matrix, int height, int width, bool editable, int headerVSize, int headerHSize);
template QTableView *MainWindow::makeTableFromMatrix(
    const Matrix2I &matrix, int height, int width, bool editable, int headerVSize, int headerHSize);

void MainWindow::updateFileStatus()
{
    QString newTitle;
    if (graph.isUnsaved()) {
        newTitle += '*';
        sequencer.clear();
    }
    if (!currentFile.isEmpty())
        newTitle += currentFile + " - ";
    newTitle += title;
    this->setWindowTitle(newTitle);
}
