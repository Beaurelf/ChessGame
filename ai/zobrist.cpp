#include "ai/zobrist.h"
#include <random>


Zobrist::Zobrist() {
    std::mt19937_64 rng(0xDEADBEEFCAFE);
    for (int c = 0; c < 2; c++)
        for (int p = 0; p < 6; p++)
            for (int sq = 0; sq < 64; sq++)
                m_table[c][p][sq] = rng();

    for (uint64_t& key : m_castlingRights)
        key = rng();

    for (uint64_t& key : m_enPassantFiles)
        key = rng();

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

    h ^= m_castlingRights[board.getCastlingRights() & 0x0F];

    uint8_t enPassantTarget = board.getEnPassantTarget();
    if (enPassantTarget != ChessBitBoard::NO_EN_PASSANT) {
        bool canCaptureEnPassant = false;

        if (side == WHITE) {
            if (enPassantTarget >= 8) {
                if (enPassantTarget % 8 != 0) {
                    uint8_t from = static_cast<uint8_t>(enPassantTarget - 9);
                    canCaptureEnPassant = board.getPieceType(from) == PAWN && board.getPieceColor(from) == WHITE;
                }
                if (!canCaptureEnPassant && enPassantTarget % 8 != 7) {
                    uint8_t from = static_cast<uint8_t>(enPassantTarget - 7);
                    canCaptureEnPassant = board.getPieceType(from) == PAWN && board.getPieceColor(from) == WHITE;
                }
            }
        } else {
            if (enPassantTarget <= 55) {
                if (enPassantTarget % 8 != 0) {
                    uint8_t from = static_cast<uint8_t>(enPassantTarget + 7);
                    canCaptureEnPassant = board.getPieceType(from) == PAWN && board.getPieceColor(from) == BLACK;
                }
                if (!canCaptureEnPassant && enPassantTarget % 8 != 7) {
                    uint8_t from = static_cast<uint8_t>(enPassantTarget + 9);
                    canCaptureEnPassant = board.getPieceType(from) == PAWN && board.getPieceColor(from) == BLACK;
                }
            }
        }

        if (canCaptureEnPassant)
            h ^= m_enPassantFiles[enPassantTarget % 8];
    }

    if (side == BLACK) h ^= m_sideToMove;
    return h;
}
