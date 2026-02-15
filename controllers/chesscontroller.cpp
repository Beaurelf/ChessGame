#include "chesscontroller.h"
#include "controllers/consts.h"
#include <QDebug>

ChessController::ChessController(bool machine, QObject *parent) : QObject(parent), m_moveController(), m_machine(machine), m_currentPlayer(WHITE) {}

PieceType ChessController::getPieceType(uint8_t pos) const
{
    return m_bitBoard.getPieceType(pos);
}

Color ChessController::getPieceColor(uint8_t pos) const
{
    return m_bitBoard.getPieceColor(pos);
}

MoveMasks ChessController::getLegalMoves(uint8_t from) const {
    MoveMasks finalMoves = {};
    if (!isCurrentPlayerPiece(from)) return finalMoves;

    // Récupérer les coups "Pseudo-Légaux" (Géométrie + Obstacles)
    // Ce sont les coups possibles physiquement sans se soucier de la sécurité du roi
    MoveMasks pseudoMoves = m_moveController.getLegalMoves(from, m_bitBoard);

    auto filterMask = [&](uint64_t mask) -> uint64_t {
        uint64_t validMask = 0ULL;

        while (mask) {
            // On récupère l'index du prochain coup à tester (bit de poids faible)
            uint8_t to = __builtin_ctzll(mask);

            // --- SIMULATION ---
            // On crée une copie temporaire du plateau
            ChessBitBoard boardCopy = m_bitBoard;
            // On joue le coup sur la copie
            boardCopy.update(from, to);
            // Si mon Roi survit sur ce plateau simulé, le coup est valide
            if (!isKingInCheck(m_currentPlayer, boardCopy)) {
                validMask |= (1ULL << to);
            }
            // On supprime le bit traité pour passer au suivant
            mask &= (mask - 1);
        }
        return validMask;
    };

    finalMoves.quietMoves = filterMask(pseudoMoves.quietMoves);
    finalMoves.captureMoves = filterMask(pseudoMoves.captureMoves);
    return finalMoves;
}

bool ChessController::isValidMove(uint8_t from, uint8_t to) const
{
    MoveMasks moveMasks = getLegalMoves(from);
    return ((1ULL << to) & (moveMasks.quietMoves | moveMasks.captureMoves)) != 0;
}

bool ChessController::isCurrentPlayerPiece(uint8_t pos) const
{
    return m_currentPlayer == m_bitBoard.getPieceColor(pos);
}

bool ChessController::isKingInCheck(Color color, const ChessBitBoard& board) const {
    Color enemyColor = (color == WHITE) ? BLACK : WHITE;
    uint8_t myKingPos = board.getKingPosition(color);
    MoveMasks enemyMoves = m_moveController.getAllLegalMoves(enemyColor, board);
    return (enemyMoves.captureMoves & (1ULL << myKingPos)) != 0;
}

bool ChessController::isCheckmate(Color color, const ChessBitBoard& board) const {
    if (!isKingInCheck(color, board)) {
        return false;
    }

    uint64_t myPieces = m_bitBoard.getOccupancy(m_currentPlayer);
    while (myPieces) {
        uint8_t pos = __builtin_ctzll(myPieces);
        MoveMasks moves = getLegalMoves(pos);
        if (moves.quietMoves || moves.captureMoves) {
            return false;
        }
        // On supprime le bit traité pour passer à la pièce suivante
        myPieces &= (myPieces - 1);
    }
    // Si on a testé toutes les pièces et qu'aucune ne peut bouger alors qu'on est en échec...
    return true;
}

void ChessController::onPromotionPieceSelected(uint8_t pos, PieceType newType) {
    m_bitBoard.promotePawn(pos, newType, m_currentPlayer);
    emit pawnPromoted(pos, newType, m_currentPlayer);
    endTurn();
}

void ChessController::onMoveRequested(uint8_t from, uint8_t to)
{
    if(!isCurrentPlayerPiece(from)) return;
    if(!isValidMove(from, to)) return;

    PieceType capturedType = m_bitBoard.getPieceType(to);
    PieceType movingType = m_bitBoard.getPieceType(from);
    Color capturedColor = (m_currentPlayer == WHITE) ? BLACK : WHITE;
    m_bitBoard.update(from, to);
    emit moveExecuted(from, to);
    if (capturedType != PieceType::NONE) {
        emit pieceCaptured(capturedType, capturedColor);
    }
    bool isWhitePromo = (m_currentPlayer == WHITE && movingType == PAWN && ((1ULL << to) & RANK_8)); // Rang 8
    bool isBlackPromo = (m_currentPlayer == BLACK && movingType == PAWN && ((1ULL << to) & RANK_1)); // Rang 1
    if (isWhitePromo || isBlackPromo) {
        // IMPORTANT : On ne change pas encore de tour.
        emit promotionDetected(to);
        return;
    }
    endTurn();
}

void ChessController::endTurn() {
    m_currentPlayer = (m_currentPlayer == WHITE) ? BLACK : WHITE;
    if(isCheckmate(m_currentPlayer, m_bitBoard)){
        emit checkMateDetected(m_currentPlayer);
        return;
    }
    if(isKingInCheck(m_currentPlayer, m_bitBoard)) {
        emit kingInCheck(m_currentPlayer);
    }
}
