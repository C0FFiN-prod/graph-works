#include "mainwindow.h"

#define INF 1.0 / 0.0
void MainWindow::algorithmFloYdWarshall()
{
    int k, i, j, n = graph.getAmount();

    if (n < 2) {
        QMessageBox::warning(this, this->windowTitle(), "Too few nodes");
        return;
    }

    Matrix2D d = graph.getMatrixAdjacent();
    Matrix2I paths(n, QList<int>(n));
    for (i = n; i--;)
        for (j = n; j--;) {
            if (d[i][j] || (i == j))
                paths[i][j] = j;
            else if (i != j)
                d[i][j] = INF;
            else
                d[i][j] = 0;
        }

    for (k = 0; k < n; ++k)
        for (i = 0; i < n; ++i)
            for (j = 0; j < n; ++j)
                if (d[i][k] < INF && d[k][j] < INF)
                    if (d[i][j] > d[i][k] + d[k][j]) {
                        d[i][j] = d[i][k] + d[k][j];
                        paths[i][j] = k;
                    }

    QList<QWidget *> widgets{new QLabel("Shortest distances"),
                             makeTableFromMatrix(d, n, n, false),
                             new QLabel("Shortest paths"),
                             makeTableFromMatrix(paths, n, n, false)};
    QString title = "Floyd Warshall " + QDateTime().currentDateTime().toString(Qt::RFC2822Date);
    addDockWidget(widgets, title);
}
