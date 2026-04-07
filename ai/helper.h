#ifndef AI_HELPER_H
#define AI_HELPER_H

#include "ai/consts.h"
#include "models/chessbitboard.h"
#include "type.h"
#include <cstdint>
#include <vector>

namespace AIHelper {

struct Move {
    uint8_t from;
    uint8_t to;
};

inline int popcount(uint64_t b) {
    return __builtin_popcountll(b);
}

inline int pstLookup(PieceType p, uint8_t pos, bool isEndgame) {
    switch (p) {
    case PAWN:   return AIConsts::PST_PAWN[pos];
    case KNIGHT: return AIConsts::PST_KNIGHT[pos];
    case BISHOP: return AIConsts::PST_BISHOP[pos];
    case ROOK:   return AIConsts::PST_ROOK[pos];
    case QUEEN:  return AIConsts::PST_QUEEN[pos];
    case KING:   return isEndgame ? AIConsts::PST_KING_EG[pos] : AIConsts::PST_KING_MG[pos];
    default:     return 0;
    }
}

std::vector<Move> generateAllMoves(ChessBitBoard& board, Color color);
}

#endif // HELPER_H
