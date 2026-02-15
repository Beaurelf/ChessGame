#ifndef PIECEITEM_H
#define PIECEITEM_H

#include <QGraphicsObject>
#include <QPixmap>
#include <QPointF>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QCursor>
#include "type.h"


class PieceItem : public QGraphicsObject {
    Q_OBJECT

public:
    explicit PieceItem(uint8_t initialPos, PieceType type, Color color, QGraphicsItem* parent = nullptr);

    void moveToCell(int to);


signals:
    /**
     * @brief Signal émis lorsque l'utilisateur relâche la pièce.
     * Le contrôleur devra valider si ce mouvement est légal.
     */
    void moveRequested(uint8_t to);
    void piecePressed(uint8_t from);

protected:
    // Méthodes standards de QGraphicsItem
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void setupUi();

private:
    PieceType m_type;
    Color m_color;
    QPixmap m_pixmap;
    // Variables internes pour le mécanisme de déplacement
    uint8_t m_pos;
    QPointF m_coord;
};

#endif // PIECEITEM_H
