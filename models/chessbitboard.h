#ifndef CHESSBITBOARD_H
#define CHESSBITBOARD_H

#include <cstdint>
#include <vector>
#include "type.h"

struct UndoInfo {
    uint8_t from;
    uint8_t to;
    PieceType movedType;
    PieceType capturedType;
    PieceType promotedTo;
    Color movedColor;
    Color capturedColor;
    uint8_t castlingRights;
    uint8_t previousEnPassantSquare;
    uint16_t previousHalfmoveClock;
    uint8_t enPassantCaptureSquare;
    bool isEnPassantCapture;
};

class ChessBitBoard
{
public:
    static constexpr uint8_t CASTLE_WK = 0x01;
    static constexpr uint8_t CASTLE_WQ = 0x02;
    static constexpr uint8_t CASTLE_BK = 0x04;
    static constexpr uint8_t CASTLE_BQ = 0x08;
    static constexpr uint8_t NO_EN_PASSANT = 0xFF;

    ChessBitBoard();

    uint8_t getCastlingRights() const;
    uint8_t getEnPassantTarget() const;
    uint16_t getHalfmoveClock() const;
    bool isFiftyMoveRuleReached() const;
    bool isEnPassantMove(uint8_t from, uint8_t to) const;
    uint8_t getEnPassantCaptureSquare(uint8_t to, Color movingColor) const;
    PieceType getPieceType(uint8_t pos) const;
    uint64_t getPieces(const Color& color, const PieceType& type) const;
    Color getPieceColor(uint8_t pos) const;
    uint64_t getOccupancy(int index) const;
    uint8_t getKingPosition(const Color& color) const;
    bool isOccupied(uint8_t pos) const;
    bool isOccupiedBy(uint8_t pos, const Color& color) const;
    void update(uint8_t from, uint8_t to);
    void undo();
    void promotePawn(uint8_t pos, const PieceType& newType, const Color& color);

private:
    // [0]=White, [1]=Black | [0..5]=KING, QUEEN, ROOK, BISHOP, KNIGHT, PAWN
    uint64_t m_pieces[2][6];
    // [0]=White, [1]=Black, [2]=Both
    uint64_t m_occupancies[3];
    uint8_t m_castlingRights;
    uint8_t m_enPassantTarget;
    uint16_t m_halfmoveClock;
    std::vector<UndoInfo> m_undoStack;
    void setupDefaultBoardPieces();
    void updateOccupancies();
};

#endif // CHESSBITBOARD_H
