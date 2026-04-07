#ifndef CHESSAI_H
#define CHESSAI_H

#include "ai/transpositiontable.h"
#include "ai/zobrist.h"
#include "models/chessbitboard.h"
#include "ai/helper.h"
#include "type.h"
#include <unordered_map>

class ChessAI
{
public:
    explicit ChessAI(int depth, Color aiColor);
    AIHelper::Move getBestMove(ChessBitBoard& board);
    void setPositionHistory(const std::unordered_map<uint64_t, int>& positionHistory);

private:
    struct MinimaxResult {
        int          score;
        AIHelper::Move bestMove;
    };

    int   m_depth;
    Color m_aiColor;
    Zobrist m_zobrist;
    TranspositionTable m_tt;
    std::unordered_map<uint64_t, int> m_positionHistory;

    // ─────────────────────────────────────────────
    // MINIMAX + ALPHA-BETA
    // ─────────────────────────────────────────────
    MinimaxResult minimax(ChessBitBoard& board, int depth, int alpha, int beta, bool maximizing,
                          std::unordered_map<uint64_t, int>& repetitionCounts);

    int drawScore(const ChessBitBoard& board) const;
    int evaluateBoard(const ChessBitBoard& board) const;
    int quiescence(ChessBitBoard& board, int alpha, int beta, bool maximizing);

    // — Composantes de l'évaluation —
    int materialScore   (const ChessBitBoard& board, Color color) const;
    int pstScore        (const ChessBitBoard& board, Color color) const;
    int pawnStructScore (const ChessBitBoard& board, Color color) const;
    int gamePhase       (const ChessBitBoard& board)              const;
    int unstoppablePawnPenalty(const ChessBitBoard& board, Color color) const;

};

#endif // CHESSAI_H
