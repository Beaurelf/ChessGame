#include "chessbitboard.h"

ChessBitBoard::ChessBitBoard() {
    m_castlingRights = CASTLE_WK | CASTLE_WQ | CASTLE_BK | CASTLE_BQ;
    m_enPassantTarget = NO_EN_PASSANT;
    setupDefaultBoardPieces();
}

uint8_t ChessBitBoard::getCastlingRights() const {
    return m_castlingRights;
}

uint8_t ChessBitBoard::getEnPassantTarget() const {
    return m_enPassantTarget;
}

bool ChessBitBoard::isEnPassantMove(uint8_t from, uint8_t to) const
{
    if (m_enPassantTarget == NO_EN_PASSANT || to != m_enPassantTarget) return false;
    if (getPieceType(from) != PAWN || getPieceType(to) != NONE) return false;

    int fileDelta = static_cast<int>(to % 8) - static_cast<int>(from % 8);
    return (fileDelta == 1 || fileDelta == -1);
}

uint8_t ChessBitBoard::getEnPassantCaptureSquare(uint8_t to, Color movingColor) const
{
    return (movingColor == WHITE)
               ? static_cast<uint8_t>(to - 8)
               : static_cast<uint8_t>(to + 8);
}

void ChessBitBoard::update(uint8_t from, uint8_t to)
{
    Color movingColor    = getPieceColor(from);
    PieceType movingType = getPieceType(from);
    Color enemyColor     = (movingColor == WHITE) ? BLACK : WHITE;

    uint8_t previousEnPassantSquare = m_enPassantTarget;
    bool isEnPassantCapture         = isEnPassantMove(from, to);
    uint8_t enPassantCaptureSquare  = NO_EN_PASSANT;
    PieceType capturedType          = getPieceType(to);
    Color capturedColor             = (capturedType != NONE || isEnPassantCapture) ? enemyColor : NO_COLOR;

    if (isEnPassantCapture) {
        capturedType = PAWN;
        enPassantCaptureSquare = getEnPassantCaptureSquare(to, movingColor);
    }

    // Save the state before the move
    m_undoStack.push_back({
        from, to,
        movingType, capturedType, NONE,
        movingColor, capturedColor,
        m_castlingRights,
        previousEnPassantSquare,
        enPassantCaptureSquare,
        isEnPassantCapture
    });

    m_enPassantTarget = NO_EN_PASSANT;

    if (capturedType != NONE) {
        uint8_t captureSquare = isEnPassantCapture ? enPassantCaptureSquare : to;
        m_pieces[enemyColor][capturedType] &= ~(1ULL << captureSquare);
    }

    uint64_t moveMask = (1ULL << from) | (1ULL << to);
    m_pieces[movingColor][movingType] ^= moveMask;

    if (movingType == PAWN) {
        uint8_t distance = (from > to) ? (from - to) : (to - from);
        if (distance == 16) {
            m_enPassantTarget = (movingColor == WHITE)
                                    ? static_cast<uint8_t>(from + 8)
                                    : static_cast<uint8_t>(from - 8);
        }
    }

    // Castling: move the rook
    uint8_t distance = (from > to) ? (from - to) : (to - from);
    if (movingType == KING && distance == 2) {
        uint8_t rookFrom, rookTo;
        if (to > from) {
            // Kingside castling
            rookFrom = (movingColor == WHITE) ? 7 : 63;
            rookTo   = (movingColor == WHITE) ? 5 : 61;
        } else {
            // Queenside castling
            rookFrom = (movingColor == WHITE) ? 0 : 56;
            rookTo   = (movingColor == WHITE) ? 3 : 59;
        }
        uint64_t rookMask = (1ULL << rookFrom) | (1ULL << rookTo);
        m_pieces[movingColor][ROOK] ^= rookMask;
    }

    // Update castling rights
    if (movingType == KING) {
        if (movingColor == WHITE) m_castlingRights &= ~(CASTLE_WK | CASTLE_WQ);
        else m_castlingRights &= ~(CASTLE_BK | CASTLE_BQ);
    }
    if (movingType == ROOK) {
        if (from == 0)  m_castlingRights &= ~CASTLE_WQ;
        if (from == 7)  m_castlingRights &= ~CASTLE_WK;
        if (from == 56) m_castlingRights &= ~CASTLE_BQ;
        if (from == 63) m_castlingRights &= ~CASTLE_BK;
    }
    // Captured rook
    if (to == 0)  m_castlingRights &= ~CASTLE_WQ;
    if (to == 7)  m_castlingRights &= ~CASTLE_WK;
    if (to == 56) m_castlingRights &= ~CASTLE_BQ;
    if (to == 63) m_castlingRights &= ~CASTLE_BK;

    updateOccupancies();
}

void ChessBitBoard::promotePawn(uint8_t pos, const PieceType& newType, const Color& color)
{
    m_pieces[color][PAWN]    &= ~(1ULL << pos);
    m_pieces[color][newType] |=  (1ULL << pos);

    // Mettre à jour le dernier UndoInfo avec la promotion
    if (!m_undoStack.empty())
        m_undoStack.back().promotedTo = newType;

    updateOccupancies();
}

