#ifndef CELLITEM_H
#define CELLITEM_H

#include "ui/consts.h"
#include <QGraphicsItem>

class CellItem : public QGraphicsObject {
    Q_OBJECT

public:
    CellItem(int cellIndex, QString color, QGraphicsItem* parent = nullptr);

    QRectF boundingRect() const override;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem*, QWidget*) override;

    void setHighlight(bool active, bool isCapture = false);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

private:
    int m_cellIndex;
    QColor m_color;
    bool m_isHighlighted = false;
    bool m_isCapture = false;

signals:
    void cellPressed(int index);
};

#endif // CELLITEM_H
