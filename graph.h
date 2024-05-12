#ifndef GRAPH_H
#define GRAPH_H
#include "node.h"
#include "edge.h"
#include "graphwidget.h"
#include "qmap.h"
#include <QException>

typedef QList<QList<double>> Matrix2D;

enum Orientation{
    Strait = 0,
    Reversed = 1,
    Both = 2,
};

struct EdgeStruct{
    double weight;
    double flow;
    EdgeStruct():
        weight(0),
        flow(0){};
    EdgeStruct(double weight,
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

    void setEdge(Node* u, Node* v, Edge& edge);

    void setEdgeFlow(unsigned int u, unsigned int v, double flow);
    void setEdgeWeight(unsigned int u, unsigned int v, double flow);
    void removeEdge(unsigned int u, unsigned int v);
    void addNode(unsigned int i);
    //void removeNode(unsigned int i);

    GraphWidget *graphView;
private:
    EdgeType getEdgeType(int i, int j, Matrix2D matrix);
    unsigned int amount;

    QMap<QPair<Node*, Node*>, Edge*> edges;
    QMap<unsigned int, Node*> nodes;

    //QMap<QPair<unsigned int, unsigned int>, EdgeStruct> edges;
    //QMap<unsigned int, Node> nodes;

    Matrix2D adjacent;
    Matrix2D flow;

};
#endif // GRAPH_H
