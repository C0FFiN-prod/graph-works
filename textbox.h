#ifndef TextBox_H
#define TextBox_H

#include <QPainter>
#include <QFont>
#include <QFontMetrics>
#include <QFontDatabase>
#include <QGraphicsItem>

class TextBox : public QGraphicsItem
{
public:
    bool containsCustomText = false;
    TextBox();
    TextBox(QString txt, QPoint coords);
    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *widget) override;
    void moveTo(QPoint coords);
    void setText(QString text);
    void setCustomText(QString text);
    QString getText();

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    //enum { Type = QGraphicsItem::ItemIsSelectable};
private:
    QString text;
    QColor selectionColor;
    QFont font;
    QPoint centerPoint;

};

#endif // TextBox_H
