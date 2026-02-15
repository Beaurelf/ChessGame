#include "pieceitem.h"
#include "ui/consts.h"
#include "ui/helper.h"

PieceItem::PieceItem(uint8_t initialPos, PieceType type, Color color, QGraphicsItem* parent)
    : QGraphicsObject(parent), m_type(type), m_color(color), m_pos(initialPos), m_coord (UIHelper::getCoordinate(m_pos))
{
    setFlag(QGraphicsItem::ItemSendsGeometryChanges);
    setAcceptHoverEvents(true);
    setZValue(1);
    setCursor(Qt::OpenHandCursor);
    setupUi();
}

void PieceItem::setupUi() {
    setPos(UIHelper::getCoordinate(m_pos));
    QString imagePath = UIHelper::getPieceImagePath(m_type, m_color);
    m_pixmap = QPixmap(imagePath).scaled(
        PIECE_ICON_SIZE,
        PIECE_ICON_SIZE,
        Qt::KeepAspectRatio,
        Qt::SmoothTransformation
    );
    this->show();
    update();
}

QRectF PieceItem::boundingRect() const {
    return QRectF(0, 0, BOARD_TILE_SIZE, BOARD_TILE_SIZE);
}

void PieceItem::moveToCell(int to) {
    m_pos = to;
    m_coord = UIHelper::getCoordinate(to);
    setPos(m_coord);
    update();
}

void PieceItem::paint(QPainter *painter, const QStyleOptionGraphicsItem*, QWidget*) {
    if (m_type == NONE || m_pixmap.isNull()) return;
    qreal offset = (BOARD_TILE_SIZE - PIECE_ICON_SIZE) / 2;
    painter->drawPixmap(offset, offset, m_pixmap);
}


void PieceItem::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    m_coord = pos();
    setZValue(100); // Passe au dessus des autres pièces
    setCursor(Qt::ClosedHandCursor);
    uint8_t from = UIHelper::getPosition(m_coord);
    emit piecePressed(from);
    event->accept();
}

void PieceItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
    setPos(event->scenePos() - QPointF(BOARD_TILE_SIZE / 2, BOARD_TILE_SIZE / 2));
    event->accept();
}

void PieceItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
    setCursor(Qt::OpenHandCursor);
    setZValue(1);

    QPointF dropPoint = pos();
    QPointF centerPoint(dropPoint.x() + BOARD_TILE_SIZE / 2, dropPoint.y() + BOARD_TILE_SIZE / 2);

    uint8_t to = UIHelper::getPosition(centerPoint);
    uint8_t from = UIHelper::getPosition(m_coord);

    if (to != from) {
        emit moveRequested(to);
    }

    setPos(m_coord);
    event->accept();
}
