#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMap>
#include <QTime>
#include "graph.h"
#include "qactiongroup.h"
#include "qboxlayout.h"
#include "qdockwidget.h"
#include "qheaderview.h"
#include "qlabel.h"
#include "qmessagebox.h"
#include "qpushbutton.h"
#include "qshortcut.h"
#include "qspinbox.h"
#include "qstandarditemmodel.h"
#include "qtableview.h"
#include "sequencer.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(const QString &title, QWidget *parent = nullptr);
    Graph graph;
    Sequencer sequencer;
    ~MainWindow();
private slots:
    void clearConsole();
    void viewModeChecked(bool checked);
    void myCopy();
    void myPaste();
    void setNodesAmountMatrix(QTableView *table, int newAmount);
    void listDataChanged(const QModelIndex &topLeft,
                         const QModelIndex &bottomRight,
                         const QVector<int> &);

private:
    Ui::MainWindow *ui;
    QString title;
    QLabel *currentFrameLabel;
    QSlider *delayBetweenFramesSlider;
    const static QRegularExpression reValidDouble, reValidDoubleLine, reValidInt;
    QMap<QString, QWidget *> docksViewMode;
    void pasteClipboardToTable(QTableView *dest);
    void handleSequencerFrameChange();
    void copyTableToClipboard(QTableView *src);
    void applyGraphMatrix(QTableView *table);
    void applyEdgesList(QTableView *table);
    void updateEdgesList(QTableView *list);
    void updateTables();
    void addRowToList(QStandardItemModel *table);
    void deleteSelectedObjects(const QFlags<DeleteOptions> &options);
    void addNode();
    void markSelectedAs(const QFlags<SelectOptions> &option);
    void clearGraph();
    template<typename T>
    void setTableFromMatrix(QTableView *table, const T &matrix, int height = -1, int width = -1);
    QList<QActionGroup *> actionGroups;
    QList<QTableView *> graphMatrixViews;
    QList<QTableView *> graphListViews;
    QMap<QString, QSpinBox *> graphCountSpins;
    void consoleLog(const QString &text);
    void addDockWidget(const QList<QWidget *> &widgets,
                       const QString &title,
                       bool closable = true,
                       bool floating = true);
    template<typename T>
    QTableView *makeTableFromMatrix(const T &matrix,
                                    int height = -1,
                                    int width = -1,
                                    bool editable = true,
                                    int headerVSize = 20,
                                    int headerHSize = 40);
    // Algorithms
    void algorithmFloYdWarshall();
    void algorithmDijkstra();
    void algorithmDinic();
    void algorithmBellmanFord();
    void algorithmSpanningTree(const QString &STType);

    void initSequencer(bool isSequenceStateless = false);

    // Saves
    QString currentFile = "";
    void updateFileStatus();
    void saveGraphToCSV(bool saveToNew);
    void readGraphFromCSV();
    void newGraph();
    bool checkForSave();
    void algorithmNetTransportProblem();
};
#endif // MAINWINDOW_H
