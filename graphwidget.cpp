#include "graphwidget.h"

#include <QScrollBar>
#include <math.h>
#include <QKeyEvent>
#include <QRandomGenerator>


GraphWidget::GraphWidget(QMap<QPair<Node*, Node*>, Edge*>* edges, QMap<unsigned int, Node*>* nodes, QFlags<GraphFlags> *flags, QWidget *parent)
    : QGraphicsView(parent), scenePtr(nullptr), edges(edges), nodes(nodes), flags(flags)
{
    this->amount = nodes->size();
    scenePtr = new QGraphicsScene(this);
    scenePtr->setItemIndexMethod(QGraphicsScene::NoIndex);
    scenePtr->setSceneRect(-500, -300, 1000, 600);
    setScene(scenePtr);

    setCacheMode(CacheBackground);
    setViewportUpdateMode(FullViewportUpdate); // TODO razobrat'sa cho za hren'
    setRenderHint(QPainter::Antialiasing);
    setTransformationAnchor(AnchorUnderMouse);
    scale(qreal(1), qreal(1));
    setMinimumSize(300, 200);
    setTransformationAnchor(QGraphicsView::NoAnchor);
    setMouseTracking(true);

//    setOptimizationFlag(QGraphicsView::DontAdjustForAntialiasing);
}

void GraphWidget::runTimer()
{
    if (!timerId)
        timerId = startTimer(1000 / 25);
}

void GraphWidget::initScene()
{
    for(auto& node: *nodes){
        if(!isItemOnScene(scene(), qgraphicsitem_cast<Node *>(node))&& node!=nullptr){
            scene()->addItem(node);
        }
    }
    for(auto& edge: *edges){
        if(!isItemOnScene(scene(), qgraphicsitem_cast<Edge *>(edge)) && edge!=nullptr){
            scene()->addItem(edge);
            scene()->addItem(&edge->info);

        }
    }
}

QFlags<GraphFlags> GraphWidget::getFlags()
{
    return *flags;
}

void GraphWidget::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Enter:
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
        node->calculateForces(flags->testFlag(GraphFlags::ManualMode));
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
void GraphWidget::mousePressEvent(QMouseEvent *event){
    if((event->button() == Qt::MiddleButton) &&
        (this->dragMode()==QGraphicsView::NoDrag)){
        // Start dragging and save the last mouse position
        setCursor(Qt::ClosedHandCursor);
        dragging = true;
        prevScenePos = event->position();
        event->accept();
    } else
        QGraphicsView::mousePressEvent(event);
}
void GraphWidget::mouseMoveEvent(QMouseEvent *event){
    if(dragging){
        QPointF diff = event->position() - prevScenePos;
        prevScenePos = event->position();

        // Scroll the view accordingly
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() - diff.x());
        verticalScrollBar()->setValue(verticalScrollBar()->value() - diff.y());
        event->accept();
    }
    else
        QGraphicsView::mouseMoveEvent(event);
}
void GraphWidget::mouseReleaseEvent(QMouseEvent *event){
    if((event->button() == Qt::MiddleButton)){
        setCursor(Qt::ArrowCursor);
        //setDragMode(NoDrag);
        dragging = false;
        event->accept();
    } else
        QGraphicsView::mouseReleaseEvent(event);
}
void GraphWidget::resizeEvent(QResizeEvent *event)
{
    int w = this->width() - nodeSize, h = this->height() - nodeSize;
    scene()->setSceneRect(-w/2, -h/2, w, h);
    runTimer();
    QGraphicsView::resizeEvent(event);
}

void GraphWidget::scaleView(qreal scaleFactor)
{
    qreal factor = transform().scale(scaleFactor, scaleFactor).mapRect(QRectF(0, 0, 1, 1)).width();
    if (factor < 0.07 || factor > 100)
        return;

    scale(scaleFactor, scaleFactor);
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
