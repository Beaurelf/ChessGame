#include <QDebug>
#include "controllers/movecontroller.h"
#include "type.h"
#include <algorithm>
#include <cstdlib>
#include <limits>
#include "chessai.h"

ChessAI::ChessAI(int depth, Color aiColor) : m_depth(depth), m_aiColor(aiColor) {}

void ChessAI::setPositionHistory(const std::unordered_map<uint64_t, int>& positionHistory)
{
    m_positionHistory = positionHistory;
}

int ChessAI::drawScore(const ChessBitBoard& board) const
{
    int staticEval = evaluateBoard(board);

    if (staticEval > 120)  return -200 - std::min(staticEval / 8, 200);
    if (staticEval < -120) return  120 + std::min((-staticEval) / 8, 120);
    return 0;
}

// ─────────────────────────────────────────────
// MINIMAX + ALPHA-BETA
// ─────────────────────────────────────────────
ChessAI::MinimaxResult ChessAI::minimax(ChessBitBoard& board, int depth, int alpha, int beta, bool maximizing,
                                        std::unordered_map<uint64_t, int>& repetitionCounts)
{
    Color currentColor = maximizing ? m_aiColor : (m_aiColor == WHITE ? BLACK : WHITE);
    Color nextColor    = (currentColor == WHITE) ? BLACK : WHITE;
    uint64_t hash      = m_zobrist.hash(board, currentColor);

    int repetitionCount = 0;
    auto repetitionIt = repetitionCounts.find(hash);
    if (repetitionIt != repetitionCounts.end())
        repetitionCount = repetitionIt->second;

    if (repetitionCount >= 3 || board.isFiftyMoveRuleReached())
        return { drawScore(board), {255, 255} };

    const bool canUseTT = (repetitionCount <= 1);

    // 1. Probe TT
    TTEntry ttEntry;
    if (canUseTT && m_tt.probe(hash, depth, alpha, beta, ttEntry))
        return { ttEntry.score, ttEntry.bestMove };

    auto moves = AIHelper::generateAllMoves(board, currentColor);

    // 2. Cas terminal — aucun coup disponible
    if (moves.empty()) {
        MoveController mc;
        uint64_t kingBB = board.getPieces(currentColor, KING);

        if (kingBB && mc.isKingInCheck(currentColor, board)) {
            // Mat — score dépend de qui est maté + depth pour préférer mat rapide
            return { maximizing ? -100000 - depth : 100000 + depth, {255, 255} };
        }
        return { 0, {255, 255} };
    }

    // 3. Cas terminal — profondeur 0
    if (depth == 0)
        return { quiescence(board, alpha, beta, maximizing), {255, 255} };

    // 4. TT move ordering — mettre le meilleur move connu en premier
    AIHelper::Move ttMove = canUseTT ? m_tt.getBestMove(hash) : AIHelper::Move{255, 255};
    if (ttMove.from != 255) {
        auto it = std::find_if(moves.begin(), moves.end(), [&](const AIHelper::Move& m) {
            return m.from == ttMove.from && m.to == ttMove.to;
        });
        if (it != moves.end()) std::iter_swap(moves.begin(), it);
    }

    auto decrementRepetition = [&](uint64_t key) {
        auto it = repetitionCounts.find(key);
        if (it == repetitionCounts.end()) return;
        if (--it->second == 0) repetitionCounts.erase(it);
    };

    MinimaxResult best;
    best.bestMove = moves[0];
    TTFlag flag   = TTFlag::UPPER;

    if (maximizing) {
        best.score = std::numeric_limits<int>::min();
        for (const AIHelper::Move& move : moves) {
            board.update(move.from, move.to);
            uint64_t nextHash = m_zobrist.hash(board, nextColor);
            ++repetitionCounts[nextHash];

            int score = minimax(board, depth - 1, alpha, beta, false, repetitionCounts).score;

            decrementRepetition(nextHash);
            board.undo();
            if (score > best.score) {
                best.score    = score;
                best.bestMove = move;
                flag          = TTFlag::EXACT;
            }
            alpha = std::max(alpha, best.score);
            if (beta <= alpha) { flag = TTFlag::LOWER; break; }
        }
    } else {
        best.score = std::numeric_limits<int>::max();
        for (const AIHelper::Move& move : moves) {
            board.update(move.from, move.to);
            uint64_t nextHash = m_zobrist.hash(board, nextColor);
            ++repetitionCounts[nextHash];

            int score = minimax(board, depth - 1, alpha, beta, true, repetitionCounts).score;

            decrementRepetition(nextHash);
            board.undo();
            if (score < best.score) {
                best.score    = score;
                best.bestMove = move;
                flag          = TTFlag::EXACT;
            }
            beta = std::min(beta, best.score);
            if (beta <= alpha) { flag = TTFlag::UPPER; break; }
        }
    }

    // 5. Stocker dans TT
    if (canUseTT)
        m_tt.store(hash, depth, best.score, flag, best.bestMove);

    return best;
}

