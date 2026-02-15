#ifndef TIMER_H
#define TIMER_H

#include <QLabel>
#include <QTimer>
#include <memory>

class Timer : public QLabel
{
    Q_OBJECT
public:
    Timer(int minutes, int second = 0, QWidget* obj = nullptr);
    ~Timer();
    void stop();
    void resume();
    QString toString();

public slots:
    void update();

signals:
    void timeout();

private:
    int m_minutes;
    int m_seconds;
    std::unique_ptr<QTimer> m_timer;
};


#endif // TIMER_H
