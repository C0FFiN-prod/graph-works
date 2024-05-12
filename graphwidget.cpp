#include "graphwidget.h"


#include <math.h>
#include <QKeyEvent>
#include <QRandomGenerator>

GraphWidget::GraphWidget( QMap<QPair<Node*, Node*>, Edge*>* edges,QMap<unsigned int, Node*>* nodes, QWidget *parent)
    : QGraphicsView(parent), edges(edges), nodes(nodes)
{
    this->amount = nodes->size();

    QGraphicsScene *scene = new QGraphicsScene(this);
    scene->setItemIndexMethod(QGraphicsScene::NoIndex);
    scene->setSceneRect(-500, -300, 1000, 600);
    setScene(scene);
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
    for(auto& node: *nodes){
        if(!isItemOnScene(scene(), qgraphicsitem_cast<Node *>(node))){
            scene()->addItem(node);
        }
    }
    for(auto& edge: *edges){
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