// ─────────────────────────────────────────────
// GET BEST MOVE — point d'entrée public
// ─────────────────────────────────────────────
AIHelper::Move ChessAI::getBestMove(ChessBitBoard& board)
{
    AIHelper::Move bestMove = {255, 255};
    std::unordered_map<uint64_t, int> repetitionCounts = m_positionHistory;

    uint64_t rootHash = m_zobrist.hash(board, m_aiColor);
    if (repetitionCounts.find(rootHash) == repetitionCounts.end())
        repetitionCounts[rootHash] = 1;

    for (int d = 1; d <= m_depth; d++) {
        auto result = minimax(board, d,
                              std::numeric_limits<int>::min(),
                              std::numeric_limits<int>::max(),
                              true,
                              repetitionCounts);
        if (result.bestMove.from != 255)
            bestMove = result.bestMove;
    }

    m_tt.clear(); // Vider APRÈS la recherche complète (avant la prochaine)
    return bestMove;
}

// ─────────────────────────────────────────────
// QUIESCENCE
// ─────────────────────────────────────────────
int ChessAI::quiescence(ChessBitBoard& board, int alpha, int beta, bool maximizing)
{
    int standPat = evaluateBoard(board);

    if (maximizing) {
        if (standPat >= beta) return beta;
        alpha = std::max(alpha, standPat);
    } else {
        if (standPat <= alpha) return alpha;
        beta = std::min(beta, standPat);
    }

    Color color = maximizing ? m_aiColor : (m_aiColor == WHITE ? BLACK : WHITE);
    auto moves  = AIHelper::generateAllMoves(board, color);

    // Aucun coup — vérifier mat ou pat
    if (moves.empty()) {
        MoveController mc;
        Color enemy     = (color == WHITE) ? BLACK : WHITE;
        uint64_t kingBB = board.getPieces(color, KING);

        if (kingBB && mc.isKingInCheck(color, board))
            return maximizing ? -100000 : 100000; // mat
        return 0; // pat
    }

    for (const AIHelper::Move& move : moves) {
        if (board.getPieceType(move.to) == NONE) continue; // captures seulement

        board.update(move.from, move.to);

        int score = quiescence(board, alpha, beta, !maximizing);
        board.undo();
        if (maximizing) {
            alpha = std::max(alpha, score);
            if (alpha >= beta) return beta;
        } else {
            beta = std::min(beta, score);
            if (beta <= alpha) return alpha;
        }
    }

    return maximizing ? alpha : beta;
}

// ─────────────────────────────────────────────
// GAME PHASE  (256 = opening, 0 = endgame)
// ─────────────────────────────────────────────
int ChessAI::gamePhase(const ChessBitBoard& board) const
{
    int phase = 0;
    phase += AIHelper::popcount(board.getPieces(WHITE, QUEEN)  | board.getPieces(BLACK, QUEEN))  * 4;
    phase += AIHelper::popcount(board.getPieces(WHITE, ROOK)   | board.getPieces(BLACK, ROOK))   * 2;
    phase += AIHelper::popcount(board.getPieces(WHITE, BISHOP) | board.getPieces(BLACK, BISHOP)) * 1;
    phase += AIHelper::popcount(board.getPieces(WHITE, KNIGHT) | board.getPieces(BLACK, KNIGHT)) * 1;
    return std::min(phase * 256 / 24, 256);
}

