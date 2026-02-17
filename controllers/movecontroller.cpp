#include "movecontroller.h"
#include "ui/consts.h"

MoveController::MoveController() : m_generator() {}

MoveMasks MoveController::getLegalMoves(uint8_t pos, const ChessBitBoard& board) const {
    MoveMasks result;

    PieceType type = board.getPieceType(pos);
    Color color = board.getPieceColor(pos);

    if (type == PieceType::NONE || color == Color::NO_COLOR) return result;

    if (type != PieceType::PAWN) {
        return getStandardMoves(pos, board, type, color);
    }
    return getPawnMoves(pos, board, color);
}

MoveMasks MoveController::getAllLegalMoves(Color color, const ChessBitBoard& board) const {
    MoveMasks totalMasks;
    uint64_t occupancy = board.getOccupancy(color);

    for (uint8_t i = 0; i < NB_CELLS; ++i) {
        if ((occupancy >> i) & 1ULL) {
            MoveMasks pieceMoves = getLegalMoves(i, board);
            totalMasks.quietMoves |= pieceMoves.quietMoves;
            totalMasks.captureMoves |= pieceMoves.captureMoves;
        }
    }
    return totalMasks;
}

MoveMasks MoveController::getStandardMoves(uint8_t pos, const ChessBitBoard& board, PieceType type, Color color) const {
    MoveMasks result;

    uint64_t ownPieces    = board.getOccupancy(color);
    uint64_t enemyPieces  = board.getOccupancy(color == WHITE ? BLACK : WHITE);
    uint64_t allOccupancy = board.getOccupancy(2);

    uint64_t rawMoves = m_generator.getPossibleMoves(type, color, pos, allOccupancy);

    uint64_t validMoves    = rawMoves & ~ownPieces;
    result.captureMoves    = validMoves & enemyPieces;
    result.quietMoves      = validMoves & ~allOccupancy;

    return result;
}

MoveMasks MoveController::getPawnMoves(uint8_t pos, const ChessBitBoard& board, Color color) const {
    MoveMasks result;
    uint64_t b = (1ULL << pos);
    uint64_t allPieces   = board.getOccupancy(2);
    uint64_t enemyPieces = board.getOccupancy(color == WHITE ? BLACK : WHITE);

    uint64_t rawMoves = m_generator.getPossibleMoves(PieceType::PAWN, color, pos, allPieces);

    // Séparer les cases d'avance et les diagonales d'attaque
    uint64_t forwardMask = (color == WHITE) ? ((b << 8) | (b << 16))
                                            : ((b >> 8) | (b >> 16));

    uint64_t attackMask = rawMoves & ~forwardMask;

    // Avance : uniquement cases vides
    uint64_t forwardMoves = rawMoves & forwardMask & ~allPieces;

    // Double pas bloqué si step1 est occupé
    uint64_t step1 = (color == WHITE) ? (b << 8) : (b >> 8);
    if (step1 & allPieces) {
        uint64_t step2 = (color == WHITE) ? (b << 16) : (b >> 16);
        forwardMoves &= ~step2;  // retirer le double pas si step1 bloqué
    }

    // Attaques : uniquement cases ennemies
    result.quietMoves   = forwardMoves;
    result.captureMoves = attackMask & enemyPieces;

    return result;
}

