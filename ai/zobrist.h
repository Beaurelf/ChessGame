// ai/zobrist.h
#ifndef ZOBRIST_H
#define ZOBRIST_H
#include <cstdint>
#include "type.h"
#include "models/chessbitboard.h"

class Zobrist {
public:
    Zobrist();
    uint64_t hash(const ChessBitBoard& board, Color side) const;

private:
    uint64_t m_table[2][6][64];
    uint64_t m_castlingRights[16];
    uint64_t m_enPassantFiles[8];
    uint64_t m_sideToMove;
};
#endif
