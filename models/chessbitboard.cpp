#include "chessbitboard.h"

ChessBitBoard::ChessBitBoard() {
    setupDefaultBoardPieces();
}

void ChessBitBoard::update(uint8_t from, uint8_t to) {
    Color movingColor = getPieceColor(from);
    PieceType movingType = getPieceType(from);

    // Si une pièce adverse est présente à l'arrivée, on la supprime
    PieceType capturedType = getPieceType(to);
    if (capturedType != NONE) {
        Color enemyColor = (movingColor == WHITE) ? BLACK : WHITE;
        m_pieces[enemyColor][capturedType] &= ~(1ULL << to);
    }

    // On déplace la pièce
    uint64_t moveMask = (1ULL << from) | (1ULL << to);
    m_pieces[movingColor][movingType] ^= moveMask;

    // Mettre à jour les masques d'occupation globaux
    updateOccupancies();
}

void ChessBitBoard::promotePawn(uint8_t pos, const PieceType& newType, const Color& color) {
    // On retire le Pion (on éteint le bit)
    m_pieces[color][PieceType::PAWN] &= ~(1ULL << pos);
    // On ajoute la nouvelle pièce
    m_pieces[color][newType] |= (1ULL << pos);
    // On met à jour les masques globaux
    updateOccupancies();
}

uint64_t ChessBitBoard::getOccupancy(int index) const {
    return m_occupancies[index];
};

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
    // --- BLANCS (WHITE) ---
    m_pieces[Color::WHITE][PieceType::PAWN]   = 0x000000000000FF00ULL; // Rangée 2 (a2-h2)
    m_pieces[Color::WHITE][PieceType::ROOK]   = 0x0000000000000081ULL; // a1, h1
    m_pieces[Color::WHITE][PieceType::KNIGHT] = 0x0000000000000042ULL; // b1, g1
    m_pieces[Color::WHITE][PieceType::BISHOP] = 0x0000000000000024ULL; // c1, f1
    m_pieces[Color::WHITE][PieceType::QUEEN]  = 0x0000000000000008ULL; // d1
    m_pieces[Color::WHITE][PieceType::KING]   = 0x0000000000000010ULL; // e1

    // --- NOIRS (BLACK) ---
    m_pieces[Color::BLACK][PieceType::PAWN]   = 0x00FF000000000000ULL; // Rangée 7 (a7-h7)
    m_pieces[Color::BLACK][PieceType::ROOK]   = 0x8100000000000000ULL; // a8, h8
    m_pieces[Color::BLACK][PieceType::KNIGHT] = 0x4200000000000000ULL; // b8, g8
    m_pieces[Color::BLACK][PieceType::BISHOP] = 0x2400000000000000ULL; // c8, f8
    m_pieces[Color::BLACK][PieceType::QUEEN]  = 0x0800000000000000ULL; // d8
    m_pieces[Color::BLACK][PieceType::KING]   = 0x1000000000000000ULL; // e8

    // Calculer les masques d'occupation globaux
    updateOccupancies();
}
