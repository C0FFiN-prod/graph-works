#include "edge.h"
#include "node.h"

#include <QPainter>
#include <QtMath>

QFlags<GraphFlags> *Edge::graphFlags = nullptr;

void drawTextAt(QPainter *painter, QPointF offsetPnt, QString& text){
    //setting up font parameters
    if(text.isEmpty()) return;
    QFont font = painter->font();
    font.setBold(false);
    font.setPointSize(7);
    painter->setFont(font);
    painter->setPen(Qt::black);

    //get text bounds
    QFontMetrics metrics(font);
    QRect rect = metrics.boundingRect(text);

    //draw padding
    int padding = 5;
    QPointF offset(-padding,padding);
    QRect backgroundRect = rect.adjusted(-padding*2, -padding*2, padding/2, padding/2);
    backgroundRect.moveCenter((offsetPnt).toPoint());
    //drawing background
    painter->setBrush(QColor(255, 255, 255, 255));
    painter->setPen(Qt::NoPen);
    painter->drawEllipse(backgroundRect);

    //drawing text
    painter->setPen(Qt::black);
    painter->drawText(backgroundRect, Qt::AlignVCenter | Qt::AlignHCenter, text);
}
void drawArrowAt(QPainter *painter, QPointF at, double angle, double arrowSize){
    QPointF destArrowP1 = at + QPointF(sin(angle - M_PI / 3) * arrowSize, cos(angle - M_PI / 3) * arrowSize);
    QPointF destArrowP2 = at + QPointF(sin(angle - M_PI + M_PI / 3) * arrowSize, cos(angle - M_PI + M_PI / 3) * arrowSize);
    painter->setBrush(Qt::black);
    painter->drawPolygon(QPolygonF() << at << destArrowP1 << destArrowP2);

}


Edge::Edge(Node *sourceNode, Node *destNode, double weight, EdgeType edgeType = EdgeType::SingleDirection)
    : edgeType(edgeType), source(sourceNode), dest(destNode), flow(0), weight(weight), bandwidth(0)
{
    setAcceptedMouseButtons(Qt::NoButton);
    source->addEdge(this);
    dest->addEdge(this);
    adjust();
    sourceNode->connectToNode(destNode);
}



Node *Edge::sourceNode() const
{
    return source;
}

Node *Edge::destNode() const
{
    return dest;
}

void Edge::setWeight(double weight)
{
    this->weight = weight;
}

void Edge::setFlow(double flow)
{
    this->flow = flow;
}

void Edge::setBandwidth(double bandwidth)
{
    this->bandwidth = bandwidth;
}

void Edge::setEdgeType(EdgeType type)
{
    this->edgeType = type;
}

double Edge::getWeight() const
{
    return this->weight;
}

double Edge::getFlow() const
{
    return this->flow;
}

double Edge::getBandwidth() const
{
    return this->bandwidth;
}

EdgeType Edge::getEdgeType()
{
    return this->edgeType;
}
void Edge::adjust()
{
    if (!source || !dest)
        return;

    QLineF line(mapFromItem(source, 0, 0), mapFromItem(dest, 0, 0));
    qreal length = line.length();

    prepareGeometryChange();

    if (length > qreal(20.)) {
        QPointF edgeOffset((line.dx() * nodeSize/2) / length, (line.dy() * nodeSize/2) / length);
        sourcePoint = line.p1() + edgeOffset;
        destPoint = line.p2() - edgeOffset;
    } else {
        sourcePoint = destPoint = line.p1();
    }
}

Edge::~Edge()
{
    //this->source->disconnectFromNode(this->dest);
}

void Edge::setFlagsPtr(QFlags<GraphFlags> *flagsPtr)
{
    graphFlags = flagsPtr;
}

QRectF Edge::boundingRect() const
{
    if (!source || !dest)
        return QRectF();

    qreal penWidth = 1;
    qreal extra = (penWidth + arrowSize) / 2.0;
    if(this->edgeType==EdgeType::BiDirectionalSame||this->edgeType==EdgeType::SingleDirection){
        return QRectF(sourcePoint, QSizeF(destPoint.x() - sourcePoint.x(),
                                          destPoint.y() - sourcePoint.y()))
            .normalized()
            .adjusted(-extra, -extra, extra, extra);

    }else{
        return QRectF(sourcePoint-QPointF(curvines, curvines), destPoint+QPointF(curvines, curvines) )
                          .normalized()
                          .adjusted(-extra, -extra, extra, extra);
    }
}

void Edge::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    if (!source || !dest)
        return;

    QLineF line(sourcePoint, destPoint);
    if (qFuzzyCompare(line.length(), qreal(0.)))
        return;

    //calculate curve points
    QPointF midPoint = QPointF((line.p1().x()+ line.p2().x())/2,(line.p1().y()+ line.p2().y())/2 );
    QString text = "";
    bool slashNeeded = false;
    if(graphFlags->testFlag(GraphFlags::ShowWeights)){
        text.append(QString::number(this->weight));
        slashNeeded = true;
    }
    if(graphFlags->testFlag(GraphFlags::ShowFlow)){
        if(slashNeeded)
            text.append('|');
        text.append(QString::number(this->flow));
        slashNeeded = true;
    }
    if(graphFlags->testFlag(GraphFlags::ShowBandwidth)){
        if(slashNeeded)
            text.append('|');
        text.append(QString::number(this->bandwidth));
    }

    //draw lines and arrows

    if(this->edgeType==EdgeType::SingleDirection){
        //draw straight line
        painter->setPen(QPen(Qt::black, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter->drawLine(line);

        drawArrowAt(painter, destPoint, atan2(-line.dy(), line.dx()), 10);

        drawTextAt(painter, midPoint, text);

    }else if(this->edgeType==EdgeType::BiDirectionalDiff){
        //calculate offset point and angle for curviness
        double teta = atan2((line.p2().y()-midPoint.y()),(line.p2().x()-midPoint.x()));
        QPointF offsetPnt = QPointF(midPoint.x() + curvines * cos(teta+M_PI/2), midPoint.y() + curvines * sin(teta+M_PI/2));

        //draw curve line
        QPainterPath path;
        painter->setPen(QPen(Qt::black, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        path.moveTo(line.p1());
        path.cubicTo(line.p1(),offsetPnt, line.p2());
        painter->drawPath(path);

        drawArrowAt(painter, destPoint, M_PI-atan2(offsetPnt.y()-destPoint.y(), offsetPnt.x()-destPoint.x()), 10);

        drawTextAt(painter,  QPointF(midPoint.x() + curvines/2 * cos(teta+M_PI/2), midPoint.y() + curvines/2 * sin(teta+M_PI/2)), text);

    }else if(this->edgeType==EdgeType::BiDirectionalSame){
        if(this->source->getIndex()<this->dest->getIndex()){
        //draw straight line
        painter->setPen(QPen(Qt::black, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter->drawLine(line);

        //if weight needs to be drawn
            drawTextAt(painter, midPoint, text);
        }
    }

}

