#ifndef EDGE_H
#define EDGE_H
#include "graphenums.h"
#include <QGraphicsItem>
#define txtOffset 15
#define curvines 25

class Node;

class Edge : public QGraphicsItem
{
public:
    Edge(Node *sourceNode, Node *destNode, double weight, EdgeType edgeType);
    Edge(Node *sourceNode, Node *destNode, double weight,double flow, EdgeType edgeType);

    Node *sourceNode() const;
    Node *destNode() const;
     //setters
    void setWeight(double weight);
    void setFlow(double flow);
    void setBandwidth(double bandwidth);
    void setEdgeType(EdgeType type);
     //getters
    double getWeight() const;
    double getFlow() const;
    double getBandwidth() const;
    EdgeType getEdgeType();

    void adjust();

    enum { Type = UserType + 2 };
    int type() const override { return Type; }
    ~Edge();
    static void setFlagsPtr(QFlags<GraphFlags> *flagsPtr);
protected:
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

private:
    static QFlags<GraphFlags> *graphFlags;
    EdgeType edgeType;
    Node *source, *dest;
    double flow;
    double weight;
    double bandwidth;
    QPointF sourcePoint;
    QPointF destPoint;
    qreal arrowSize = 10;
};
#endif // EDGE_H
