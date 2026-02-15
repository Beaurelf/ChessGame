#ifndef HELPER_H
#define HELPER_H

#include <QPointF>
#include <QString>
#include "type.h"
#include "ui/consts.h"

namespace UIHelper {

    /**
    * @brief Retourne le chemin de l'image (constante) selon le type et la couleur.
    */
    inline QString getPieceImagePath(PieceType type, Color color) {
        if (color == WHITE) {
            switch (type) {
            case KING:   return WHITE_KING;
            case QUEEN:  return WHITE_QUEEN;
            case ROOK:   return WHITE_ROOK;
            case BISHOP: return WHITE_BISHOP;
            case KNIGHT: return WHITE_KNIGHT;
            case PAWN:   return WHITE_PAWN;
            default:     return "";
            }
            return "";
        }
        switch (type) {
        case KING:   return BLACK_KING;
        case QUEEN:  return BLACK_QUEEN;
        case ROOK:   return BLACK_ROOK;
        case BISHOP: return BLACK_BISHOP;
        case KNIGHT: return BLACK_KNIGHT;
        case PAWN:   return BLACK_PAWN;
        default:     return "";

        }
    }

    /**
     * @brief Convertit un index Bitboard (0-63) en coordonnées pixels QPointF.
     */
    inline QPointF getCoordinate(uint8_t pos) {
        uint8_t col = pos % BOARD_SIZE;
        uint8_t row = pos / BOARD_SIZE;

        // Calcul des pixels : Y inversé pour que Row 0 soit en bas
        qreal x = col * BOARD_TILE_SIZE;
        qreal y = (BOARD_SIZE - 1 - row) * BOARD_TILE_SIZE;

        return QPointF(x, y);
    }

    /**
     * @brief Convertit des coordonnées pixels QPointF en index Bitboard (0-63).
     */
    inline uint8_t getPosition(QPointF p) {
        int col = static_cast<int>(p.x() / BOARD_TILE_SIZE);
        int visualRow = static_cast<int>(p.y() / BOARD_TILE_SIZE);

        // On inverse l'axe Y : la ligne visuelle 0 (en haut) est la ligne 7 du Bitboard
        int row = (BOARD_SIZE - 1) - visualRow;

        if (col < 0) col = 0;
        if (col >= BOARD_SIZE) col = BOARD_SIZE - 1;
        if (row < 0) row = 0;
        if (row >= BOARD_SIZE) row = BOARD_SIZE - 1;

        return static_cast<uint8_t>(row * BOARD_SIZE + col);
    }

    /**
     * @brief Extrait les indices (0-63) de tous les bits à 1 dans un masque.
     * @param moves le masque binaire de movement (ex: quietMoves ou captureMoves).
     * @return Un vecteur contenant les positions des cases.
     */
    inline std::vector<uint8_t> getLegalMovePositions(uint64_t moves) {
        std::vector<uint8_t> positions;
        while(moves)
        {
            positions.push_back(__builtin_ctzll(moves));
            moves &= (moves - 1);
        }
        return positions;
    }
}

#endif // HELPER_H
