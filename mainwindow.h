#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "graph.h"
#include "qactiongroup.h"
#include "qpushbutton.h"
#include "qshortcut.h"
#include "qspinbox.h"
#include "qtableview.h"
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
    Graph graph;
    ~MainWindow();

private slots:
    void buttonClearConsoleClicked();
    void viewModeChecked(bool checked);
    void myCopy();
    void myPaste();
    void setNodesAmountMatrix(QTableView *table, int newAmount);
    void setNodesAmountList(QTableView *table, int newAmount);

private:
    Ui::MainWindow *ui;
    QMap<QWidget *, QPushButton *> pins;
    QMap<QString, QWidget *> docksViewMode;
    void unpinTab(int index);
    void pinTab();
    void pasteClipboardToTable(QTableView *dest);
    void copyTableToClipboard(QTableView *src);
    void applyNodesAmountMatrix(QTableView *table);
    void applyNodesAmountList(QTableView *table);
    void addRowToList(QTableView *table, const QSet<int>& importantCols);
    QActionGroup *nodeMovementGroup;
    QList<QTableView *> graphMatrixViews;
    QList<QTableView *> graphListViews;
    QMap<QString, QSpinBox *> graphCountSpins;
};
#endif // MAINWINDOW_H
