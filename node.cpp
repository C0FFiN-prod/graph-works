
#include "edge.h"
#include "node.h"
#include "graphwidget.h"

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QRandomGenerator>
#include <QStyleOption>

Node::Node(int index, GraphWidget *graphWidget)
    : defaultColor(NodeColors::DefaultColor)
    , currentColor(defaultColor)
    , graph(graphWidget)
    , index(index)
    , displayName(QString::number(index))
{
    setFlag(ItemIsMovable);
    setFlag(ItemSendsGeometryChanges);
    setFlag(ItemIsSelectable);
    setCacheMode(DeviceCoordinateCache);
    setZValue(1);
    this->setPos(QRandomGenerator::global()->bounded(300), QRandomGenerator::global()->bounded(300));
}

Node::Node()
    : defaultColor(NodeColors::DefaultColor)
    , currentColor(defaultColor)
    , graph(nullptr)
    , index(0)
    , displayName("0")
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

void Node::calculateForces(bool manual)
{
    if (!scene() || manual || scene()->mouseGrabberItem() == this) {
        newPos = pos();
        this->graph->stabilizingIteration = 0;
        return;
    }
    QRectF sceneRect = scene()->sceneRect();
    double minScreen = qMin(sceneRect.width(), sceneRect.height());
    double threshold = 0.1;
    double k_r = 80;
    double k_a = .05;
    double k_c = 0.002;
    double damping = 1;
    double cooling = std::exp(-0.01 * graph->stabilizingIteration);
    // Sum up all forces pushing this item away
    QPointF vec;
    qreal xvel = 0;
    qreal yvel = 0;
    qsizetype nodesCount = 0;
    const QList<QGraphicsItem *> items = scene()->items();
    for (QGraphicsItem *item : items) {
        Node *node = qgraphicsitem_cast<Node *>(item);
        if (!node)
            continue;
        nodesCount++;
        vec = mapToItem(node, 0, 0);
        qreal dx = vec.x();
        qreal dy = vec.y();
        double l = std::pow(dx * dx + dy * dy, 1.0);
        xvel += k_r * dx / (l + 0.001);
        yvel += k_r * dy / (l + 0.001);
    }
    vec = mapFromScene(0, 0);
    xvel += k_c * vec.x() * qMax(sqrt(nodesCount), 0.1) / minScreen * 1000;
    yvel += k_c * vec.y() * qMax(sqrt(nodesCount), 0.1) / minScreen * 1000;
    // Now subtract all forces pulling items together
    for (Edge *edge : std::as_const(edgeList)) {
        if (edge->getEdgeType() == EdgeType::Loop)
            continue;
        double k_t = edge->getEdgeType() == EdgeType::SingleDirection ? 2 : 1;
        if (edge->sourceNode() == this)
            vec = mapToItem(edge->destNode(), 0, 0);
        else
            vec = mapToItem(edge->sourceNode(), 0, 0);
        qreal dx = vec.x();
        qreal dy = vec.y();
        double l = sqrt(dx * dx + dy * dy);
        double dl = (minScreen / qLn(nodesCount) / 2 - l);
        if (l < 0.1)
            l += 0.1;
        xvel += dx / l * k_a * dl * k_t;
        yvel += dy / l * k_a * dl * k_t;
    }

    xvel = qMax(-minScreen / 10, qMin(minScreen / 10, xvel)) * damping * cooling;
    yvel = qMax(-minScreen / 10, qMin(minScreen / 10, yvel)) * damping * cooling;

    if (qAbs(xvel) < threshold)
        xvel = 0;
    if (qAbs(yvel) < threshold)
        yvel = 0;

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
        if (children.contains(this)) {
            edgeList.removeIf([](Edge* i){
                return i->getEdgeType() == EdgeType::Loop;
            });
            this->children.remove(this);
            this->parents.remove(this);
        }
    }else {
        if (children.contains(node)) {
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

int Node::getIndex() const
{
    return this->index;
}

void Node::setIndex(unsigned int i)
{
    this->index = i;
}

void Node::toggle(bool enabled)
{
    this->enabled = enabled;
    setDefaultColor(enabled ? NodeColors::DefaultColor : NodeColors::DisabledColor);
    for (auto e : edgeList) {
        e->setDefaultColor(e->isEnabled() && e->sourceNode()->isEnabled()
                                   && e->destNode()->isEnabled()
                               ? NodeColors::DefaultColor
                               : NodeColors::DisabledColor);
        e->resetColor();
        // qDebug() << this->index << e->sourceNode()->isEnabled() << e->destNode()->isEnabled();
    }
    this->resetColor();
}

bool Node::isEnabled()
{
    return this->enabled;
}

QString Node::getDisplayName() const
{
    return this->displayName;
}

void Node::setDisplayName(const QString &name)
{
    this->displayName = name;
}

void Node::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    // Установка стиля узла
    painter->fillPath(shape(), QBrush(Qt::white));
    // Настройка шрифта для текста
    QFont font = painter->font();
    font.setFamily("Lucida Console");
    // Отображение индекса или имени узла (в верхнем секторе)
    QString nodeText = graph->getFlags().testFlag(GraphFlags::DisplayIndex) ? QString::number(index)
                                                                            : displayName;
    if (!graph->getFlags().testAnyFlags(
            {GraphFlags::ShowWeights, GraphFlags::ShowFlow, GraphFlags::ShowBandwidth})) {
        font.setBold(true);
        font.setPointSize(qMax(4, qMin(nodeSize / nodeText.length(), 14)));
        painter->setFont(font);
        painter->setPen(Qt::black);
        QFontMetrics metrics(font);
        painter->drawText(boundingRect().center() - metrics.tightBoundingRect(nodeText).center(),
                          nodeText);
    } else {
        // Проверяем
        // Проверяем наличие петли
        Edge *loopEdge = nullptr;
        for (const auto &edge : edgeList) {
            if (edge->getEdgeType() == EdgeType::Loop) {
                loopEdge = edge;
                break;
            }
        }

        if (loopEdge) {
            // Вычисляем активные параметры петли
            QList<QString> activeParameters;
            {
                activeParameters.append(nodeText);
                if (graph->getFlags().testFlag(GraphFlags::ShowWeights)) {
                    activeParameters.append(QString::number(loopEdge->getWeight()));
                }
                if (graph->getFlags().testFlag(GraphFlags::ShowFlow)) {
                    activeParameters.append(QString::number(loopEdge->getFlow()));
                }
                if (graph->getFlags().testFlag(GraphFlags::ShowBandwidth)) {
                    activeParameters.append(QString::number(loopEdge->getBandwidth()));
                }
            }
            font.setBold(false);

            // Расчёт и отображение секторов

            int paramCount = activeParameters.size();
            if (paramCount > 0) {
                double radius = nodeSize / 2.0;
                double spanAngle = 360.0 / paramCount;
                double startAngle = (180.0 - spanAngle) / 2;

                for (const auto &param : activeParameters) {
                    // Рисуем сектор
                    painter->setPen(Qt::black);
                    painter->drawPie(QRectF(-radius, -radius, radius * 2, radius * 2),
                                     startAngle * 16,
                                     spanAngle * 16);

                    // Отображение текста внутри сектора
                    double angle = startAngle + spanAngle / 2.0;
                    double textX = radius * 0.5 * cos(angle * M_PI / 180.0);
                    double textY = -radius * 0.5 * sin(angle * M_PI / 180.0);
                    QPointF textPnt{textX, textY};
                    font.setPointSize(
                        qMax(4,
                             qMin((nodeSize / (activeParameters.size() - 1)) / param.length(), 10)));
                    painter->setFont(font);
                    QFontMetrics metrics(font);
                    textPnt -= metrics.tightBoundingRect(param).center();
                    painter->setPen(Qt::black);
                    painter->drawText(textPnt, param);

                    startAngle += spanAngle;
                }
            }
        }
    }
    painter->setPen(isSelected() ? QPen({NodeColors::SelectionColor}, 2) : QPen(currentColor, 2));
    painter->drawPath(shape());
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

void Node::setDefaultColor(const NodeColors clr)
{
    defaultColor = QColor(clr);
}
void Node::resetColor()
{
    currentColor = defaultColor;
}
void Node::setCurrentColor(const QColor clr)
{
    currentColor = clr;
}

QVariant Node::itemChange(GraphicsItemChange change, const QVariant &value)
{
    switch (change) {
    case ItemPositionHasChanged:
        for (Edge *edge : std::as_const(edgeList))
            edge->adjust();
        graph->runTimer();
        break;
    default:
        break;
    };

    return QGraphicsItem::itemChange(change, value);
}

void Node::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->modifiers() & Qt::ControlModifier) {
        QGraphicsItem::mousePressEvent(event);
        return;
    }
    event->accept();
    for (auto item : this->scene()->items()) {
        if (item != this) {
            item->setSelected(false);
        }
    }

    this->setSelected(!isSelected());
    update();

}

void Node::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    update();
    QGraphicsItem::mouseReleaseEvent(event);
}
