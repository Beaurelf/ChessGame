#include "ui/game.h"
#include "ui/consts.h"

Game::Game(QWidget *parent)
    : QMainWindow(parent), m_home(nullptr), m_chessBoard(nullptr), m_container(nullptr), m_machine(false)
{
    setup();
}

Game::~Game(){}


void Game::setup()
{
    this->setWindowIcon(QIcon(APP_ICON));
    this->setStyleSheet(
        "QPushButton#Controls{"
        "height: 50px; "
        "font-size: 20px; "
        "}"
        );
    this->setFixedSize(QSize(HOME_WIDTH, HOME_HEIGHT));

    m_home = make_unique<Home>();
    connect(m_home.get(), &Home::launchGame, this, &Game::launchGame);
    connect(m_home.get(), &Home::exit, this, &Game::exit);

    this->setCentralWidget(m_home.release());
}

void Game::goToHome()
{
    this->setFixedSize(QSize(HOME_WIDTH, HOME_HEIGHT));
    m_home = make_unique<Home>();
    connect(m_home.get(), &Home::launchGame, this, &Game::launchGame);
    connect(m_home.get(), &Home::exit, this, &Game::exit);
    this->setCentralWidget(m_home.release());
}

void Game::restart()
{
    auto dialog = std::make_unique<QDialog>(this);
    dialog->setWindowTitle("Confirmation");

    auto layout = std::make_unique<QVBoxLayout>();
    auto btnGroup = std::make_unique<QHBoxLayout>();
    auto label = std::make_unique<QLabel>("Êtes-vous sûr de vouloir commencer une nouvelle partie ?");

    auto btnYes = std::make_unique<QPushButton>("Oui");
    auto btnNo = std::make_unique<QPushButton>("Non");

    connect(btnYes.get(), &QPushButton::clicked, dialog.get(), &QDialog::accept);
    connect(btnNo.get(), &QPushButton::clicked, dialog.get(), &QDialog::reject);

    btnGroup->addWidget(btnYes.release());
    btnGroup->addWidget(btnNo.release());

    layout->addWidget(label.release());
    layout->addLayout(btnGroup.release());

    dialog->setLayout(layout.release());

    if (dialog->exec() == QDialog::Accepted) {
        launchGame(m_machine);
    }
}

void Game::launchGame(bool machine)
{
    m_machine = machine;
    m_chessBoard = make_unique<ChessBoard>(machine);
    connect(m_chessBoard.get(), &ChessBoard::restart, this, &Game::restart);
    connect(m_chessBoard.get(), &ChessBoard::goToHome, this, &Game::goToHome);

    m_container = make_unique<QWidget>();
    m_layout = make_unique<QHBoxLayout>();

    auto controlsLayout = make_unique<QVBoxLayout>();
    auto controlsWidget = make_unique<QWidget>();

    m_btnRestart = make_unique<QPushButton>("Nouvelle partie");
    m_btnHome = make_unique<QPushButton>("Menu principal");

    QPixmap cursor(CURSOR);
    cursor = cursor.scaled(QSize(CURSOR_SIZE, CURSOR_SIZE));

    m_btnRestart->setCursor(QCursor(cursor));
    m_btnHome->setCursor(QCursor(cursor));

    m_btnHome->setObjectName("Controls");
    m_btnRestart->setObjectName("Controls");

    connect(m_btnHome.get(), &QPushButton::clicked, this, &Game::goToHome);
    connect(m_btnRestart.get(), &QPushButton::clicked, this, &Game::restart);

    controlsLayout->setAlignment(Qt::AlignTop);
    controlsLayout->addWidget(m_btnHome.release());
    controlsLayout->addWidget(m_btnRestart.release());

    controlsWidget->setLayout(controlsLayout.release());
    controlsWidget->setFixedSize(QSize(CONTROLS_WIDTH, CONTROLS_HEIGHT));

    //layout->setAlignment(Qt::AlignLeft);
    m_layout->addWidget(controlsWidget.release());
    m_layout->addWidget(m_chessBoard.release());

    m_container->setLayout(m_layout.release());
    this->setFixedSize(QSize(GAME_WIDTH, GAME_HEIGHT));
    this->setCentralWidget(m_container.release());
}

void Game::exit()
{
    this->close();
}
