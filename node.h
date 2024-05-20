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
    Node(const Node& node);
    void addEdge(Edge *edge);
    QSet<Edge *> edges() const;

    enum { Type = UserType + 1 };
    int type() const override { return Type; }

    void calculateForces(bool manual);
    bool advancePosition();

    void connectToNode(Node* node);
    void disconnectFromNode(Node* node);
    QSet<Node*> getChilden();
    QSet<Node*> getParents();

    QRectF boundingRect() const override;
    QPainterPath shape() const override;

    int getIndex() const;
    void setIndex(unsigned int i);

    QString getDisplayName() const;
    void setDisplayName(const QString &name);

    Node& operator=(const Node&);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *widget) override;
    void extracted();
    ~Node();
    QSet<Edge *> edgeList;

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

private:
    QSet<Node *> children;
    QSet<Node *> parents;

    QPointF newPos;
    GraphWidget *graph;
    int index;
    QString displayName;
};

#endif // NODE_H
