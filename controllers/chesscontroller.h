#ifndef CHESSCONTROLLER_H
#define CHESSCONTROLLER_H

#include "ai/chessai.h"
#include "soundcontroller.h"
#include "movecontroller.h"
#include "models/chessbitboard.h"
#include <QObject>
#include <QFutureWatcher>

class ChessController: public QObject
{
    Q_OBJECT
public:
    ChessController(bool machine, QObject *parent = nullptr);
    PieceType getPieceType(uint8_t pos) const;
    Color getPieceColor(uint8_t pos) const;
    MoveMasks getCurrentPlayerLegalMoves(uint8_t from);

public slots:
    void onMoveRequested(uint8_t from, uint8_t to);
    void onPromotionPieceSelected(uint8_t pos, PieceType newType);
    void onAiMoveReady();

signals:
    void moveExecuted(uint8_t from, uint8_t to);
    void castleRookMoved(uint8_t rookFrom, uint8_t rookTo);
    void pieceCaptured(PieceType type, const Color& color);
    void checkMateDetected(const Color& color);
    void promotionDetected(uint8_t pos);
    void pawnPromoted(uint8_t pos, const PieceType& newType, const Color& color);

private:
    ChessBitBoard m_bitBoard;
    MoveController m_moveController;
    SoundController m_soundController;
    bool m_machine;
    bool m_isAiTurn;
    Color m_currentPlayer;
    ChessAI m_chessAi;
    QFutureWatcher<AIHelper::Move> m_aiWatcher;

    /**
     * @brief Vérifie si la pièce à une position donnée appartient au joueur courant.
     * @param pos Index de la case (0-63).
     * @return true si la case contient une pièce de la couleur demandée.
     */
    bool isCurrentPlayerPiece(uint8_t pos) const;
    MoveMasks getLegalMoves(uint8_t pos);
    bool isValidMove(uint8_t from, uint8_t to);
    bool isKingInCheck(Color color, const ChessBitBoard& board) const ;
    bool isCheckmate(Color color, const ChessBitBoard& board);
    void endTurn();
    void handleAiTurnIfNeeded();
};

#endif // CHESSCONTROLLER_H
