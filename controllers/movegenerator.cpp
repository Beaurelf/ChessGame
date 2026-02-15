#include "movegenerator.h"
#include "consts.h"

MoveGenerator::MoveGenerator() {
    m_moveStrategies[PieceType::PAWN]   = [this](uint8_t pos, Color color, uint64_t allOccupancy) { return generatePawnMoves(pos, color, allOccupancy); };
    m_moveStrategies[PieceType::KNIGHT] = [this](uint8_t pos, Color color, uint64_t allOccupancy) { return generateKnightMoves(pos, color, allOccupancy); };
    m_moveStrategies[PieceType::BISHOP] = [this](uint8_t pos, Color color, uint64_t allOccupancy) { return generateBishopMoves(pos, color, allOccupancy); };
    m_moveStrategies[PieceType::ROOK]   = [this](uint8_t pos, Color color, uint64_t allOccupancy) { return generateRookMoves(pos, color, allOccupancy); };
    m_moveStrategies[PieceType::QUEEN]  = [this](uint8_t pos, Color color, uint64_t allOccupancy) { return generateQueenMoves(pos, color, allOccupancy); };
    m_moveStrategies[PieceType::KING]   = [this](uint8_t pos, Color color, uint64_t allOccupancy) { return generateKingMoves(pos, color, allOccupancy); };
}

uint64_t MoveGenerator::getPossibleMoves(PieceType type, Color color, uint8_t pos, uint64_t allOccupancy) const {
    auto it = m_moveStrategies.find(type);
    if (it != m_moveStrategies.end()) {
        return it->second(pos, color, allOccupancy);
    }
    return 0ULL;
}

uint64_t MoveGenerator::generatePawnMoves(uint8_t pos, Color color, uint64_t /*allOccupancy*/) const {
    uint64_t moves = 0ULL;
    uint64_t b = (1ULL << pos);

    if (color == Color::WHITE) {
        // Avance
        moves |= (b << 8);
        if (b & RANK_2) moves |= (b << 16);

        // Attaques diagonales
        if (pos % 8 != 0) moves |= (b << 7);  // Nord-Ouest
        if (pos % 8 != 7) moves |= (b << 9);  // Nord-Est
    } else {
        // Avance
        moves |= (b >> 8);
        if (b & RANK_7) moves |= (b >> 16);

        // Attaques diagonales
        if (pos % 8 != 0) moves |= (b >> 9);  // Sud-Ouest
        if (pos % 8 != 7) moves |= (b >> 7);  // Sud-Est
    }

    return moves;
}

uint64_t MoveGenerator::generateKnightMoves(uint8_t pos, Color /*color*/, uint64_t /*allOccupancy*/) const {
    uint64_t moves = 0ULL;
    uint64_t b = (1ULL << pos);

    // --- Vers le HAUT ---
    // Up 2, Right 1 (+17) : On va à DROITE, donc interdit si on est sur la colonne H
    moves |= (b & ~FILE_H) << 17;
    // Up 2, Left 1 (+15) : On va à GAUCHE, donc interdit si on est sur la colonne A
    moves |= (b & ~FILE_A) << 15;
    // Up 1, Right 2 (+10) : On va 2x à DROITE, interdit si G ou H
    moves |= (b & ~(FILE_G | FILE_H)) << 10;
    // Up 1, Left 2 (+6) : On va 2x à GAUCHE, interdit si A ou B
    moves |= (b & ~(FILE_A | FILE_B)) << 6;


    // --- Vers le BAS ---
    // Down 2, Right 1 (-15) : On va à DROITE, interdit si colonne H
    // Note : 17 bas (gauche), 15 bas (droite)
    moves |= (b & ~FILE_H) >> 15;
    // Down 2, Left 1 (-17) : On va à GAUCHE, interdit si colonne A
    moves |= (b & ~FILE_A) >> 17;
    // Down 1, Right 2 (-6) : On va à DROITE, interdit si G ou H
    moves |= (b & ~(FILE_G | FILE_H)) >> 6;
    // Down 1, Left 2 (-10) : On va à GAUCHE, interdit si A ou B
    moves |= (b & ~(FILE_A | FILE_B)) >> 10;

    return moves;
}

uint64_t MoveGenerator::generateRookMoves(uint8_t pos, Color /*color*/, uint64_t allOccupancy) const {
    uint64_t moves = 0ULL;
    uint64_t rook = (1ULL << pos);

    // Nord (+8)
    uint64_t n = rook;
    while (n << 8) {
        n <<= 8;
        moves |= n;
        if (n & allOccupancy) break;  // bloqué
    }
    // Sud (-8)
    uint64_t s = rook;
    while (s >> 8) {
        s >>= 8;
        moves |= s;
        if (s & allOccupancy) break;
    }
    // Est (+1)
    uint64_t e = rook;
    while ((e & ~FILE_H) && (e << 1)) {
        e <<= 1;
        moves |= e;
        if (e & allOccupancy) break;
    }
    // Ouest (-1)
    uint64_t w = rook;
    while ((w & ~FILE_A) && (w >> 1)) {
        w >>= 1;
        moves |= w;
        if (w & allOccupancy) break;
    }

    return moves;
}

uint64_t MoveGenerator::generateBishopMoves(uint8_t pos, Color /*color*/, uint64_t allOccupancy) const {
    uint64_t moves = 0ULL;
    uint64_t bishop = (1ULL << pos);

    // Nord-Est (+9)
    uint64_t ne = bishop;
    while ((ne & ~FILE_H) && (ne << 9)) {
        ne <<= 9;
        moves |= ne;
        if (ne & allOccupancy) break;
    }
    // Sud-Ouest (-9)
    uint64_t sw = bishop;
    while ((sw & ~FILE_A) && (sw >> 9)) {
        sw >>= 9;
        moves |= sw;
        if (sw & allOccupancy) break;
    }
    // Nord-Ouest (+7)
    uint64_t nw = bishop;
    while ((nw & ~FILE_A) && (nw << 7)) {
        nw <<= 7;
        moves |= nw;
        if (nw & allOccupancy) break;
    }
    // Sud-Est (-7)
    uint64_t se = bishop;
    while ((se & ~FILE_H) && (se >> 7)) {
        se >>= 7;
        moves |= se;
        if (se & allOccupancy) break;
    }

    return moves;
}

uint64_t MoveGenerator::generateQueenMoves(uint8_t pos, Color color, uint64_t allOccupancy) const {
    return generateRookMoves(pos, color, allOccupancy)
    | generateBishopMoves(pos, color, allOccupancy);
}
uint64_t MoveGenerator::generateKingMoves(uint8_t pos, Color /*color*/, uint64_t /*allOccupancy*/) const {
    uint64_t moves = 0ULL;
    uint64_t b = (1ULL << pos);

    // --- Mouvements Verticaux ---
    moves |= (b << 8); // Nord
    moves |= (b >> 8); // Sud

    // --- Mouvements Latéraux ---
    moves |= (b & ~FILE_H) << 1; // Est (Droit)
    moves |= (b & ~FILE_A) >> 1; // Ouest (Gauche)

    // --- Diagonales ---
    moves |= (b & ~FILE_H) << 9; // Nord-Est
    moves |= (b & ~FILE_A) << 7; // Nord-Ouest
    moves |= (b & ~FILE_H) >> 7; // Sud-Est
    moves |= (b & ~FILE_A) >> 9; // Sud-Ouest

    return moves;
}
