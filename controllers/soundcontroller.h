#ifndef SOUNDCONTROLLER_H
#define SOUNDCONTROLLER_H

#include <QObject>
#include <QSoundEffect>
#include <QMap>

enum class SoundType {
    MOVE,
    CAPTURE,
    CHECK,
    CASTLE,
    PROMOTION,
    GAME_END,
};

class SoundController: public QObject
{   
public:
    explicit SoundController(QObject* parent = nullptr);
    void playSound(SoundType type);
    void extracted();
    void setVolume(qreal volume); // 0.0 à 1.0
    void setMuted(bool muted);

private:
    QMap<SoundType, QSoundEffect*> m_sounds;
    qreal m_volume = 0.7;
    bool m_muted = false;

    void loadSounds();
};

#endif // SOUNDCONTROLLER_H
