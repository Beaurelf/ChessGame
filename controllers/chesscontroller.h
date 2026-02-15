#ifndef CHESSCONTROLLER_H
#define CHESSCONTROLLER_H

#include "controllers/movecontroller.h"
#include "models/chessbitboard.h"
#include <QObject>

class ChessController: public QObject
{
    Q_OBJECT
public:
    ChessController(bool machine, QObject *parent = nullptr);
    PieceType getPieceType(uint8_t pos) const;
    Color getPieceColor(uint8_t pos) const;
    MoveMasks getLegalMoves(uint8_t pos) const;

public slots:
    void onMoveRequested(uint8_t from, uint8_t to);
    void onPromotionPieceSelected(uint8_t pos, PieceType newType);

signals:
    void moveExecuted(uint8_t from, uint8_t to);
    void pieceCaptured(PieceType type, const Color& color);
    void kingInCheck(const Color& color);
    void checkMateDetected(const Color& color);
    void promotionDetected(uint8_t pos);
    void pawnPromoted(uint8_t pos, const PieceType& newType, const Color& color);

private:
    ChessBitBoard m_bitBoard;
    MoveController m_moveController;
    bool m_machine;
    Color m_currentPlayer;

    /**
     * @brief Vérifie si la pièce à une position donnée appartient au joueur courant.
     * @param pos Index de la case (0-63).
     * @return true si la case contient une pièce de la couleur demandée.
     */
    bool isCurrentPlayerPiece(uint8_t pos) const;
    bool isValidMove(uint8_t from, uint8_t to) const;
    bool isKingInCheck(Color color, const ChessBitBoard& board) const;
    bool isCheckmate(Color color, const ChessBitBoard& board) const;
    void endTurn();
};

#endif // CHESSCONTROLLER_H
