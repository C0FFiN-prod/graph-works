#include <QBitArray>
#include "mainwindow.h"
#include "ui_mainwindow.h"
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
            if (d[i][j] < 0) {
                QMessageBox::warning(this,
                                     this->windowTitle(),
                                     "For this algorithm graph must not contain negative weights");
                return;
            }

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
    QString title("Floyd Warshall " + QDateTime().currentDateTime().toString());
    addDockWidget(widgets, title);
}

int minDistance(int n, QList<double> &dist, QBitArray &sptSet)
{
    // Initialize min value
    double min = INF;
    int min_index = -1;

    for (int v = 0; v < n; v++)
        if (sptSet[v] == false && dist[v] <= min)
            min = dist[v], min_index = v;

    return min_index;
};

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

    // Initialize all distances as INFINITE and stpSet[] as
    // false
    for (i = 0; i < n; i++)
        dist[i] = INF, sptSet[i] = false;
    int src, u;
    if ((src = graph.getSourceIndex()) == -1)
        src = 0;

    // Distance of source vertex from itself is always 0
    dist[src] = 0;

    // Find shortest path for all vertices
    for (i = 0; i < n - 1; i++) {
        // Pick the minimum distance vertex from the set of
        // vertices not yet processed. u is always equal to
        // src in the first iteration.
        u = minDistance(n, dist, sptSet);

        // Mark the picked vertex as processed
        sptSet[u] = true;

        // Update dist value of the adjacent vertices of the
        // picked vertex.
        for (k = 0; k < n; k++)

            // Update dist[v] only if is not in sptSet,
            // there is an edge from u to v, and total
            // weight of path from src to  v through u is
            // smaller than current value of dist[v]
            if (!sptSet[k] && d[u][k] && dist[u] != INF && dist[u] + d[u][k] < dist[k])
                dist[k] = dist[u] + d[u][k];
    }
    Matrix2D distM(1, dist);
    QString title("Dijkstra " + QDateTime::currentDateTime().toString());
    QList<QWidget *> widgets = {
        new QLabel("Shortest distances from node " + graph.getNodeName(src)),
        makeTableFromMatrix(distM, 1, n, false),
    };
    addDockWidget(widgets, title);
}

#include "graph.h" // Assuming this header contains the necessary class and method declarations
#include <algorithm>
#include <queue>

bool MaxFlowDinicBFS(int s, int t, QList<int> &d, Matrix2D &f, const Matrix2D &c, int n)
{
    std::queue<int> q;
    q.push(s);

    std::fill(d.begin(), d.end(), -1);
    d[s] = 0;

    while (!q.empty()) {
        int v = q.front();
        q.pop();

        for (int to = 0; to < n; ++to) {
            if (d[to] == -1 && f[v][to] < c[v][to]) {
                q.push(to);
                d[to] = d[v] + 1;
            }
        }
    }

    return d[t] != -1;
}

double MaxFlowDinicDFS(int v,
                       int s,
                       int t,
                       const Matrix2D &c,
                       Matrix2D &f,
                       QList<int> &d,
                       QList<int> &ptr,
                       double flow,
                       int n)
{
    if (flow == 0)
        return 0;

    if (v == t)
        return flow;

    for (; ptr[v] < n; ++ptr[v]) {
        int to = ptr[v];

        if (d[to] != d[v] + 1)
            continue;

        double pushed = MaxFlowDinicDFS(to, s, t, c, f, d, ptr, fmin(flow, c[v][to] - f[v][to]), n);

        if (pushed != 0) {
            f[v][to] += pushed;
            f[to][v] -= pushed;
            return pushed;
        }
    }

    return 0;
}

void MainWindow::algorithmDinic()
{
    int n = graph.getAmount();
    if (n < 2) {
        QMessageBox::warning(this, this->windowTitle(), "Too few nodes");
        return;
    }

    int s, t;
    if ((s = graph.getSourceIndex()) == -1)
        s = 0;
    if ((t = graph.getDestIndex()) == -1)
        t = n - 1;

    Matrix2D c = graph.getMatrixBandwidth();
    Matrix2D f(n, QList<double>(n, 0));
    QList<int> d(n, -1);
    QList<int> ptr(n, 0);

    double maxFlow = 0;

    while (MaxFlowDinicBFS(s, t, d, f, c, n)) {
        std::fill(ptr.begin(), ptr.end(), 0);

        double pushed;
        do {
            pushed = MaxFlowDinicDFS(s, s, t, c, f, d, ptr, INF, n);
            maxFlow += pushed;
        } while (pushed != 0);
    }
    QString title("Dinic " + QDateTime::currentDateTime().toString());
    consoleLog(title);
    consoleLog("Max Flow = " + QString::number(maxFlow));
    addDockWidget({new QLabel("Optimized flow matrix"), makeTableFromMatrix(f, n, n, false)}, title);
}

// The main function that finds shortest distances from src to
// all other vertices using Bellman-Ford algorithm. The function
// also detects negative weight cycle
void MainWindow::algorithmBellmanFord()
{
    // Step 1: Initialize distances from src to all other vertices
    // as INFINITE
    int src, n = graph.getAmount();
    if (n < 2) {
        QMessageBox::warning(this, this->windowTitle(), "Too few nodes");
        return;
    }
    if ((src = graph.getSourceIndex()) == -1)
        src = 0;
    QList<double> dist(n, INF);
    auto edges(graph.getListEdges());
    dist[src] = 0;
    int i, u, v;
    double w;
    // Step 2: Relax all edges |V| - 1 times. A simple shortest
    // path from src to any other vertex can have at-most |V| - 1
    // edges
    for (i = 0; i < n - 1; i++)
        // Update dist value and parent index of the adjacent vertices of
        // the picked vertex. Consider only those vertices which are still in
        // queue
        for (auto &edge : edges) {
            u = edge[0].toInt();
            v = edge[1].toInt();
            w = edge[2].toDouble();
            if ((dist[u] != INF) && (dist[u] + w < dist[v]))
                dist[v] = dist[u] + w;
        }

    // Step 3: check for negative-weight cycles. The above step
    // guarantees shortest distances if graph doesn't contain
    // negative weight cycle. If we get a shorter path, then there
    // is a cycle.

    QString title = "Bellmann Ford " + QDateTime::currentDateTime().toString();

    for (auto &edge : edges) {
        u = edge[0].toInt();
        v = edge[1].toInt();
        w = edge[2].toDouble();
        if ((dist[u] != INF) && (dist[u] + w < dist[v])) {
            consoleLog(title);
            consoleLog("Graph contains negative weight cycle");
            return;
        }
    }
    addDockWidget({new QLabel("Shortest distances from node " + graph.getNodeName(src)),
                   makeTableFromMatrix(Matrix2D(1, dist), n, n, false)},
                  title);
}

