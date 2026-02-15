#include "Timer.h"
#include "qdebug.h"
#include <iomanip>
#include <sstream>

Timer::Timer(int minute, int seconde, QWidget *obj): QLabel(obj), m_minutes(minute), m_seconds(seconde){
    m_timer = std::make_unique<QTimer>(this);
    setText(toString());
    connect(m_timer.get(), &QTimer::timeout, this, &Timer::update);
}

Timer::~Timer() {}

void Timer::update(){
    m_seconds--;
    if(m_seconds == -1){
        m_minutes--;
        m_seconds = 59;
        if(m_minutes == -1){
            stop();
            emit timeout();
        }
    }
    if(m_minutes >= 0) {
        setText(toString());
    }
}

void Timer::stop() {
    m_timer->stop();
}

void Timer::resume(){
    m_timer->start(1000);
}

QString Timer::toString(){
    std::ostringstream oss;
    oss << std::setw(2) << std::setfill('0') << m_minutes
        << ":"
        << std::setw(2) << std::setfill('0') << m_seconds;
    return QString::fromStdString(oss.str());
}