void ChessBitBoard::undo()
{
    if (m_undoStack.empty()) return;

    UndoInfo info = m_undoStack.back();
    m_undoStack.pop_back();

    // Restore special-move state
    m_castlingRights = info.castlingRights;
    m_enPassantTarget = info.previousEnPassantSquare;

    if (info.promotedTo != NONE) {
        // Retirer la pièce promue
        m_pieces[info.movedColor][info.promotedTo] &= ~(1ULL << info.to);
        // Remettre le pion
        m_pieces[info.movedColor][PAWN] |= (1ULL << info.from);
    } else {
        uint64_t moveMask = (1ULL << info.from) | (1ULL << info.to);
        m_pieces[info.movedColor][info.movedType] ^= moveMask;
    }

    // Castling: restore the rook
    uint8_t distance = (info.from > info.to) ? (info.from - info.to) : (info.to - info.from);
    if (info.movedType == KING && distance == 2) {
        uint8_t rookFrom, rookTo;
        if (info.to > info.from) {
            // Kingside castling
            rookFrom = (info.movedColor == WHITE) ? 7 : 63;
            rookTo   = (info.movedColor == WHITE) ? 5 : 61;
        } else {
            // Queenside castling
            rookFrom = (info.movedColor == WHITE) ? 0 : 56;
            rookTo   = (info.movedColor == WHITE) ? 3 : 59;
        }
        uint64_t rookMask = (1ULL << rookFrom) | (1ULL << rookTo);
        m_pieces[info.movedColor][ROOK] ^= rookMask;
    }

    if (info.capturedType != NONE) {
        uint8_t captureSquare = info.isEnPassantCapture ? info.enPassantCaptureSquare : info.to;
        m_pieces[info.capturedColor][info.capturedType] |= (1ULL << captureSquare);
    }

    updateOccupancies();
}

uint64_t ChessBitBoard::getOccupancy(int index) const {
    return m_occupancies[index];
};

uint64_t ChessBitBoard::getPieces(const Color& color, const PieceType& type) const {
    return m_pieces[color][type];
}

bool ChessBitBoard::isOccupied(uint8_t pos) const {
    return m_occupancies[2] & (1ULL << pos);
}

bool ChessBitBoard::isOccupiedBy(uint8_t pos, const Color& color) const {
    return m_occupancies[color] & (1ULL << pos);
}

PieceType ChessBitBoard::getPieceType(uint8_t pos) const {
    if (!isOccupied(pos)) return PieceType::NONE;
    for (int type = 0; type < 6; ++type) {
        if ((m_pieces[0][type] | m_pieces[1][type]) & (1ULL << pos)) {
            return static_cast<PieceType>(type);
        }
    }
    return PieceType::NONE;
}

Color ChessBitBoard::getPieceColor(uint8_t pos) const {
    if (isOccupiedBy(pos, Color::WHITE)) return Color::WHITE;
    if (isOccupiedBy(pos, Color::BLACK)) return Color::BLACK;
    return Color::NO_COLOR;
}

void ChessBitBoard::updateOccupancies() {
    m_occupancies[0] = 0ULL;
    m_occupancies[1] = 0ULL;
    m_occupancies[2] = 0ULL;

    for (uint8_t type = 0; type < 6; ++type) {
        m_occupancies[0] |= m_pieces[0][type];
        m_occupancies[1] |= m_pieces[1][type];
    }
    m_occupancies[2] = m_occupancies[0] | m_occupancies[1];
}

uint8_t ChessBitBoard::getKingPosition(const Color& color) const
{
    uint64_t kingBB = m_pieces[color][PieceType::KING];
    return static_cast<uint8_t>(__builtin_ctzll(kingBB));
}

void ChessBitBoard::setupDefaultBoardPieces() {
    // BLANCS (WHITE)
    m_pieces[Color::WHITE][PieceType::PAWN]   = 0x000000000000FF00ULL; // Rangée 2 (a2-h2)
    m_pieces[Color::WHITE][PieceType::ROOK]   = 0x0000000000000081ULL; // a1, h1
    m_pieces[Color::WHITE][PieceType::KNIGHT] = 0x0000000000000042ULL; // b1, g1
    m_pieces[Color::WHITE][PieceType::BISHOP] = 0x0000000000000024ULL; // c1, f1
    m_pieces[Color::WHITE][PieceType::QUEEN]  = 0x0000000000000008ULL; // d1
    m_pieces[Color::WHITE][PieceType::KING]   = 0x0000000000000010ULL; // e1

    // NOIRS (BLACK)
    m_pieces[Color::BLACK][PieceType::PAWN]   = 0x00FF000000000000ULL; // Rangée 7 (a7-h7)
    m_pieces[Color::BLACK][PieceType::ROOK]   = 0x8100000000000000ULL; // a8, h8
    m_pieces[Color::BLACK][PieceType::KNIGHT] = 0x4200000000000000ULL; // b8, g8
    m_pieces[Color::BLACK][PieceType::BISHOP] = 0x2400000000000000ULL; // c8, f8
    m_pieces[Color::BLACK][PieceType::QUEEN]  = 0x0800000000000000ULL; // d8
    m_pieces[Color::BLACK][PieceType::KING]   = 0x1000000000000000ULL; // e8

    // Calculer les masques d'occupation globaux
    updateOccupancies();
}
