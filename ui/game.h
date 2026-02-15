#ifndef GAME_H
#define GAME_H

#include "ui/chessboard.h"
#include "ui/home.h"
#include <QMainWindow>
#include <QPushButton>
#include <QHBoxLayout>
#include <memory>

using namespace std;

class Game : public QMainWindow
{
    Q_OBJECT
private:
    unique_ptr<Home> m_home;
    unique_ptr<ChessBoard> m_chessBoard;
    unique_ptr<QWidget> m_container;

    unique_ptr<QHBoxLayout> m_layout;
    unique_ptr<QPushButton> m_btnRestart;
    unique_ptr<QPushButton> m_btnHome;

    bool m_machine;

public:
    explicit Game(QWidget *parent = nullptr);
    ~Game();
    void setup();

public slots:
    void goToHome();
    void launchGame(bool machine);
    void restart();
    void exit();
};

#endif
