#ifndef GRAPH_H
#define GRAPH_H
#include "node.h"
#include "qmap.h"

typedef QList<QList<double>> Matrix2D;

enum Orientation{
    Strait = 0,
    Reversed = 1,
    Both = 2,
};

struct Edge{
    double weight;
    double bandwidth;
    double flow;
    Edge():
        weight(0),
        bandwidth(0),
        flow(0){};
    Edge(double weight,
         double bandwidth,
         double flow):
        weight(weight),
        bandwidth(bandwidth),
        flow(flow){};
};
class Graph
{
public:
    void addEdge(unsigned int, unsigned int, Edge edge);
    void removeEdge(unsigned int, unsigned int);
    Graph();
    Graph(unsigned int size);
    Graph(Matrix2D matrix);
    const Matrix2D getMatrixAdjacent();
    const Matrix2D getMatrixFlow();
    const Matrix2D getMatrixBandwidth();
private:
    unsigned int amount;
    QMap<QPair<unsigned int, unsigned int>, Edge> edges;
    QMap<unsigned int, Node> nodes;
    Matrix2D adjacent;
    Matrix2D flow;
    Matrix2D bandwidth;
};
#endif // GRAPH_H
