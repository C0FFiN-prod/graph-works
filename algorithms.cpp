#include <QBitArray>
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
    QString title = "Floyd Warshall " + QDateTime().currentDateTime().toString();
    addDockWidget(widgets, title);
}
void MainWindow::algorithmDijkstra()
{
    int k, i, n = graph.getAmount();
    if (n < 2) {
        QMessageBox::warning(this, this->windowTitle(), "Too few nodes");
        return;
    }

    Matrix2D d = graph.getMatrixAdjacent();
    QList<double> dist(n); // The output array.  dist[i] will hold the
        // shortest
    // distance from src to i
    QBitArray sptSet(n); // sptSet[i] will be true if vertex i is
        // included in shortest
    // path tree or shortest distance from src to i is
    // finalized

    auto minDistance = [](int n, QList<double> dist, QBitArray sptSet) {
        // Initialize min value
        double min = INF, min_index;

        for (int v = 0; v < n; v++)
            if (sptSet[v] == false && dist[v] <= min)
                min = dist[v], min_index = v;

        return min_index;
    };

    // Initialize all distances as INFINITE and stpSet[] as
    // false
    for (i = 0; i < n; i++)
        dist[i] = INF, sptSet[i] = false;
    int src;
    if ((src = graph.getSourceIndex()) == -1)
        src = 0;

    // Distance of source vertex from itself is always 0
    dist[src] = 0;

    // Find shortest path for all vertices
    for (i = 0; i < n - 1; i++) {
        // Pick the minimum distance vertex from the set of
        // vertices not yet processed. u is always equal to
        // src in the first iteration.
        double u = minDistance(n, dist, sptSet);

        // Mark the picked vertex as processed
        sptSet[u] = true;

        // Update dist value of the adjacent vertices of the
        // picked vertex.
        for (k = 0; k < n; k++)

            // Update dist[v] only if is not in sptSet,
            // there is an edge from u to v, and total
            // weight of path from src to  v through u is
            // smaller than current value of dist[v]
            if (!sptSet[k] && d[u][k] && dist[u] != INT_MAX && dist[u] + d[u][k] < dist[k])
                dist[k] = dist[u] + d[u][k];
    }
    Matrix2D distM(1, dist);
    QString title = "Dijkstra " + QDateTime::currentDateTime().toString();
    QList<QWidget *> widgets = {
        new QLabel("Shortest distances from node " + graph.getNodeName(src)),
        makeTableFromMatrix(distM, 1, n, false),
    };
    addDockWidget(widgets, title);
}
