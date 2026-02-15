QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    controllers/chesscontroller.cpp \
    controllers/movecontroller.cpp \
    models/chessbitboard.cpp \
    ui/cellitem.cpp \
    ui/chessboard.cpp \
    main.cpp \
    ui/game.cpp \
    ui/home.cpp \
    controllers/movegenerator.cpp \
    ui/pieceitem.cpp \
    ui/timer.cpp

HEADERS += \
    controllers/chesscontroller.h \
    controllers/consts.h \
    controllers/movecontroller.h \
    models/chessbitboard.h \
    type.h \
    ui/cellitem.h \
    ui/chessboard.h \
    ui/consts.h \
    ui/game.h \
    ui/helper.h \
    ui/home.h \
    controllers/movegenerator.h \
    ui/pieceitem.h \
    ui/timer.h

RESOURCES += \
    resources.qrc

TRANSLATIONS += \
    ChessGame_fr_CA.ts
CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
