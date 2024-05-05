#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "qactiongroup.h"
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

    void buttonClearConsoleClicked();

    void on_actionUnpin_current_tab_triggered();


    void buttonPinToggled(bool checked);
    void viewModeChecked(bool checked);

private:
    Ui::MainWindow *ui;
    QMap<QWidget *, QPushButton *> pins;
    QMap<QString, QWidget *> tabs;
    void unpinTab(int index);
    void pinTab();
    QActionGroup *nodeMovementGroup;
};
#endif // MAINWINDOW_H
