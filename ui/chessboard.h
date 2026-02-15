#ifndef CHESSBOARD_H
#define CHESSBOARD_H

#include "controllers/chesscontroller.h"
#include "ui/Timer.h"
#include "ui/cellitem.h"
#include "ui/pieceitem.h"
#include <QGraphicsView>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMainWindow>
#include <QMessageBox>
#include <QPushButton>
#include <QWidget>
#include <vector>
#include <memory>

using namespace std;
typedef array<unique_ptr<CellItem>, NB_CELLS> Cells;


class ChessBoard: public QWidget
{
    Q_OBJECT

private:
    unique_ptr<QGraphicsView> m_view;
    unique_ptr<QGraphicsScene> m_scene;
    unique_ptr<Timer> m_timer1;
    unique_ptr<Timer> m_timer2;
    bool m_isTimer1Active;
    QMainWindow* m_parent;
    unique_ptr<ChessController> m_chessController;
    unique_ptr<QVBoxLayout> m_capturedBlackLayout;
    unique_ptr<QVBoxLayout> m_capturedWhiteLayout;
    unordered_map<PieceType, unique_ptr<QLabel>> m_capturedWhiteLabels;
    unordered_map<PieceType, unique_ptr<QLabel>> m_capturedBlackLabels;
    Cells m_cells;
    unordered_map<uint8_t, unique_ptr<PieceItem>> m_pieceItems;
    vector<uint8_t> m_highlightedCellPositions;
    set<uint8_t> m_capturedCellPositions;
    bool m_machine; // true si le joueur joue contre la machine
    // QSoundEffect sound_;
    unordered_map<uint8_t, MoveMasks> m_legalMovesCache;
    uint8_t m_from;

    void clearOldHighlights();
    void setupCapturedPiecesLayouts();
    void initializeBoard();
    void addPieceToScene(uint8_t index, const PieceType& type, const Color&  color);
    void setupUi();
    void setup();

public:
    ChessBoard(bool machine, QMainWindow *parent = nullptr);
    ~ChessBoard();

signals:
    void  restart();
    void goToHome();
    void moveRequested(uint8_t from, uint8_t to);
    void promotionPieceSelected(uint8_t pos, PieceType newType);

public slots:
    void onCellPressed(uint8_t index);
    void onPiecePressed(uint8_t from);
    void onMoveRequested(uint8_t to);
    void onMoveExecuted(uint8_t from, uint8_t to);
    void onPieceCaptured(const PieceType& type, const Color& color);
    void onKingInCheck(const Color& color);
    void onCheckMateDetected(const Color& color);
    void onPromotionDetected(uint8_t pos);
    void onPawnPromoted(uint8_t pos, const PieceType& type, const Color& color);
};

#endif // CHESSBOARD_H
