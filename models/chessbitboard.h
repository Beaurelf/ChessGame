#ifndef CHESSBITBOARD_H
#define CHESSBITBOARD_H

#include <cstdint>
#include "type.h"

class ChessBitBoard
{
public:
    ChessBitBoard();

    PieceType getPieceType(uint8_t pos) const;
    Color getPieceColor(uint8_t pos) const;
    uint64_t getOccupancy(int index) const;
    uint8_t getKingPosition(const Color& color) const;
    bool isOccupied(uint8_t pos) const;
    bool isOccupiedBy(uint8_t pos, const Color& color) const;
    void update(uint8_t from, uint8_t to);
    void promotePawn(uint8_t pos, const PieceType& newType, const Color& color);

private:
    // [0]=White, [1]=Black | [0..5]=KING, QUEEN, ROOK, BISHOP, KNIGHT, PAWN
    uint64_t m_pieces[2][6];
    // [0]=White, [1]=Black, [2]=Both
    uint64_t m_occupancies[3];
    void setupDefaultBoardPieces();
    void updateOccupancies();
};

#endif // CHESSBITBOARD_H
