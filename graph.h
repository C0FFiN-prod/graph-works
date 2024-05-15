#ifndef GRAPH_H
#define GRAPH_H
#include "node.h"
#include "edge.h"
#include "graphwidget.h"
#include "qmap.h"
#include <QException>

typedef QList<QList<double>> Matrix2D;




class Graph
{
public:
    Graph();
    Graph(unsigned int size);
    Graph(Matrix2D &matrix);
    const Matrix2D getMatrixAdjacent();
    const Matrix2D getMatrixFlow();
    const Matrix2D getMatrixBandwidth();
    void setMatrixAdjacent(Matrix2D &matrix);
    void setMatrixFlow(Matrix2D &matrix);
    void setMatrixBandwidth(Matrix2D &matrix);

    void setEdge(Node* u, Node* v, Edge& edge);

    void setEdgeFlow(unsigned int u, unsigned int v, double flow);
    void setEdgeWeight(unsigned int u, unsigned int v, double flow);
    void removeEdge(unsigned int u, unsigned int v);
    void addNode(unsigned int i);

    const QFlags<GraphFlags> getFlags();
    void setFlag(GraphFlags flag);
    void setFlags(QFlags<GraphFlags> flags);
    void unsetFlag(GraphFlags flag);
    void toggleFlag(GraphFlags flag);
    //void removeNode(unsigned int i);
    bool unsavedChanges = false;
    GraphWidget *graphView;
private:
    void resizeGraph(unsigned int oldAmount, unsigned int newAmount);
    QFlags<GraphFlags> flags;
    EdgeType getEdgeType(int i, int j, Matrix2D &matrix);
    unsigned int amount;

    QMap<QPair<Node*, Node*>, Edge*> edges;
    QMap<unsigned int, Node*> nodes;

    //QMap<QPair<unsigned int, unsigned int>, EdgeStruct> edges;
    //QMap<unsigned int, Node> nodes;

    Matrix2D adjacent;
    Matrix2D flow;
    Matrix2D bandwidth;

};
#endif // GRAPH_H
