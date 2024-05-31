#include "TextBox.h"
#include "qgraphicsscene.h"
#include "qgraphicssceneevent.h"


TextBox::TextBox(): text(""), centerPoint(QPoint(0,0))
{
    selectionColor = Qt::white;
    setFlag(ItemIsSelectable);

}

TextBox::TextBox(QString txt, QPoint coords): text(txt), centerPoint(coords)
{
    selectionColor = Qt::white;
    setFlag(ItemIsSelectable);
}

QRectF TextBox::boundingRect() const
{   QFontMetrics metrics(this->font);
    QRect rect = metrics.boundingRect(text);
    int padding = 5;
    QRect backgroundRect = rect.adjusted(-padding*2, -padding*2, padding/2, padding/2);
    backgroundRect.moveCenter(centerPoint);

    return backgroundRect;
}

QPainterPath TextBox::shape() const
{
    QPainterPath path;
    QFontMetrics metrics(font);
    int padding = 5;
    QPointF offset(-padding,padding);
    QRect rect = metrics.boundingRect(text);
    QRect backgroundRect = rect.adjusted(-padding*2, -padding*2, padding/2, padding/2);
    backgroundRect.moveCenter((this->centerPoint));
    path.addEllipse(backgroundRect);
    return path;
}

void TextBox::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *widget)
{   //0078d4
    if(isSelected()){
        selectionColor = QColor(0,120,212);

    }else{
        selectionColor = Qt::white;
    }
    //setting up font parameters
    if(text.isEmpty()) return;
    this->font = painter->font();
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
    backgroundRect.moveCenter((this->centerPoint));
    //drawing background

    painter->setPen(Qt::NoPen);
    painter->setBrush(selectionColor);
    painter->drawEllipse(backgroundRect);
    //painter->setPen(Qt::yellow);
    //painter->drawRect(boundingRect());
    //drawing text
    painter->setPen(Qt::black);
    painter->drawText(backgroundRect, Qt::AlignVCenter | Qt::AlignHCenter, text);



}

void TextBox::moveTo(QPoint coords)
{
    centerPoint = coords;
}

void TextBox::setText(QString text)
{
    this->text =text;
}

void TextBox::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if(!(event->modifiers() & Qt::ControlModifier)){
        for(auto item: this->scene()->items()){
            if(item!=this){
                item->setSelected(false);

            }
        }
    }
    //QGraphicsItem::mousePressEvent(event);

    setSelected(!isSelected());
    update();
}



