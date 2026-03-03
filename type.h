#ifndef TYPE_H
#define TYPE_H

enum PieceType {
    KING,
    QUEEN,
    ROOK,
    BISHOP,
    KNIGHT,
    PAWN,
    NONE
};

enum Color {
    WHITE,
    BLACK,
    NO_COLOR
};

enum class SoundType {
    MOVE,
    CAPTURE,
    CHECK,
    CASTLE,
    PROMOTION,
    GAME_END,
};

#endif // TYPE_H
