#ifndef CHESSAI_H
#define CHESSAI_H

#include "ai/transpositiontable.h"
#include "ai/zobrist.h"
#include "models/chessbitboard.h"
#include "ai/helper.h"
#include "type.h"

class ChessAI
{
public:
    explicit ChessAI(int depth, Color aiColor);
    AIHelper::Move getBestMove(const ChessBitBoard& board);

private:
    struct MinimaxResult {
        int          score;
        AIHelper::Move bestMove;
    };

    int   m_depth;
    Color m_aiColor;
    Zobrist m_zobrist;
    TranspositionTable m_tt;

    // ─────────────────────────────────────────────
    // MINIMAX + ALPHA-BETA
    // ─────────────────────────────────────────────
    MinimaxResult minimax(const ChessBitBoard& board, int depth, int alpha, int beta, bool maximizing);

    int evaluateBoard(const ChessBitBoard& board) const;
    int quiescence(const ChessBitBoard& board, int alpha, int beta, bool maximizing);

    // — Composantes de l'évaluation —
    int materialScore   (const ChessBitBoard& board, Color color) const;
    int pstScore        (const ChessBitBoard& board, Color color) const;
    int pawnStructScore (const ChessBitBoard& board, Color color) const;
    int gamePhase       (const ChessBitBoard& board)              const;
};

#endif // CHESSAI_H
