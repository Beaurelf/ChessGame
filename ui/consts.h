#ifndef CONSTS_H
#define CONSTS_H

#include <QString>

// Colors
const QString COLOR_WHITE = "white";
const QString COLOR_BLACK = "black";
const QString COLOR_GRAY = "#AAAAAC";
const QString COLOR_RED = "#FA0033";
const QString COLOR_BEIGE = "#CCAD78";
const QString COLOR_BROWN = "#7E5430";

// White Pieces
const QString WHITE_KING = ":/images/img/roi_blanc.jpg";
const QString WHITE_QUEEN = ":/images/img/reine_blanc.jpg";
const QString WHITE_ROOK = ":/images/img/tour_blanc.jpg";
const QString WHITE_BISHOP = ":/images/img/fou_blanc.jpg";
const QString WHITE_KNIGHT = ":/images/img/chevalier_blanc.png";
const QString WHITE_PAWN = ":/images/img/pion_blanc.jpg";

// Black Pieces
const QString BLACK_KING = ":/images/img/roi_noir.jpg";
const QString BLACK_QUEEN = ":/images/img/reine_noir.jpg";
const QString BLACK_ROOK = ":/images/img/tour_noir.jpg";
const QString BLACK_BISHOP = ":/images/img/fou_noir.jpg";
const QString BLACK_KNIGHT = ":/images/img/chevalier_noir.jpg";
const QString BLACK_PAWN = ":/images/img/pion_noir.jpg";

// UI Assets
const QString MARKER = ":/images/img/marquer.jpg";
const QString CURSOR = ":/images/img/cursor.png";
const QString APP_ICON = ":/images/img/app_icon.png";
const QString MOVE_AUDIO = "qrc:/audios/audio/audio_deplacement.wav";

// Dimensions and Constraints
const int BOARD_TILE_SIZE = 80;
const int PIECE_ICON_SIZE = 60;
const int CURSOR_SIZE = 60;

const int HOME_HEIGHT = 680;
const int HOME_WIDTH = 680;

const int GAME_HEIGHT = 750;
const int GAME_WIDTH = 975;

const int CONTROLS_HEIGHT = 680;
const int CONTROLS_WIDTH = 200;

const int COLOR_COUNT = 8;
const int BOARD_SIZE = 8;
const int PIECE_COUNT = 8;

const int MIN_POSITION = 0;
const int MAX_POSITION = 8;

const int PROMOTION_DIALOG_SIZE = 200;
const int CAPTURED_PIECE_SIZE = 25;
const int HOME_CONSTRAINT = 150;

const int NB_CELLS = 64;


#endif // CONSTS_H
