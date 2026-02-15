#ifndef MOVECONTROLLER_H
#define MOVECONTROLLER_H

#include <cstdint>
#include "movegenerator.h"
#include "models/chessbitboard.h"

struct MoveMasks {
    uint64_t quietMoves = 0ULL;
    uint64_t captureMoves = 0ULL;
};

class MoveController
{
public:
    MoveController();

    MoveMasks getLegalMoves(uint8_t pos, const ChessBitBoard& board) const;
    MoveMasks getAllLegalMoves(Color color, const ChessBitBoard& board) const;
    MoveMasks getPawnMoves(uint8_t pos, const ChessBitBoard& board, Color color) const;
    MoveMasks getStandardMoves(uint8_t pos, const ChessBitBoard& board, PieceType type, Color color) const;

private:
    MoveGenerator m_generator;
};

#endif
