#ifndef GRAPH_H
#define GRAPH_H
#include "node.h"
#include "qmap.h"
#include <QException>

typedef QList<QList<double>> Matrix2D;

enum Orientation{
    Strait = 0,
    Reversed = 1,
    Both = 2,
};

struct Edge{
    double weight;
    double flow;
    Edge():
        weight(0),
        flow(0){};
    Edge(double weight,
         double flow):
        weight(weight),
        flow(flow){};
};
class Graph
{
public:
    Graph();
    Graph(unsigned int size);
    Graph(Matrix2D matrix);
    const Matrix2D getMatrixAdjacent();
    const Matrix2D getMatrixFlow();
    void setMatrixAdjacent(Matrix2D matrix);
    void setMatrixFlow(Matrix2D matrix);
    void setEdge(unsigned int u, unsigned int v, Edge& edge);
    void setEdgeFlow(unsigned int u, unsigned int v, double flow);
    void setEdgeWeight(unsigned int u, unsigned int v, double flow);
    void removeEdge(unsigned int u, unsigned int v);
    void addNode(unsigned int i, int x, int y);
    void removeNode(unsigned int i);
private:
    unsigned int amount;
    QMap<QPair<unsigned int, unsigned int>, Edge> edges;
    QMap<unsigned int, Node> nodes;
    Matrix2D adjacent;
    Matrix2D flow;
    void addEdge(unsigned int u, unsigned int v, Edge& edge);
};
#endif // GRAPH_H