// ─────────────────────────────────────────────
// MATERIAL SCORE
// ─────────────────────────────────────────────
int ChessAI::materialScore(const ChessBitBoard& board, Color color) const
{
    int score = 0;
    for (PieceType p : {QUEEN, ROOK, BISHOP, KNIGHT, PAWN})
        score += AIHelper::popcount(board.getPieces(color, p)) * AIConsts::PIECE_VALUE[p];
    return score;
}

// ─────────────────────────────────────────────
// PST SCORE
// ─────────────────────────────────────────────
int ChessAI::pstScore(const ChessBitBoard& board, Color color) const
{
    int score = 0;
    bool endgame = (gamePhase(board) < 64);

    for (PieceType p : {PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING}) {
        uint64_t pieces = board.getPieces(color, p);
        while (pieces) {
            uint8_t pos = __builtin_ctzll(pieces);
            pieces &= pieces - 1;

            // Inverser l'index pour BLACK (les tables sont du point de vue WHITE)
            uint8_t idx = (color == BLACK) ? (63 - pos) : pos;
            score += AIHelper::pstLookup(p, idx, endgame);
        }
    }
    return score;
}

// ─────────────────────────────────────────────
// PAWN STRUCTURE
// ─────────────────────────────────────────────
int ChessAI::pawnStructScore(const ChessBitBoard& board, Color color) const
{
    int score = 0;
    uint64_t pawns    = board.getPieces(color, PAWN);
    uint64_t oppPawns = board.getPieces(color == WHITE ? BLACK : WHITE, PAWN);

    for (int col = 0; col < 8; col++) {
        uint64_t fileMask = 0x0101010101010101ULL << col;
        int count = AIHelper::popcount(pawns & fileMask);

        // Pion doublé
        if (count > 1) score -= 20 * (count - 1);

        // Pion isolé
        uint64_t adjMask = 0ULL;
        if (col > 0) adjMask |= 0x0101010101010101ULL << (col - 1);
        if (col < 7) adjMask |= 0x0101010101010101ULL << (col + 1);
        if ((pawns & fileMask) && !(pawns & adjMask))
            score -= 25 * count;

        // Pion passé
        uint64_t myPawnsOnFile = pawns & fileMask;
        while (myPawnsOnFile) {
            uint8_t sq = __builtin_ctzll(myPawnsOnFile);
            myPawnsOnFile &= myPawnsOnFile - 1;

            uint64_t aheadMask = 0ULL;
            int rank = sq / 8;
            if (color == WHITE) {
                for (int r = rank + 1; r < 8; r++)
                    aheadMask |= (fileMask | adjMask) & (0xFFULL << (r * 8));
            } else {
                for (int r = rank - 1; r >= 0; r--)
                    aheadMask |= (fileMask | adjMask) & (0xFFULL << (r * 8));
            }

            if (!(oppPawns & aheadMask)) {
                int advancement = (color == WHITE) ? rank : (7 - rank);

                // Bonus : la promotion imminente doit dominer l'éval sans ca l'IA ne prend pas la promotion en compte
                static const int PASSED_BONUS[8] = { 0, 10, 20, 40, 70, 120, 220, 0 };
                score += PASSED_BONUS[advancement];
            }
        }
    }
    return score;
}

