#ifndef GRAPH_H
#define GRAPH_H
#include <QException>
#include "edge.h"
#include "graphwidget.h"
#include "node.h"
#include "qbitarray.h"
#include "qmap.h"

typedef QList<QList<double>> Matrix2D;
typedef QList<QList<int>> Matrix2I;

class Graph
{
public:
    Graph();
    Graph(unsigned int size);
    Graph(Matrix2D &matrix);
    ~Graph();
    const Matrix2D getMatrixAdjacent(bool full = false);
    const Matrix2D getMatrixFlow(bool full = false);
    const Matrix2D getMatrixBandwidth(bool full = false);
    const QList<QVariantList> getListEdges(bool full = false);
    void setMatrixAdjacent(Matrix2D &matrix);
    void setMatrixFlow(Matrix2D &matrix);
    void setMatrixBandwidth(Matrix2D &matrix);

    void setEdge(unsigned int u, unsigned int v, double w);

    void setEdgeFlow(unsigned int u, unsigned int v, double f);
    void setEdgeWeight(unsigned int u, unsigned int v, double w);
    void setEdgeBandwidth(unsigned int u, unsigned int v, double b);
    QString getNodeName(unsigned int u);
    Edge* getEdge(unsigned int u, unsigned int v);
    void removeEdge(unsigned int u, unsigned int v);
    void removeNode(unsigned int index);
    void addNode(unsigned int i, const QString &name);
    void updateNodes();

    bool edgeExists(unsigned int u, unsigned int v);
    const QFlags<GraphFlags> getFlags();
    void setFlag(GraphFlags flag, bool set = true);
    void setFlags(QFlags<GraphFlags> flags);
    void unsetFlag(GraphFlags flag);
    void toggleFlag(GraphFlags flag);
    void toggleNode(Node *node, bool enabled);
    void toggleNode(unsigned int index, bool enabled);
    void toggleEdge(Edge *edge, bool enabled);
    void toggleEdge(unsigned int u, unsigned int v, bool enabled);
    void toggleEdges(const QList<Edge *> &edgeList, bool enabled);
    bool isEdgeEnabled(Edge *edge);
    bool isNodeEnabled(Node *node);
    void toggleNodes(const QBitArray &mask);
    QBitArray getEnablingMask();
    GraphWidget *graphView;
    unsigned int getAmount();

    int getSourceIndex();
    int getDestIndex();

    void setSourceIndex(unsigned int sourceIndex);
    void setDestIndex(unsigned int destIndex);
    bool isUnsaved();
    void changesSaved();
    void clear();

private:
    bool unsavedChanges = false;
    void resizeGraph(unsigned int oldAmount, unsigned int newAmount);
    QFlags<GraphFlags> flags;
    EdgeType getEdgeType(int i, int j, Matrix2D &matrix);
    unsigned int amount;
    QBitArray enablingMask;
    QSet<Edge *> disabledEdges;
    QMap<QPair<Node*, Node*>, Edge*> edges;
    QMap<unsigned int, Node*> nodes;

    //QMap<QPair<unsigned int, unsigned int>, EdgeStruct> edges;
    //QMap<unsigned int, Node> nodes;

    Node *src = nullptr;
    Node *dst = nullptr;

    Matrix2D adjacent;
    Matrix2D flow;
    Matrix2D bandwidth;
    Matrix2D makeEnabledMatrix(const Matrix2D &matrix);
};
#endif // GRAPH_H
