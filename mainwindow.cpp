#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QToolTip>
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

