#include <QTime>
#include "mainwindow.h"
#include "qdockwidget.h"
#include "qheaderview.h"
#include "qlabel.h"
#include "qlayout.h"
#include "qmessagebox.h"
#define INF 1.0 / 0.0
void MainWindow::algorithmFloYdWarshall()
{
    int k, i, j, n = graph.getAmount();

    if (n < 2) {
        QMessageBox::warning(this, this->windowTitle(), "Too few nodes");
        return;
    }

    Matrix2D d = graph.getMatrixAdjacent();
    Matrix2I paths(n, QList<int>(n, n));
    for (i = n; i--;)
        for (j = n; j--;)
            if (!d[i][j] || i == j)
                d[i][j] = INF;

    for (k = 0; k < n; ++k)
        for (i = 0; i < n; ++i)
            for (j = 0; j < n; ++j)
                if (d[i][k] < INF && d[k][j] < INF)
                    if (d[i][j] > d[i][k] + d[k][j]) {
                        d[i][j] = d[i][k] + d[k][j];
                        paths[i][j] = k;
                    }

    QDockWidget *dock = new QDockWidget(this);
    auto container = new QWidget();
    auto layout = new QVBoxLayout(container);
    dock->setWidget(container);
    dock->setFloating(true);
    dock->setWindowTitle("Floyd Warshall "
                         + QDateTime().currentDateTime().toString(Qt::RFC2822Date));
    dock->setFeatures(QDockWidget::DockWidgetClosable);
    auto table = new QTableView();
    table->setModel(new QStandardItemModel(0, 0));
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->verticalHeader()->setDefaultSectionSize(20);
    table->horizontalHeader()->setDefaultSectionSize(40);
    setTableFromMatrix(table, d);
    layout->addWidget(new QLabel("Shortest distances"));
    layout->addWidget(table);
    table = new QTableView();
    table->setModel(new QStandardItemModel(0, 0));
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->verticalHeader()->setDefaultSectionSize(20);
    table->horizontalHeader()->setDefaultSectionSize(40);
    layout->addWidget(new QLabel("Shortest paths"));
    layout->addWidget(table);
    setTableFromMatrix(table, paths);
    dock->show();
}
