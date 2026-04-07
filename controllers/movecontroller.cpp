#include "movecontroller.h"

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

    for (uint8_t i = 0; i < 64; ++i) {
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

    // --- Roque ---
    if (type == KING) {
        Color enemyColor = (color == WHITE) ? BLACK : WHITE;
        uint8_t rights = board.getCastlingRights();

        if (color == WHITE) {
            // Petit roque : e1(4) → g1(6), tour h1(7) → f1(5)
            if ((rights & ChessBitBoard::CASTLE_WK)
                && !(allOccupancy & ((1ULL << 5) | (1ULL << 6)))
                && !isCellAttacked(4, enemyColor, board)
                && !isCellAttacked(5, enemyColor, board))
            {
                result.quietMoves |= (1ULL << 6);
            }
            // Grand roque : e1(4) → c1(2), tour a1(0) → d1(3)
            if ((rights & ChessBitBoard::CASTLE_WQ)
                && !(allOccupancy & ((1ULL << 1) | (1ULL << 2) | (1ULL << 3)))
                && !isCellAttacked(4, enemyColor, board)
                && !isCellAttacked(3, enemyColor, board))
            {
                result.quietMoves |= (1ULL << 2);
            }
        } else {
            // Petit roque : e8(60) → g8(62), tour h8(63) → f8(61)
            if ((rights & ChessBitBoard::CASTLE_BK)
                && !(allOccupancy & ((1ULL << 61) | (1ULL << 62)))
                && !isCellAttacked(60, enemyColor, board)
                && !isCellAttacked(61, enemyColor, board))
            {
                result.quietMoves |= (1ULL << 62);
            }
            // Grand roque : e8(60) → c8(58), tour a8(56) → d8(59)
            if ((rights & ChessBitBoard::CASTLE_BQ)
                && !(allOccupancy & ((1ULL << 57) | (1ULL << 58) | (1ULL << 59)))
                && !isCellAttacked(60, enemyColor, board)
                && !isCellAttacked(59, enemyColor, board))
            {
                result.quietMoves |= (1ULL << 58);
            }
        }
    }

    return result;
}

bool MoveController::isCellAttacked(uint8_t square, Color attackerColor, const ChessBitBoard& board) const {
    uint64_t allOccupancy = board.getOccupancy(2);

    // Vérifier les pièces non-pion via génération brute (pas de roque)
    for (PieceType type : {KING, QUEEN, ROOK, BISHOP, KNIGHT}) {
        uint64_t pieces = board.getPieces(attackerColor, type);
        while (pieces) {
            uint8_t pos = __builtin_ctzll(pieces);
            pieces &= pieces - 1;
            uint64_t attacks = m_generator.getPossibleMoves(type, attackerColor, pos, allOccupancy);
            if (attacks & (1ULL << square)) return true;
        }
    }

    // Vérifier les pions (attaques diagonales uniquement)
    uint64_t pawns = board.getPieces(attackerColor, PAWN);
    while (pawns) {
        uint8_t pos = __builtin_ctzll(pawns);
        pawns &= pawns - 1;
        uint64_t b = (1ULL << pos);
        uint64_t pawnAttacks = 0;
        if (attackerColor == WHITE) {
            if (pos % 8 != 0) pawnAttacks |= (b << 7);
            if (pos % 8 != 7) pawnAttacks |= (b << 9);
        } else {
            if (pos % 8 != 0) pawnAttacks |= (b >> 9);
            if (pos % 8 != 7) pawnAttacks |= (b >> 7);
        }
        if (pawnAttacks & (1ULL << square)) return true;
    }

    return false;
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

    // Captures: enemy pieces + en passant
    result.quietMoves   = forwardMoves;
    result.captureMoves = attackMask & enemyPieces;

    uint8_t enPassantTarget = board.getEnPassantTarget();
    if (enPassantTarget != ChessBitBoard::NO_EN_PASSANT) {
        uint64_t enPassantMask = (1ULL << enPassantTarget);
        if (attackMask & enPassantMask) {
            result.captureMoves |= enPassantMask;
        }
    }

    return result;
}

