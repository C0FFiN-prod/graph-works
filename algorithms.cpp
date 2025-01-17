#include <QBitArray>
#include "mainwindow.h"
#define INF 1.0 / 0.0
template <typename T>
QString printVector(const QList<T>& row){
    auto debug = qDebug();
    QString result = "";
    bool first = true;
    for(auto& cell : row){
        debug << cell << '\t';
        if(!first) result.append(',');
        else first = false;
        result.append(QVariant(cell).toString());
    }
    return result;
}

void printMatrix(const Matrix2D& matrix){
    qDebug() << "----------------------------";
    for(auto& row : matrix){
        printVector(row);
    }
    qDebug() << "----------------------------";
}

void highlightPath(Sequencer* sequencer, const QList<int>& path, const QString& color) {
    for (int i = 0; i < path.size() - 1; ++i) {
        int from = path[i];
        int to = path[i + 1];
        sequencer->addCommand(
            QString("SET_COLOR edge %1,%2 %3").arg(from).arg(to).arg(color));
    }
}

QPair<QSet<QPair<int, int>>, QSet<QPair<int, int>>> getPathSets(
    const QList<int>& oldPath,
    const QList<int>& newPath){
    QSet<QPair<int, int>> oldEdges;
    QSet<QPair<int, int>> newEdges;

    for (int i = 0; i < oldPath.size() - 1; ++i) {
        if(oldPath[i] == oldPath[i + 1])continue;
        oldEdges.insert({oldPath[i], oldPath[i + 1]});
    }
    for (int i = 0; i < newPath.size() - 1; ++i) {
        if(newPath[i] == newPath[i + 1])continue;
        newEdges.insert({newPath[i], newPath[i + 1]});
    }
    return qMakePair(oldEdges, newEdges);
}

bool highlightCommonEdges(Sequencer* sequencer,
                          QSet<QPair<int, int>>& oldEdges,
                          QSet<QPair<int, int>>& newEdges,
                          const QString &color = "ORANGE") {

    for (const auto& edge : oldEdges.intersect(newEdges)) {
        sequencer->addCommand(
            QString("SET_COLOR edge %1,%2 %3").arg(edge.first).arg(edge.second).arg(color));
    }
    return newEdges == oldEdges;
}




