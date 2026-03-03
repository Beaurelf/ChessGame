// ai/transpositiontable.h
#ifndef TRANSPOSITIONTABLE_H
#define TRANSPOSITIONTABLE_H
#include <cstdint>
#include <vector>
#include "ai/helper.h"

enum class TTFlag : uint8_t { EMPTY, EXACT, LOWER, UPPER };

struct TTEntry {
    uint64_t       hash     = 0;
    int            score    = 0;
    int            depth    = 0;
    TTFlag         flag     = TTFlag::EMPTY;
    AIHelper::Move bestMove = {255, 255};
};

class TranspositionTable {
public:
    explicit TranspositionTable(size_t sizeMB = 64);

    void store(uint64_t hash, int depth, int score, TTFlag flag, AIHelper::Move move);

    bool probe(uint64_t hash, int depth, int alpha, int beta, TTEntry& out) const;

    AIHelper::Move getBestMove(uint64_t hash) const;

    void clear();

private:
    std::vector<TTEntry> m_table;
    size_t               m_size;
};
#endif
