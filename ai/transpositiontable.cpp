#include "transpositiontable.h"

 TranspositionTable::TranspositionTable(size_t sizeMB) {
    size_t entries = (sizeMB * 1024 * 1024) / sizeof(TTEntry);
    m_table.resize(entries);
    m_size = entries;
}

void TranspositionTable::store(uint64_t hash, int depth, int score, TTFlag flag, AIHelper::Move move) {
    TTEntry& e = m_table[hash % m_size];
    // Remplacer seulement si profondeur plus grande
    if (e.flag == TTFlag::EMPTY || depth >= e.depth) {
        e = { hash, score, depth, flag, move };
    }
}

bool TranspositionTable::probe(uint64_t hash, int depth, int alpha, int beta, TTEntry& out) const {
    const TTEntry& e = m_table[hash % m_size];
    if (e.flag == TTFlag::EMPTY || e.hash != hash) return false;
    if (e.depth < depth) return false;

    out = e;
    if (e.flag == TTFlag::EXACT)                    return true;
    if (e.flag == TTFlag::LOWER && e.score >= beta) return true;
    if (e.flag == TTFlag::UPPER && e.score <= alpha) return true;
    return false;
}

AIHelper::Move TranspositionTable::getBestMove(uint64_t hash) const {
    const TTEntry& e = m_table[hash % m_size];
    if (e.hash == hash && e.flag != TTFlag::EMPTY)
        return e.bestMove;
    return {255, 255};
}

void TranspositionTable::clear() { std::fill(m_table.begin(), m_table.end(), TTEntry{}); }