int ChessAI::unstoppablePawnPenalty(const ChessBitBoard& board, Color color) const
{
    Color opp       = (color == WHITE) ? BLACK : WHITE;
    uint64_t pawns  = board.getPieces(color, PAWN);
    uint64_t oppKing = board.getPieces(opp, KING);
    if (!oppKing) return 0;

    int    kingPos  = __builtin_ctzll(oppKing);
    int    kingRow  = kingPos / 8, kingCol = kingPos % 8;
    int    penalty  = 0;

    uint64_t allOcc    = board.getOccupancy(2);
    uint64_t oppRooks  = board.getPieces(opp, ROOK);
    uint64_t oppQueens = board.getPieces(opp, QUEEN);
    uint64_t oppAll    = board.getOccupancy(opp == WHITE ? 0 : 1);

    while (pawns) {
        uint8_t sq  = __builtin_ctzll(pawns);
        pawns      &= pawns - 1;

        int row = sq / 8, col = sq % 8;
        int advancement = (color == WHITE) ? row : (7 - row);
        if (advancement < 4) continue; // Pas encore critique

        int promRow  = (color == WHITE) ? 7 : 0;
        int pawnDist = std::abs(row - promRow);

        // Vérifier si le chemin est physiquement bloqué
        uint64_t pathMask = 0;
        if (color == WHITE) {
            for (int r = row + 1; r <= promRow; ++r)
                pathMask |= (1ULL << (r * 8 + col));
        } else {
            for (int r = row - 1; r >= promRow; --r)
                pathMask |= (1ULL << (r * 8 + col));
        }
        if (allOcc & pathMask) continue; // Chemin bloqué, pas de bonus

        // 1) Règle du carré : le roi adverse peut-il rattraper ?
        int kingDist = std::max(std::abs(kingRow - promRow),
                                std::abs(kingCol - col));
        if (kingDist <= pawnDist + 1) continue;

        // 2) Tours/Dames sur la même colonne → interception triviale
        uint64_t fileMask = 0x0101010101010101ULL << col;
        if ((oppRooks | oppQueens) & fileMask) continue;

        // 3) Toute autre pièce adverse (hors roi) peut-elle intercepter ?
        //    Pour chaque case du chemin, si la distance de Chebyshev
        //    d'une pièce ≤ nombre de coups du pion pour y arriver,
        //    elle peut potentiellement bloquer/capturer.
        bool canIntercept = false;
        uint64_t oppPieces = oppAll & ~oppKing;
        while (oppPieces && !canIntercept) {
            uint8_t psq = __builtin_ctzll(oppPieces);
            oppPieces  &= oppPieces - 1;

            int pr = psq / 8, pc = psq % 8;

            if (color == WHITE) {
                for (int r = row + 1; r <= promRow; ++r) {
                    int pawnMoves = r - row;
                    int pieceDist = std::max(std::abs(pr - r), std::abs(pc - col));
                    if (pieceDist <= pawnMoves) {
                        canIntercept = true;
                        break;
                    }
                }
            } else {
                for (int r = row - 1; r >= promRow; --r) {
                    int pawnMoves = row - r;
                    int pieceDist = std::max(std::abs(pr - r), std::abs(pc - col));
                    if (pieceDist <= pawnMoves) {
                        canIntercept = true;
                        break;
                    }
                }
            }
        }

        if (!canIntercept)
            penalty += 600 + advancement * 50; // Quasi-promotion garantie
    }
    return penalty;
}

// ─────────────────────────────────────────────
// EVALUATE BOARD
// ─────────────────────────────────────────────
int ChessAI::evaluateBoard(const ChessBitBoard& board) const
{
    Color opp   = m_aiColor == WHITE ? BLACK : WHITE;
    int   score = 0;

    score += (materialScore  (board, m_aiColor) - materialScore  (board, opp)) * 10;
    score += (pstScore       (board, m_aiColor) - pstScore       (board, opp)) * 1;
    score += (pawnStructScore(board, m_aiColor) - pawnStructScore(board, opp)) * 1;
    score += unstoppablePawnPenalty(board, m_aiColor) - unstoppablePawnPenalty(board, opp);

    const int halfmoveClock = board.getHalfmoveClock();
    if (halfmoveClock > 80) {
        int drawPressure = (halfmoveClock - 80) * 6;
        if (score > 0) score -= drawPressure;
        else if (score < 0) score += drawPressure / 2;
    }
    return score;
}
