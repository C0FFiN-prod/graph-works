#include "mainwindow.h"
#include "titlebar.h"
#include "ui_mainwindow.h"
#include "QLayout"
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    auto flags = this->windowFlags();
    this->setWindowFlags(flags|Qt::FramelessWindowHint);
    titlebar = new TitleBar(this);
    ui->setupUi(this);
    this->setMenuWidget(titlebar);
    qApp->installEventFilter(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj->objectName() == titlebar->objectName()){

        if(event->type() == QEvent::MouseButtonPress)
        {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
            leftButtonPressed = (mouseEvent->button() & Qt::LeftButton);
            if(leftButtonPressed)
                m_dragPosition = mouseEvent->globalPosition() - frameGeometry().topLeft();
        }
        if((event->type() == QEvent::MouseMove)&&leftButtonPressed){
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
            move((mouseEvent->globalPosition() - m_dragPosition).toPoint());
        }
        if((event->type() == QEvent::MouseButtonRelease)){
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
            leftButtonPressed = !(mouseEvent->button() & Qt::LeftButton);
        }
    }
    return false;
}
