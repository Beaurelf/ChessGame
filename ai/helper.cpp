#include "ai/helper.h"
#include "controllers/movecontroller.h"

namespace AIHelper {

std::vector<Move> generateAllMoves(ChessBitBoard& board, Color color)
{
    std::vector<Move> captures;
    std::vector<Move> quiets;
    MoveController mc;

    uint64_t occupancy = board.getOccupancy(color);
    while (occupancy) {
        uint8_t from = __builtin_ctzll(occupancy);
        occupancy &= occupancy - 1;

        // Pseudo-légaux
        MoveMasks pseudo = mc.getLegalMoves(from, board);

        // Filtrer — retirer les coups qui exposent le roi
        auto filterMask = [&](uint64_t mask) {
            uint64_t valid = 0ULL;
            while (mask) {
                uint8_t to = __builtin_ctzll(mask);
                mask &= mask - 1;

                board.update(from, to);

                // Vérifier que le roi de la couleur courante n'est pas en échec
                Color enemyColor = (color == WHITE) ? BLACK : WHITE;
                uint8_t kingPos  = board.getKingPosition(color);
                MoveMasks enemyMoves = mc.getAllLegalMoves(enemyColor, board);

                if (!(enemyMoves.captureMoves & (1ULL << kingPos)))
                    valid |= (1ULL << to);
                board.undo();
            }
            return valid;
        };

        uint64_t validCaptures = filterMask(pseudo.captureMoves);
        uint64_t validQuiets   = filterMask(pseudo.quietMoves);

        // Extraire captures
        uint64_t caps = validCaptures;
        while (caps) {
            uint8_t to = __builtin_ctzll(caps);
            caps &= caps - 1;
            captures.push_back({ from, to });
        }

        // Extraire quiets
        uint64_t q = validQuiets;
        while (q) {
            uint8_t to = __builtin_ctzll(q);
            q &= q - 1;
            quiets.push_back({ from, to });
        }
    }

    // MVV-LVA — capturer la pièce la plus précieuse en premier
    std::sort(captures.begin(), captures.end(), [&](const Move& a, const Move& b) {
        return AIConsts::PIECE_VALUE[board.getPieceType(a.to)]
               > AIConsts::PIECE_VALUE[board.getPieceType(b.to)];
    });

    captures.insert(captures.end(), quiets.begin(), quiets.end());
    return captures;
}
}

