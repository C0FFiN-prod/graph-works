#ifndef NODE_H
#define NODE_H

#include <QGraphicsItem>
#include <QList>

#define nodeSize 35
#define strokeWidth 1
class Edge;
class GraphWidget;

class Node : public QGraphicsItem
{
public:
    Node(int index, GraphWidget *graphWidget);
    Node();

    void addEdge(Edge *edge);
    QList<Edge *> edges() const;

    enum { Type = UserType + 1 };
    int type() const override { return Type; }

    void calculateForces();
    bool advancePosition();

    void connectToNode(Node* node);
    void disconnectFromNode(Node* node);
    QSet<Node*> getChilden();
    QSet<Node*> getParents();

    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    int getIndex();
    void setIndex(unsigned int);

    Node& operator=(const Node&);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void extracted();
    ~Node();

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

private:
    QList<Edge *> edgeList;
    QSet<Node *> children;
    QSet<Node *> parents;

    QPointF newPos;
    GraphWidget *graph;
    int index;
};

#endif // NODE_H
