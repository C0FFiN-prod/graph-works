
#ifndef EDGE_H
#define EDGE_H

#include <QGraphicsItem>
#define txtOffset 15
#define curvines 25
enum EdgeType{
    BiDirectionalSame =0,
    BiDirectionalDiff = 1,
    SingleDirection = 2,
    Loop = 3,
    Error =-1
};
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
     //getters
    double getWeight();
    double getFlow();
    EdgeType getEdgeType();

    void adjust();

    enum { Type = UserType + 2 };
    int type() const override { return Type; }
    ~Edge();

protected:
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

private:
    EdgeType edgeType;
    Node *source, *dest;
    double flow;
    double weight;
    QPointF sourcePoint;
    QPointF destPoint;
    qreal arrowSize = 10;
};

#endif // EDGE_H
