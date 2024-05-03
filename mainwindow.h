#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "qpushbutton.h"
#include <QMap>
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

private slots:

    void on_buttonClearConsole_clicked();

    void on_actionUnpin_current_tab_triggered();


    void on_buttonPin_toggled(bool checked);
    void on_viewMode_checked(bool checked);


private:
    Ui::MainWindow *ui;
    QMap<QWidget *, QPushButton *> pins;
    QMap<QString, QWidget *> tabs;
    void unpinTab(int index);
    void pinTab();

};
#endif // MAINWINDOW_H
