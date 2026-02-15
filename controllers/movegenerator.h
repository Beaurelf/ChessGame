#ifndef MOVEGENERATOR_H
#define MOVEGENERATOR_H

#include "type.h"
#include <cstdint>
#include <unordered_map>
#include <functional>

class MoveGenerator
{
public:
    MoveGenerator();

    uint64_t getPossibleMoves(PieceType type, Color color, uint8_t pos, uint64_t allOccupancy) const;

private:
    // Les 6 méthodes de génération
    uint64_t generatePawnMoves(uint8_t pos, Color color, uint64_t allOccupancy) const;
    uint64_t generateKnightMoves(uint8_t pos, Color color, uint64_t allOccupancy) const;
    uint64_t generateBishopMoves(uint8_t pos, Color color, uint64_t allOccupancy) const;
    uint64_t generateRookMoves(uint8_t pos, Color color, uint64_t allOccupancy) const;
    uint64_t generateQueenMoves(uint8_t pos, Color color, uint64_t allOccupancy) const;
    uint64_t generateKingMoves(uint8_t pos, Color color, uint64_t allOccupancy) const;

    // La map de stratégies de genération de mouvement
    std::unordered_map<PieceType, std::function<uint64_t(uint8_t, Color, uint64_t)>> m_moveStrategies;

};

#endif // MOVEGENERATOR_H
