#include "ai/zobrist.h"
#include <random>


Zobrist::Zobrist() {
    std::mt19937_64 rng(0xDEADBEEFCAFE);
    for (int c = 0; c < 2; c++)
        for (int p = 0; p < 6; p++)
            for (int sq = 0; sq < 64; sq++)
                m_table[c][p][sq] = rng();
    m_sideToMove = rng();
}

uint64_t Zobrist::hash(const ChessBitBoard& board, Color side) const {
    uint64_t h = 0;
    for (int c = 0; c < 2; c++) {
        Color color = c == 0 ? WHITE : BLACK;
        for (int p = 0; p < 6; p++) {
            uint64_t pieces = board.getPieces(color, (PieceType)p);
            while (pieces) {
                uint8_t sq = __builtin_ctzll(pieces);
                pieces    &= pieces - 1;
                h         ^= m_table[c][p][sq];
            }
        }
    }
    if (side == BLACK) h ^= m_sideToMove;
    return h;
}
