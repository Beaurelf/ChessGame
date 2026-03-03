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
    uint64_t m_table[2][7][64];
    uint64_t m_sideToMove;
};
#endif
