
#include "edge.h"
#include "node.h"
#include "graphwidget.h"

#include <QGraphicsScene>
#include <QRandomGenerator>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOption>

Node::Node(int index, GraphWidget *graphWidget)
    : graph(graphWidget)
{
    this->index = index;
    setFlag(ItemIsMovable);
    setFlag(ItemSendsGeometryChanges);
    setCacheMode(DeviceCoordinateCache);
    setZValue(-1);
    this->setPos(QRandomGenerator::global()->bounded(300), QRandomGenerator::global()->bounded(300));
}

Node::Node():graph(nullptr), index(0)
{}


void Node::addEdge(Edge *edge)
{
    edgeList << edge;
    edge->adjust();
}

QSet<Edge *> Node::edges() const
{
    return edgeList;
}

void Node::calculateForces()
{
    if (!scene() || scene()->mouseGrabberItem() == this) {
        newPos = pos();
        return;
    }

    // Sum up all forces pushing this item away
    qreal xvel = 0;
    qreal yvel = 0;
    const QList<QGraphicsItem *> items = scene()->items();
    for (QGraphicsItem *item : items) {
        Node *node = qgraphicsitem_cast<Node *>(item);
        if (!node)
            continue;

        QPointF vec = mapToItem(node, 0, 0);
        qreal dx = vec.x();
        qreal dy = vec.y();
        double l = 2.0 * (dx * dx + dy * dy);
        if (l > 0) {
            xvel += (dx * 550.0) / l;
            yvel += (dy * 550.0) / l;
        }
    }

    // Now subtract all forces pulling items together
    double weight = (edgeList.size() + 1) * 50;
    for (const Edge *edge : std::as_const(edgeList)) {
        QPointF vec;
        if (edge->sourceNode() == this)
            vec = mapToItem(edge->destNode(), 0, 0);
        else
            vec = mapToItem(edge->sourceNode(), 0, 0);
        xvel -= vec.x() / weight;
        yvel -= vec.y() / weight;
    }

    if (qAbs(xvel) < 0.1 && qAbs(yvel) < 0.1)
        xvel = yvel = 0;

    QRectF sceneRect = scene()->sceneRect();
    newPos = pos() + QPointF(xvel, yvel);
    newPos.setX(qMin(qMax(newPos.x(), sceneRect.left() + 10), sceneRect.right() - 10));
    newPos.setY(qMin(qMax(newPos.y(), sceneRect.top() + 10), sceneRect.bottom() - 10));
}

bool Node::advancePosition()
{
    if (newPos == pos())
        return false;

    setPos(newPos);
    return true;
}

void Node::connectToNode(Node *node)
{
    if(node==nullptr){
        return;
    }
    this->children.insert(node);
    node->parents.insert(this);
}

void Node::disconnectFromNode(Node *node) {
    if (node == nullptr) {
        return;
    }
    if(node == this){

        if(children.find(this)!=children.end()){
            edgeList.removeIf([](Edge* i){
                return i->getEdgeType() == EdgeType::Loop;
            });
            this->children.remove(this);
            this->parents.remove(this);
        }
    }else {
        if(children.find(node)!=children.end()){
            edgeList.removeIf([&node](Edge* i){
                return i->destNode() == node;
            });
            node->edgeList.removeIf([this](Edge* i){
                return i->sourceNode() == this;
            });
            this->children.remove(node);
            node->parents.remove(this);
        }
    }
}

QSet<Node *> Node::getChilden()
{
    return this->children;
}

QSet<Node *> Node::getParents()
{
    return this->parents;
}

QRectF Node::boundingRect() const
{
    qreal adjust = 2;
    return QRectF( -nodeSize/2 - adjust, -nodeSize/2 - adjust, nodeSize+strokeWidth + adjust, nodeSize+strokeWidth + adjust);
}

QPainterPath Node::shape() const
{
    QPainterPath path;
    path.addEllipse(-nodeSize/2, -nodeSize/2, nodeSize, nodeSize);
    return path;
}

int Node::getIndex()
{
    return this->index;
}

void Node::setIndex(unsigned int num)
{
    this->index= num;
}


void Node::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *) {
    // drawing circle
    painter->setPen(QPen(Qt::black, 1));
    painter->drawEllipse(-nodeSize/2, -nodeSize/2, nodeSize, nodeSize);

    //font setting up
    QFont font = painter->font();
    font.setBold(true);
    font.setPointSize(14);
    painter->setFont(font);

    //get text boxing
    QString nodeText = QString::number(index);
    QFontMetrics metrics(font);
    QRect textRect = metrics.boundingRect(nodeText);

    //get coords of text
    QPointF textPnt(-(textRect.width())/2 -0.7 , textRect.height()/3. - 0.7);;

    //drawing text
    painter->setPen(Qt::black);
    painter->drawText(textPnt, nodeText);
}

Node::~Node()
{
    for(auto& child: std::as_const(children)){
        this->disconnectFromNode(child);
    }
    for(auto& parent: std::as_const(parents)){
        parent->disconnectFromNode(this);
    }
}

QVariant Node::itemChange(GraphicsItemChange change, const QVariant &value)
{
    switch (change) {
    case ItemPositionHasChanged:
        for (Edge *edge : edgeList)
            edge->adjust();
        graph->itemMoved();
        break;
    default:
        break;
    };

    return QGraphicsItem::itemChange(change, value);
}

void Node::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    update();
    QGraphicsItem::mousePressEvent(event);
}

void Node::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    update();
    QGraphicsItem::mouseReleaseEvent(event);
}
