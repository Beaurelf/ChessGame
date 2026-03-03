#ifndef CONTROLLER_CONSTS_H
#define CONTROLLER_CONSTS_H

#include <cstdint>
#include <QString>
#include "type.h"
#include <QMap>

namespace ControllerConst{

constexpr uint64_t RANK_1 = 0x00000000000000FFULL;
constexpr uint64_t RANK_2 = 0x000000000000FF00ULL;
constexpr uint64_t RANK_3 = 0x0000000000FF0000ULL;
constexpr uint64_t RANK_4 = 0x00000000FF000000ULL;
constexpr uint64_t RANK_5 = 0x000000FF00000000ULL;
constexpr uint64_t RANK_6 = 0x0000FF0000000000ULL;
constexpr uint64_t RANK_7 = 0x00FF000000000000ULL;
constexpr uint64_t RANK_8 = 0xFF00000000000000ULL;

constexpr uint64_t FILE_A = 0x0101010101010101ULL;
constexpr uint64_t FILE_B = 0x0202020202020202ULL;
constexpr uint64_t FILE_C = 0x0404040404040404ULL;
constexpr uint64_t FILE_D = 0x0808080808080808ULL;
constexpr uint64_t FILE_E = 0x1010101010101010ULL;
constexpr uint64_t FILE_F = 0x2020202020202020ULL;
constexpr uint64_t FILE_G = 0x4040404040404040ULL;
constexpr uint64_t FILE_H = 0x8080808080808080ULL;

const QString MOVE_AUDIO = ":/audios/audio/deplacement.wav";
const QString CAPTURE_AUDIO = ":/audios/audio/capture.wav";
const QString ENDING_AUDIO = ":/audios/audio/ending.wav";
const QString PROMOTION_AUDIO = ":/audios/audio/promotion.wav";
const QString CHECK_AUDIO = ":/audios/audio/echec.wav";

inline const QMap<SoundType, QString> SOUND_FILES = {
    {SoundType::MOVE,        MOVE_AUDIO},
    {SoundType::CAPTURE,     CAPTURE_AUDIO},
    {SoundType::CHECK,       CHECK_AUDIO},
    {SoundType::CASTLE,      MOVE_AUDIO},
    {SoundType::PROMOTION,   PROMOTION_AUDIO},
    {SoundType::GAME_END,    ENDING_AUDIO},
};

}

#endif // CONSTS_H
