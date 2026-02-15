#include "cellitem.h"
#include "ui/helper.h"
#include <QPainter>
#include <QBrush>
#include <QRadialGradient>
#include <QGraphicsSceneMouseEvent>

CellItem::CellItem(int cellIndex, QString color, QGraphicsItem* parent)
    : QGraphicsObject(parent),
    m_cellIndex(cellIndex),
    m_color(color),
    m_isHighlighted(false),
    m_isCapture(false)
{
    setPos(UIHelper::getCoordinate(cellIndex));
}

QRectF CellItem::boundingRect() const {
    return QRectF(0, 0, BOARD_TILE_SIZE, BOARD_TILE_SIZE);
}

void CellItem::setHighlight(bool active, bool isCapture) {
    m_isHighlighted = active;
    m_isCapture = isCapture;
    update();
}

void CellItem::paint(QPainter *painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    Q_UNUSED(option);
    Q_UNUSED(widget);

    QRectF rec = boundingRect();

    painter->fillRect(rec, m_color);

    if (!m_isHighlighted) return;
    if (m_isCapture) {
        // Style pour une capture : on recouvre d'une couleur spécifique (Beige)
        QColor captureColor(COLOR_RED);
        captureColor.setAlpha(150);
        painter->fillRect(rec, captureColor);
    } else {
        // Style pour un déplacement simple : le petit cercle noir transparent (Gradient)
        QPointF center(rec.width() / 2, rec.height() / 2);
        painter->setPen(Qt::NoPen);

        // Création du gradient radial pour un effet plus doux
        QRadialGradient gradient(center, BOARD_TILE_SIZE / 5);
        QColor markerColor = QColor(COLOR_BLACK);
        markerColor.setAlpha(150);

        gradient.setColorAt(0, markerColor);
        gradient.setColorAt(1, Qt::transparent);

        painter->setBrush(QBrush(gradient));
        painter->drawEllipse(center, BOARD_TILE_SIZE / 6, BOARD_TILE_SIZE / 6);
    }

}

void CellItem::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    QGraphicsObject::mousePressEvent(event);
    emit cellPressed(m_cellIndex);
}
