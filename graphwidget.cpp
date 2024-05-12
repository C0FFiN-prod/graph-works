#include "graphwidget.h"


#include <math.h>
#include <QKeyEvent>
#include <QRandomGenerator>

GraphWidget::GraphWidget( QMap<QPair<Node*, Node*>, Edge*>* edges,QMap<unsigned int, Node*>* nodes, QWidget *parent)
    : QGraphicsView(parent), scenePtr(nullptr), edges(edges), nodes(nodes)
{
    this->amount = nodes->size();
    scenePtr = new QGraphicsScene(this);
    scenePtr->setItemIndexMethod(QGraphicsScene::NoIndex);
    scenePtr->setSceneRect(-500, -300, 1000, 600);
    setScene(scenePtr);

    setCacheMode(CacheBackground);
    setViewportUpdateMode(BoundingRectViewportUpdate);
    setRenderHint(QPainter::Antialiasing);
    setTransformationAnchor(AnchorUnderMouse);
    scale(qreal(1), qreal(1));
    setMinimumSize(300, 00);

}

void GraphWidget::itemMoved()
{
    if (!timerId)
        timerId = startTimer(1000 / 25);
}

void GraphWidget::initScene()
{
    scene()->clear();
    QMap<int, Node*> n;
    QList<Edge*> e(edges->size());
    for(auto [i, node] : nodes->asKeyValueRange()){
        n[i] = new Node(*node);
    }
    int i = 0;
    for(auto& edge : *edges){
        int u = edge->sourceNode()->getIndex();
        int v = edge->destNode()->getIndex();
        e[i++] = new Edge(n[u],n[v],edge->getWeight(), edge->getEdgeType());
    }

    for(auto& node: n){
        if(!isItemOnScene(scene(), qgraphicsitem_cast<Node *>(node))){
            scene()->addItem(node);
        }
    }
    for(auto& edge: e){
        if(!isItemOnScene(scene(), qgraphicsitem_cast<Edge *>(edge))){
        scene()->addItem(edge);
        }
    }
}

void GraphWidget::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Enter:
        shuffle();
        break;
    default:
        QGraphicsView::keyPressEvent(event);
    }
}

void GraphWidget::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);

    QList<Node *> nodes;
    QList<Edge *> edges;
    const QList<QGraphicsItem *> items = scene()->items();
    for (QGraphicsItem *item : items) {
        if (Node *node = qgraphicsitem_cast<Node *>(item)){
            nodes << node;

        }
        if (Edge *edge = qgraphicsitem_cast<Edge *>(item)){
            edges << edge;
        }
    }

    for (Node *node : nodes){
        node->calculateForces();
    }


    bool itemsMoved = false;
    for (Node *node : nodes) {
        if (node->advancePosition())
            itemsMoved = true;
    }

    if (!itemsMoved) {
        killTimer(timerId);
        timerId = 0;
    }
}

#if QT_CONFIG(wheelevent)
void GraphWidget::wheelEvent(QWheelEvent *event)
{
    scaleView(pow(2., -event->angleDelta().y() / 240.0));
}
#endif

void GraphWidget::scaleView(qreal scaleFactor)
{
    qreal factor = transform().scale(scaleFactor, scaleFactor).mapRect(QRectF(0, 0, 1, 1)).width();
    if (factor < 0.07 || factor > 100)
        return;

    scale(scaleFactor, scaleFactor);
}

void GraphWidget::shuffle()
{
    const QList<QGraphicsItem *> items = scene()->items();
    for (QGraphicsItem *item : items) {
        if (qgraphicsitem_cast<Node *>(item))
            item->setPos(-150 + QRandomGenerator::global()->bounded(300), -150 + QRandomGenerator::global()->bounded(300));
    }
}

void GraphWidget::zoomIn()
{
    scaleView(qreal(1.2));
}

void GraphWidget::zoomOut()
{
    scaleView(1 / qreal(1.2));
}

bool isItemOnScene(QGraphicsScene *scene, QGraphicsItem *item)
{
    for(auto i: scene->items()){
        if(i==item){
            return true;
        }
    }
    return false;
}
