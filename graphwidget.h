#ifndef GRAPHWIDGET_H
#define GRAPHWIDGET_H


#include "edge.h"
#include <QGraphicsView>
typedef QList<QList<double>> Matrix2D;
class Node;

class GraphWidget : public QGraphicsView
{
    Q_OBJECT

public:
    GraphWidget(QMap<QPair<Node*, Node*>, Edge*>* edges, QMap<unsigned int, Node*>* nodes, QWidget *parent = nullptr);

    void itemMoved();
    void initScene();

public slots:
    void shuffle();
    void zoomIn();
    void zoomOut();
protected:
    void keyPressEvent(QKeyEvent *event) override;
    void timerEvent(QTimerEvent *event) override;
#if QT_CONFIG(wheelevent)
    void wheelEvent(QWheelEvent *event) override;
#endif

    void scaleView(qreal scaleFactor);

private:
    int timerId = 0;
    //Node *centerNode;
    //Matrix2D matrix;

    QMap<QPair<Node*, Node*>, Edge*>* edges;
    QMap<unsigned int, Node*>* nodes;

    int amount;
};

#endif // GRAPHWIDGET_H