void MainWindow::algorithmFloYdWarshall()
{
    int k, i, j, n = graph.getAmount();

    if (n < 2) {
        QMessageBox::warning(this, this->title, "Too few nodes");
        return;
    }

    Matrix2D d = graph.getMatrixAdjacent();
    Matrix2I paths(n, QList<int>(n));

    QMap<QPair<int, int>, QList<int>> shortestPaths;
    sequencer.clear();
    // Инициализация матриц
    for (i = n; i--;)
        for (j = n; j--;) {
            if (d[i][j] < 0) {
                QMessageBox::warning(this,
                                     this->title,
                                     "For this algorithm graph must not contain negative weights");
                return;
            }

            if (d[i][j] || (i == j)) {
                paths[i][j] = j;
                shortestPaths[{i, j}] = {i, j};
            } else if (i != j) {
                d[i][j] = INF;
                shortestPaths[{i, j}] = {};
            } else {
                d[i][j] = 0;
                shortestPaths[{i, j}] = {i};
            }
        }

    // Основной цикл алгоритма
    for (k = 0; k < n; ++k) {
        for (i = 0; i < n; ++i) {
            for (j = 0; j < n; ++j) {
                if(i == j) continue;
                QList<int> oldPath = shortestPaths[{i, j}];       // Старый путь
                QList<int> newPath = shortestPaths[{i, k}] + shortestPaths[{k, j}].mid(1); // Новый путь
                sequencer.addFrame();
                sequencer.addCommand(QString("RESET_COLORS"));
                sequencer.addCommand(QString("SET_COLOR node %1 RED").arg(k));
                sequencer.addCommand(QString("SET_COLOR node %1 ORANGE").arg(i));
                sequencer.addCommand(QString("SET_COLOR node %1 BLUE").arg(j));
                sequencer.addCommand(
                    QString("SET_TEXT point 0,-250 Checking paths\nFrom %1 to %2 via %3\n")
                        .arg(i).arg(j).arg(k)+
                    QString("Old shortest path from %1 to %2: %3\n")
                        .arg(i).arg(j).arg(printVector(oldPath))+
                    QString("New possible path via %1: %2\n")
                        .arg(k).arg(printVector(newPath))+
                    QString("Comparing lengths: Old = %1, New = %2")
                        .arg(d[i][j]).arg(d[i][k] + d[k][j]));


                auto[oldEdges, newEdges] = getPathSets(oldPath, newPath);

                // Подсветка путей
                if(!oldEdges.empty() || !newEdges.empty()){
                    sequencer.addFrame();
                    highlightPath(&sequencer, oldPath, "YELLOW"); // Подсветить старый путь
                    highlightPath(&sequencer, newPath, "RED");   // Подсветить новый путь
                    highlightCommonEdges(&sequencer, oldEdges, newEdges); // Общие ребра (оранжевый)
                }
                if(oldEdges != newEdges){
                    // Проверяем и обновляем путь
                    if (d[i][k] < INF && d[k][j] < INF) {
                        sequencer.addFrame();
                        if (d[i][j] > d[i][k] + d[k][j]) {
                            d[i][j] = d[i][k] + d[k][j];
                            paths[i][j] = k;
                            shortestPaths[{i, j}] = newPath;

                            // Подсветка лучшего пути
                            highlightPath(&sequencer, oldPath, "DEFAULT");
                            highlightPath(&sequencer, newPath, "GREEN");
                            sequencer.addCommand(
                                QString("SET_TEXT point 0,-250 Path updated: New shortest path from %1 to %2 = %3")
                                    .arg(i)
                                    .arg(j)
                                    .arg(d[i][j]));
                        } else {
                            highlightPath(&sequencer, newPath, "DEFAULT");
                            highlightPath(&sequencer, oldPath, "GREEN");
                            sequencer.addCommand(
                                QString("SET_TEXT point 0,-250 No changes: Current shortest path from %1 to %2 = %3")
                                        .arg(i)
                                        .arg(j)
                                        .arg(d[i][j]));
                        }
                    }
                } else if(!oldEdges.empty()) {
                    sequencer.addFrame();
                    highlightPath(&sequencer, oldPath, "GREEN");
                    sequencer.addCommand(
                        QString("SET_TEXT point 0,-250 No changes: Current shortest path from %1 to %2 = %3")
                            .arg(i)
                            .arg(j)
                            .arg(d[i][j]));
                }
            }
        }
    }

    // Завершающий кадр
    sequencer.addFrame();
    sequencer.addCommand("RESET_COLORS");
    sequencer.addCommand("REMOVE_TYPE_TEXT point");
    initSequencer(true);
    QList<QWidget *> widgets{new QLabel("Shortest distances"),
                             makeTableFromMatrix(d, n, n, false),
                             new QLabel("Shortest paths"),
                             makeTableFromMatrix(paths, n, n, false)};
    QString title("Floyd Warshall " + QDateTime().currentDateTime().toString());
    addDockWidget(widgets, title);
}

QList<int> reconstructPath(int src, int target, const QList<int> &previous) {
    QList<int> path;
    for (int at = target; at != -1; at = previous[at]) {
        path.prepend(at); // Добавляем вершину в начало списка
    }
    if (path[0] != src) {
        return {}; // Путь недостижим
    }
    return path;
}

