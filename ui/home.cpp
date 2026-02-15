#include "ui/home.h"
#include "ui/consts.h"
Home::Home(QMainWindow *parent) :
    QWidget(parent)
{

    QWidget* widget = new QWidget;
    QWidget* opacityWidget = new QWidget;

    widget->setObjectName("Home");

    QGraphicsOpacityEffect* opacityEffect = new QGraphicsOpacityEffect;

    //effect->setOpacity(100);

    QVBoxLayout* container = new QVBoxLayout;
    QVBoxLayout* layout = new QVBoxLayout;
    QVBoxLayout* opacityLayout = new QVBoxLayout;

    opacityWidget->setObjectName("OpacityWidget");

    QLabel* welcome = new QLabel("Chess");
    welcome->setAlignment(Qt::AlignCenter);
    QPushButton* btn1 = new QPushButton("J1 VS CPU");
    QPushButton* btn2 = new QPushButton("J1 VS J2");
    QPushButton* btnExit = new QPushButton("Quitter");

    QPixmap cursor(CURSOR);
    cursor = cursor.scaled(QSize(CURSOR_SIZE, CURSOR_SIZE));

    btn1->setCursor(QCursor(cursor));
    btn2->setCursor(QCursor(cursor));
    btnExit->setCursor(QCursor(cursor));

    connect(btn1 , &QPushButton::clicked,[this]{ emit launchGame(true);});
    connect(btn2 , &QPushButton::clicked,[this]{ emit launchGame(false);});
    connect(btnExit , &QPushButton::clicked,[this]{ emit exit();});

    layout->addWidget(welcome);
    layout->addWidget(btn1);
    layout->addWidget(btn2);
    layout->addWidget(btnExit);

    opacityLayout->addWidget(opacityWidget, Qt::AlignCenter);
    opacityWidget->setLayout(layout);
    opacityWidget->setGraphicsEffect(opacityEffect);

    widget->setLayout(opacityLayout);
    widget->setContentsMargins(HOME_CONSTRAINT, HOME_CONSTRAINT, HOME_CONSTRAINT, HOME_CONSTRAINT);

    container->addWidget(widget, Qt::AlignCenter);

    setStyleSheet(
        "QWidget#Home{"
        "background-image: url(:/images/img/fond.png); "
        "background-repeat: no-repeat; "
        "background-position: center;"
        "}"
        "QWidget#OpacityWidget{"
        "background: rgba(0, 0, 0, 0.75); "
        "border: 1px solid rgba(255, 255, 255, 0.35);"
        "border-radius: 20px; "
        "}"
        "QLabel{"
        "font-family: Century Gothic; "
        "font-weight: bold; "
        "font-size: 55px; "
        "}"
        "QPushButton{"
        "height: 50px; "
        "font-size: 20px; "
        "}"
        );
    this->setLayout(container);
}

Home::~Home(){}
