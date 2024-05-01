#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "titlebar.h"
#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    TitleBar* titlebar;
    bool eventFilter(QObject *obj, QEvent *event) override;
    QPointF m_dragPosition;
    bool leftButtonPressed = false;
};
#endif // MAINWINDOW_H