void markVisited(Sequencer& sequencer, const QBitArray& visited, const QString& color){
    for(int i = 0; i < visited.size(); ++i){
        if(visited[i])
            sequencer.addCommand(QString("SET_COLOR node %1 %2").arg(i).arg(color));
    }
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
    int n = graph.getAmount();
    if (n < 2) {
        QMessageBox::warning(this, this->title, "Too few nodes");
        return;
    }

    Matrix2D d = graph.getMatrixAdjacent();
    QList<double> dist(n, INF); // Список кратчайших расстояний
    QBitArray sptSet(n, false); // Множество обработанных вершин
    QList<int> previous(n, -1); // Хранение предшественников для восстановления пути

    int src = graph.getSourceIndex();
    if (src == -1) src = 0;

    dist[src] = 0;

    sequencer.clear();
    // Find shortest path for all vertices
    for (int i = 0; i < n - 1; i++) {
        // Pick the minimum distance vertex from the set of
        // vertices not yet processed. u is always equal to
        // src in the first iteration.
        int u = minDistance(n, dist, sptSet);
        sptSet[u] = true;
        // Анимация выбора текущей вершины
        sequencer.addFrame();
        sequencer.addCommand(QString("RESET_COLORS"));
        markVisited(sequencer, sptSet, "GRAY");
        sequencer.addCommand(QString("SET_COLOR node %1 ORANGE").arg(src));
        sequencer.addCommand(QString("SET_COLOR node %1 RED").arg(u));
        sequencer.addCommand(QString("SET_TEXT point 0,-250 Processing node %1").arg(graph.getNodeName(u)));
        highlightPath(&sequencer, reconstructPath(src, u, previous), "ORANGE");
        for(int k = 0; k < n; ++k){
            if (!sptSet[k] && d[u][k] && dist[u] != INF)
                sequencer.addCommand(QString("SET_COLOR edge %1,%2 RED").arg(u).arg(k));
        }

        // Mark the picked vertex as processed
        for (int k = 0; k < n; k++) {
            if (!sptSet[k] && d[u][k] && dist[u] != INF) {
                // Анимация обновления расстояний
                sequencer.addFrame();
                sequencer.addCommand(QString("RESET_COLORS"));
                markVisited(sequencer, sptSet, "GRAY");
                sequencer.addCommand(QString("SET_COLOR node %1 RED").arg(u));
                sequencer.addCommand(QString("SET_COLOR node %1 BLUE").arg(k));
                sequencer.addCommand(QString("SET_COLOR node %1 ORANGE").arg(src));
                highlightPath(&sequencer, reconstructPath(src, u, previous), "ORANGE");
                sequencer.addCommand(QString("SET_COLOR edge %1,%2 RED").arg(u).arg(k));
                if(dist[u] + d[u][k] < dist[k]){
                    sequencer.addCommand(
                        QString("SET_TEXT point 0,-250 Updating distance to %1: %2 -> %3")
                            .arg(graph.getNodeName(k))
                            .arg(dist[k])
                            .arg(dist[u] + d[u][k]));


                    dist[k] = dist[u] + d[u][k];
                    previous[k] = u; // Обновляем предшественника
                } else {
                    sequencer.addCommand(
                        QString("SET_TEXT point 0,-250 Not updating distance to %1: %2")
                            .arg(graph.getNodeName(k))
                            .arg(dist[k]));
                }
                highlightPath(&sequencer, reconstructPath(src, k, previous), "GREEN");
            }
        }
    }

    // Завершающий кадр
    sequencer.addFrame();
    sequencer.addCommand("RESET_COLORS");
    sequencer.addCommand("REMOVE_TYPE_TEXT point");
    initSequencer(true);

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
        QMessageBox::warning(this, this->title, "Too few nodes");
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
    sequencer.clear();

    while (MaxFlowDinicBFS(s, t, d, f, c, n)) {
        std::fill(ptr.begin(), ptr.end(), 0);

        sequencer.addFrame();
        sequencer.addCommand("RESET_COLORS");
        sequencer.addCommand("REMOVE_ALL_TEXT");
        sequencer.addCommand("SET_COLOR node " + QString::number(s) + " GREEN");
        sequencer.addCommand("SET_COLOR node " + QString::number(t) + " RED");
        sequencer.addCommand("SET_TEXT point 0,-250 Building level graph");

        for (int i = 0; i < n; ++i) {
            if (d[i] != -1) {
                sequencer.addFrame();
                sequencer.addCommand("REMOVE_ALL_TEXT");
                sequencer.addCommand("SET_COLOR node " + QString::number(i) + " BLUE");
                sequencer.addCommand("SET_TEXT point 0,-250 Level of node " + QString::number(i)
                                     + ": " + QString::number(d[i]) + "");
            }
        }

        double pushed;
        do {
            pushed = MaxFlowDinicDFS(s, s, t, c, f, d, ptr, INF, n);

            if (pushed > 0) {
                maxFlow += pushed;
            }
            sequencer.addFrame();
            sequencer.addCommand("REMOVE_TYPE_TEXT point");
            sequencer.addCommand("SET_COLOR node " + QString::number(s) + " GREEN");
            sequencer.addCommand("SET_COLOR node " + QString::number(t) + " RED");
            sequencer.addCommand(
                QString("SET_TEXT point 0,-250 DFS: Attempting to push flow = %1").arg(pushed));

            for (int v = 0; v < n; ++v) {
                if (ptr[v] > 0) {
                    sequencer.addFrame();
                    sequencer.addCommand("REMOVE_TYPE_TEXT point");
                    sequencer.addCommand("SET_COLOR node " + QString::number(v) + " ORANGE");
                    sequencer.addCommand("SET_TEXT point 0,-250  Pointer at node "
                                         + QString::number(v) + ": " + QString::number(ptr[v]) + "");
                }
            }

            if (pushed > 0) {
                for (int v = 0; v < n; ++v) {
                    for (int to = 0; to < n; ++to) {
                        if (f[v][to] > 0) {
                            sequencer.addFrame();
                            sequencer.addCommand("SET_COLOR edge " + QString::number(v) + ","
                                                 + QString::number(to) + " YELLOW");
                            sequencer.addCommand(QString("SET_TEXT edge %1,%2 Flow: %3/%4")
                                                     .arg(v)
                                                     .arg(to)
                                                     .arg(f[v][to])
                                                     .arg(c[v][to]));
                        }
                    }
                }
            }
        } while (pushed != 0);
    }

    // Кадр: Завершение алгоритма
    sequencer.addFrame();
    sequencer.addCommand("RESET_COLORS");
    sequencer.addCommand("REMOVE_ALL_TEXT");
    sequencer.addCommand(
        "SET_TEXT point 0,-250 Algorithm completed: Max Flow = " + QString::number(maxFlow) + "");

    QString title("Dinic " + QDateTime::currentDateTime().toString());
    consoleLog(title);
    consoleLog("Max Flow = " + QString::number(maxFlow));
    addDockWidget({new QLabel("Optimized flow matrix"), makeTableFromMatrix(f, n, n, false)}, title);

    // Инициализация анимации
    initSequencer(true);
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
        QMessageBox::warning(this, this->title, "Too few nodes");
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

#include "simplex_solver.cpp"




void MainWindow::algorithmNetTransportProblem()
{
    int n = graph.getAmount();

    if (n < 2) {
        QMessageBox::warning(this, this->title, "Too few nodes");
        return;
    }

    Matrix2D bandwidths = graph.getMatrixBandwidth();
    Matrix2D weights = graph.getMatrixAdjacent();

    QList<QPair<int, int>> edges;
    QList<double> B, C;
    int absSum = 0;
    int sum = 0;
    for (int i = 0; i < n; ++i) {
        sum += bandwidths[i][i];
        absSum += abs(bandwidths[i][i]);
        for (int j = 0; j < n; ++j) {
            if (i != j && bandwidths[i][j] != 0) {
                edges.emplace_back(i, j);
                B.push_back(bandwidths[i][j]);
                C.push_back(weights[i][j]);
            }
        }
    }

    int hasVirtualNode = sum == 0 ? 0 : 1;
    if(hasVirtualNode) {
        for(int i = 0; i < n; ++i){
            if((sum > 0 && bandwidths[i][i] < 0)
                || (sum < 0 && bandwidths[i][i] > 0)){
                edges.emplace_back(n, i);
                B.push_back(absSum*absSum);
                C.push_back(0);
            }
        }
    }

    for (int i = 0; i < n; ++i) {
        B.push_back(bandwidths[i][i]);
    }

    if(hasVirtualNode) B.push_back(sum);

    //printVector(B);
    n = C.size();
    Matrix2D A(B.size(), QList<double>(n, 0));

    for (int i = 0; i < n; ++i) {
        A[i][i] = 1;
        A[n + edges[i].first][i] = 1;
        A[n + edges[i].second][i] = -1;
    }
    //printMatrix(A);

    int n_slack = n;
    int n_art = A.size() - n;
    C.insert(C.constEnd(), n_slack, 0);
    C.insert(C.constEnd(), n_art, SIMPLEX_M);
    //printVector(C);
    Matrix2D tableau(A.size(), QList<double>(n + n_art + n_slack + 1, 0));
    for (int i = 0; i < n; ++i) {
        tableau[i][i] = 1;
        tableau[i][n + i] = 1;
        tableau[i].back() = B[i];
    }
    for (int i = 0; i < n_art; ++i) {
        tableau[n + i][n + n_slack + i] = 1;
        int sign = B[n + i] < 0 ? -1 : 1;
        tableau[n + i].back() = B[n + i] * sign;
        for (int j = 0; j < n; ++j) {
            tableau[n + i][j] = A[n + i][j] * sign;
        }
    }
    printMatrix(tableau);
    try {
        auto [results, f_opt] = simplex(tableau, C, n);
        printMatrix(tableau);

        n = bandwidths.size() + hasVirtualNode;
        Matrix2D flows(n, QList<double>(n, 0));
        for (int i = results.size(); i--;) {
            auto [j, k] = edges[i];
            flows[j][k] = results[i];
        }
        if(hasVirtualNode) flows[n-1][n-1] = sum;
        QString title = "Net Transport Problem " + QDateTime::currentDateTime().toString();
        QString message = QString("Minimal cost = %1\n").arg(f_opt) + (hasVirtualNode ? QString(sum > 0 ?"Leftovers = %1" : "Shortages = %1").arg(abs(sum)) : "");
        addDockWidget({new QLabel(message),
                       new QLabel("Optimized flows"),
                       makeTableFromMatrix(flows, n, n, false)},
                      title);
        consoleLog(title);
        consoleLog(message);

    } catch (std::runtime_error &e) {
        QMessageBox::warning(this, title, e.what());
    }
}
