#ifndef GRAPHWIDGET_H
#define GRAPHWIDGET_H

#include "node.h"
#include "edge.h"
#include "textbox.h"

#include "graphenums.h"
#include <QGraphicsView>

typedef QList<QList<double>> Matrix2D;
class Node;

class GraphWidget : public QGraphicsView
{
    Q_OBJECT

public:
    GraphWidget(QMap<QPair<Node*, Node*>, Edge*>* edges, QMap<unsigned int, Node*>* nodes, QFlags<GraphFlags> *flags, QWidget *parent = nullptr);
    qsizetype stabilizingIteration = 0;
    qsizetype getMaxStabilizingIteration();
    void runTimer();
    void initScene();
    void resetNodesColor();
    void resetEdgesColor();
    QFlags<GraphFlags> getFlags();

    QMap<QPair<Node *, Node *>, Edge *> *getEdges();
    QMap<unsigned int, Node *> *getNodes();
public slots:
    void zoomIn();
    void zoomOut();
protected:
    void keyPressEvent(QKeyEvent *event) override;
    void timerEvent(QTimerEvent *event) override;
#if QT_CONFIG(wheelevent)
    void wheelEvent(QWheelEvent *event) override;
#endif
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void scaleView(qreal scaleFactor);

private:
    qsizetype maxStabilizintIteration = 1000;
    bool dragging = false;
    QPointF prevScenePos;
    int timerId = 0;
    QGraphicsScene *scenePtr;
    QMap<QPair<Node*, Node*>, Edge*>* edges;
    QMap<unsigned int, Node*>* nodes;
    int amount;
    QFlags<GraphFlags> *flags;
};
bool isItemOnScene(QGraphicsScene *scene, QGraphicsItem *item);

#endif // GRAPHWIDGET_H
