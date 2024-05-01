#include "titlebar.h"
#include "ui_titlebar.h"

TitleBar::TitleBar(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TitleBar)
    , parent(parent)
{
    ui->setupUi(this);
    connect(ui->pushButton_Close, &QPushButton::clicked, this, &TitleBar::close);
    connect(ui->pushButton_Hide, &QPushButton::clicked, this, &TitleBar::hide);
    connect(ui->pushButton_FullScreen, &QPushButton::clicked, this, &TitleBar::maximize);
}

TitleBar::~TitleBar()
{
    delete ui;
}



void TitleBar::close()
{
    parent->close();
}
void TitleBar::hide()
{
    parent->showMinimized();
}
void TitleBar::maximize()
{
    if(!ui->pushButton_FullScreen->isChecked())
        parent->showNormal();
    else
        parent->showMaximized();
}

