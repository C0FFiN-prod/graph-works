#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "graphwidget.h"
#include "graph.h"


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
        connect(act, &QAction::toggled, this, &MainWindow::on_viewMode_checked);
        view->addAction(act);
        for(auto button: tab->findChildren<QPushButton *>()){
            if(button->objectName().startsWith("buttonPin")){
                pins[tab] = button;
                connect(button, &QPushButton::clicked, this, &MainWindow::on_buttonPin_toggled);
            }
        }
    }

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_buttonClearConsole_clicked()
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





void MainWindow::on_buttonPin_toggled(bool checked)
{
    if(!checked){
        pinTab();
    }else{
        unpinTab();
    }
}

void MainWindow::on_viewMode_checked(bool checked)
{
    auto act = qobject_cast<QAction*>(sender());
    auto *tab = tabs[act->text()];
    ui->tabWidget->setTabVisible(ui->tabWidget->indexOf(tab), checked);
}





void MainWindow::on_buttonApplyAdjMatr_clicked()
{


}


void MainWindow::on_actionAddNode_triggered()
{

}

