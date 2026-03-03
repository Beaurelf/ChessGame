#include "SoundController.h"
#include "controllers/consts.h"
#include <QUrl>
#include <QTimer>

SoundController::SoundController(QObject* parent) : QObject(parent) {
    QTimer::singleShot(100, this, &SoundController::loadSounds);
}

void SoundController::loadSounds() {
    // Créer les effets sonores
    for (auto it = ControllerConst::SOUND_FILES.begin(); it != ControllerConst::SOUND_FILES.end(); ++it) {
        QSoundEffect* sound = new QSoundEffect();
        sound->setSource(QUrl::fromLocalFile(it.value()));
        sound->setVolume(m_volume);
        m_sounds[it.key()] = sound;
    }
}

void SoundController::playSound(SoundType type) {
    if (m_muted) return;

    if (m_sounds.contains(type)) {
        m_sounds[type]->play();
    }
}

void SoundController::setVolume(qreal volume) {
    m_volume = qMin(1.0, qMax(0.0, volume));
    for (auto sound : std::as_const(m_sounds)) {
        sound->setVolume(m_volume);
    }
}

void SoundController::setMuted(bool muted) {
    m_muted = muted;
}
