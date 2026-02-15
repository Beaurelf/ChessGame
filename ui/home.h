#ifndef HOME_H
#define HOME_H

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QMainWindow>
#include <QGraphicsEffect>
#include <QPushButton>
#include <QPixmap>


class Home : public QWidget
{
    Q_OBJECT

public:
    explicit Home(QMainWindow* parent = nullptr);
    ~Home();

signals:
    void launchGame(bool machine);
    void exit();
};
#endif // HOME_H
