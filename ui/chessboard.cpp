#include "ui/chessboard.h"
#include "ui/helper.h"
#include <QGroupBox>
#include <QRadioButton>
#include <QButtonGroup>

ChessBoard::ChessBoard(bool machine, QMainWindow* parent) : m_parent(parent), m_machine(machine) {
    m_chessController = make_unique<ChessController>(machine, parent);
    setup();
}

ChessBoard::~ChessBoard(){}

void ChessBoard::setupUi() {
    m_view = make_unique<QGraphicsView>();
    m_scene = make_unique<QGraphicsScene>();
    m_capturedBlackLayout = make_unique<QVBoxLayout>();
    m_capturedWhiteLayout = make_unique<QVBoxLayout>();

    m_scene->setSceneRect(0, 0, UIConsts::BOARD_SIZE * UIConsts::BOARD_TILE_SIZE, UIConsts::BOARD_SIZE * UIConsts::BOARD_TILE_SIZE);
    m_view->setScene(m_scene.get());
    m_view->setAlignment(Qt::AlignCenter);

    auto chessBoardContainer = make_unique<QVBoxLayout>();
    auto ChessBoardLayout = make_unique<QHBoxLayout>();

    setupCapturedPiecesLayouts();
    initializeBoard();

    // Initialisation des muniteries
    QFont font("Century Gothic", 18);
    m_timer1 = make_unique<Timer>(10);
    m_timer2 = make_unique<Timer>(10);
    m_timer1->setFont(font);
    m_timer2->setFont(font);
    m_timer1->setAlignment(Qt::AlignLeft);
    m_timer2->setAlignment(Qt::AlignRight);

    ChessBoardLayout->addLayout(m_capturedBlackLayout.get());
    ChessBoardLayout->addWidget(m_view.get());
    ChessBoardLayout->addLayout(m_capturedWhiteLayout.get());

    chessBoardContainer->addWidget(m_timer2.get());
    chessBoardContainer->addLayout(ChessBoardLayout.release());
    chessBoardContainer->addWidget(m_timer1.get());

    setLayout(chessBoardContainer.release());
}
void ChessBoard::setup()
{
    setupUi();
    connect(m_timer1.get(), &Timer::timeout, this, [this](){
        this->onCheckMateDetected(Color::WHITE);
    });
    connect(m_timer2.get(), &Timer::timeout, this, [this](){
        this->onCheckMateDetected(Color::BLACK);
    });
    connect(m_chessController.get(), &ChessController::moveExecuted, this, &ChessBoard::onMoveExecuted);
    connect(m_chessController.get(), &ChessController::castleRookMoved, this, &ChessBoard::onCastleRookMoved);
    connect(m_chessController.get(), &ChessController::enPassantCapturePerformed, this, &ChessBoard::onEnPassantCapturePerformed);
    connect(m_chessController.get(), &ChessController::pieceCaptured, this, &ChessBoard::onPieceCaptured);
    connect(m_chessController.get(), &ChessController::checkMateDetected, this, &ChessBoard::onCheckMateDetected);
    connect(m_chessController.get(), &ChessController::drawDetected, this, &ChessBoard::onDrawDetected);
    connect(m_chessController.get(), &ChessController::promotionDetected, this, &ChessBoard::onPromotionDetected);
    connect(m_chessController.get(), &ChessController::pawnPromoted, this, &ChessBoard::onPawnPromoted);
    connect(this, &ChessBoard::moveRequested, m_chessController.get(), &ChessController::onMoveRequested);
    connect(this, &ChessBoard::promotionPieceSelected, m_chessController.get(), &ChessController::onPromotionPieceSelected);

    m_timer1->resume();
    m_isTimer1Active = true;
}

void ChessBoard::setupCapturedPiecesLayouts() {

    vector<QString> blackPiecesImages = {UIConsts::BLACK_PAWN, UIConsts::BLACK_KNIGHT, UIConsts::BLACK_BISHOP, UIConsts::BLACK_ROOK, UIConsts::BLACK_QUEEN};
    vector<QString> whitePiecesImages = {UIConsts::WHITE_PAWN, UIConsts::WHITE_KNIGHT, UIConsts::WHITE_BISHOP, UIConsts::WHITE_ROOK, UIConsts::WHITE_QUEEN};
    vector<PieceType> pieceTypes = {PAWN, KNIGHT, BISHOP, ROOK, QUEEN};

    for (size_t i = 0; i < blackPiecesImages.size(); ++i) {
        // --- Setup pour les pièces NOIRES capturées ---
        auto blackRow = make_unique<QHBoxLayout>();
        auto imgBlack = make_unique<QLabel>();

        m_capturedBlackLabels[pieceTypes[i]] = make_unique<QLabel>("0");

        imgBlack->setFixedSize(QSize(UIConsts::CAPTURED_PIECE_SIZE, UIConsts::CAPTURED_PIECE_SIZE));
        imgBlack->setScaledContents(true);
        imgBlack->setPixmap(QPixmap(blackPiecesImages[i]));

        blackRow->addWidget(imgBlack.release());
        blackRow->addWidget(m_capturedBlackLabels[pieceTypes[i]].get());
        blackRow->addStretch();

        // --- Setup pour les pièces BLANCHES capturées ---
        auto whiteRow = make_unique<QHBoxLayout>();
        auto imgWhite = make_unique<QLabel>();

        m_capturedWhiteLabels[pieceTypes[i]] = make_unique<QLabel>("0");

        imgWhite->setFixedSize(QSize(UIConsts::CAPTURED_PIECE_SIZE, UIConsts::CAPTURED_PIECE_SIZE));
        imgWhite->setScaledContents(true);
        imgWhite->setPixmap(QPixmap(whitePiecesImages[i]));

        whiteRow->addWidget(m_capturedWhiteLabels[pieceTypes[i]].get());
        whiteRow->addWidget(imgWhite.release());
        whiteRow->addStretch();

        m_capturedBlackLayout->addLayout(blackRow.release());
        m_capturedWhiteLayout->addLayout(whiteRow.release());
    }
}

void ChessBoard::initializeBoard() {
    for (uint8_t i = 0; i < 64; ++i) {
        uint8_t row = i / UIConsts::BOARD_SIZE;
        uint8_t col = i % UIConsts::BOARD_SIZE;
        QString cellColor = ((row + col) % 2 == 0) ? UIConsts::COLOR_WHITE : UIConsts::COLOR_GRAY;

        m_cells[i] = make_unique<CellItem>(i, cellColor);
        m_scene->addItem(m_cells[i].get());

        connect(m_cells[i].get(), &CellItem::cellPressed, this, &ChessBoard::onCellPressed);

        Color color = m_chessController->getPieceColor(i);
        if(color == Color::NO_COLOR) continue;

        PieceType type = m_chessController->getPieceType(i);
        addPieceToScene(i, type, color);
    }
}

void ChessBoard::addPieceToScene(uint8_t index, const PieceType& type, const Color& color) {
    m_pieceItems[index] = make_unique<PieceItem>(index, type, color);
    m_scene->addItem(m_pieceItems[index].get());
    connect(m_pieceItems[index].get(), &PieceItem::moveRequested, this, &ChessBoard::onMoveRequested);
    connect(m_pieceItems[index].get(), &PieceItem::piecePressed, this, &ChessBoard::onPiecePressed);
}

void ChessBoard::clearOldHighlights() {
    for (uint8_t pos : m_highlightedCellPositions) {
        if (m_cells[pos]) {
            m_cells[pos]->setHighlight(false);
        }
    }
    m_highlightedCellPositions.clear();
}

void ChessBoard::onCellPressed(uint8_t index){
    emit moveRequested(m_from, index);
}

void ChessBoard::onPiecePressed(uint8_t clickedPos){
    auto itValidMove = std::find(m_highlightedCellPositions.begin(), m_highlightedCellPositions.end(), clickedPos);
    if (itValidMove != m_highlightedCellPositions.end()) {
        emit moveRequested(m_from, clickedPos);
        return;
    }
    clearOldHighlights();
    m_from = clickedPos;
    MoveMasks moveMasks;
    auto it = m_legalMovesCache.find(clickedPos);
    if (it != m_legalMovesCache.end()) {
        moveMasks = m_legalMovesCache[clickedPos];
    } else {
        moveMasks = m_chessController->getCurrentPlayerLegalMoves(clickedPos);
        m_legalMovesCache[clickedPos] = moveMasks;
    }

    auto quietMovePos = UIHelper::getLegalMovePositions(moveMasks.quietMoves);
    auto captureMovePos = UIHelper::getLegalMovePositions(moveMasks.captureMoves);

    for(auto pos: quietMovePos){
        m_cells[pos]->setHighlight(true);
        m_highlightedCellPositions.push_back(pos);
    }

    for(auto pos: captureMovePos){
        m_cells[pos]->setHighlight(true, true);
        m_highlightedCellPositions.push_back(pos);
        m_capturedCellPositions.insert(pos);
    }
}

void ChessBoard::onMoveExecuted(uint8_t from, uint8_t to){
    m_legalMovesCache.clear(); // On vide le cache car le plateau a changé
    m_capturedCellPositions.clear();
    clearOldHighlights();
    if (m_pieceItems[to]) {
        m_scene->removeItem(m_pieceItems[to].get());
        m_pieceItems[to].reset();
    }
    m_pieceItems[to] = std::move(m_pieceItems[from]);
    if (m_pieceItems[to]) {
        m_pieceItems[to]->moveToCell(to);
    }
    m_pieceItems[from] = nullptr;
    // Mettre en surbrillance le déplacement
    for(auto& pos: m_lastMovePostions){
        m_cells[pos]->setHighlight(false);
    }
    m_lastMovePostions.clear();
    m_lastMovePostions.insert(to);
    m_lastMovePostions.insert(from);

    for(auto& pos: m_lastMovePostions){
        m_cells[pos]->setHighlight(true, false, true);
    }

    if(m_isTimer1Active){
        m_timer1->stop();
        m_timer2->resume();
        m_isTimer1Active = !m_isTimer1Active;
        return;
    }
    m_timer2->stop();
    m_timer1->resume();
    m_isTimer1Active = !m_isTimer1Active;
}

void ChessBoard::onMoveRequested(uint8_t to){
    emit moveRequested(m_from, to);
}

void ChessBoard::onCastleRookMoved(uint8_t rookFrom, uint8_t rookTo) {
    if (m_pieceItems[rookTo]) {
        m_scene->removeItem(m_pieceItems[rookTo].get());
        m_pieceItems[rookTo].reset();
    }
    m_pieceItems[rookTo] = std::move(m_pieceItems[rookFrom]);
    if (m_pieceItems[rookTo]) {
        m_pieceItems[rookTo]->moveToCell(rookTo);
    }
    m_pieceItems[rookFrom] = nullptr;
}

void ChessBoard::onEnPassantCapturePerformed(uint8_t capturedPos) {
    if (m_pieceItems[capturedPos]) {
        m_scene->removeItem(m_pieceItems[capturedPos].get());
        m_pieceItems[capturedPos].reset();
    }
}

void ChessBoard::onPieceCaptured(const PieceType& type, const Color& color) {
    auto& labels = (color == WHITE) ? m_capturedWhiteLabels : m_capturedBlackLabels;
    if (labels.find(type) == labels.end() || !labels[type]) return;
    int currentCount = labels[type]->text().toInt();
    labels[type]->setText(QString::number(currentCount + 1));
}

void ChessBoard::onCheckMateDetected(const Color& loserColor)
{
    m_timer1->stop();
    m_timer2->stop();
    QString winnerName = (loserColor == BLACK) ? "Blanc" : "Noir";

    QDialog dialog(this);
    dialog.setWindowTitle("Échec et Mat");

    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    QHBoxLayout* btnGroup = new QHBoxLayout();

    QLabel* label = new QLabel(QString("Le joueur %1 a remporté la partie !\nQue voulez-vous faire ?").arg(winnerName));
    label->setAlignment(Qt::AlignCenter);

    QPushButton* btnRestart = new QPushButton("Nouvelle partie");
    QPushButton* btnMenu = new QPushButton("Menu principal");

    connect(btnRestart, &QPushButton::clicked, &dialog, &QDialog::accept);
    connect(btnMenu, &QPushButton::clicked, &dialog, &QDialog::reject);

    btnGroup->addWidget(btnRestart);
    btnGroup->addWidget(btnMenu);

    layout->addWidget(label);
    layout->addLayout(btnGroup);

    if (dialog.exec() == QDialog::Accepted) {
        emit restart();
    } else {
        emit goToHome();
    }
}

void ChessBoard::onDrawDetected(const QString& reason)
{
    m_timer1->stop();
    m_timer2->stop();

    QDialog dialog(this);
    dialog.setWindowTitle("Partie nulle");

    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    QHBoxLayout* btnGroup = new QHBoxLayout();

    QLabel* label = new QLabel(QString("La partie est nulle.\nRaison : %1\n\nQue voulez-vous faire ?").arg(reason));
    label->setAlignment(Qt::AlignCenter);
    label->setWordWrap(true);

    QPushButton* btnRestart = new QPushButton("Nouvelle partie");
    QPushButton* btnMenu = new QPushButton("Menu principal");

    connect(btnRestart, &QPushButton::clicked, &dialog, &QDialog::accept);
    connect(btnMenu, &QPushButton::clicked, &dialog, &QDialog::reject);

    btnGroup->addWidget(btnRestart);
    btnGroup->addWidget(btnMenu);

    layout->addWidget(label);
    layout->addLayout(btnGroup);

    if (dialog.exec() == QDialog::Accepted) {
        emit restart();
    } else {
        emit goToHome();
    }
}

void ChessBoard::onPromotionDetected(uint8_t pos)
{
    auto fenetre = make_unique<QDialog>(this);
    fenetre->setWindowTitle("Promotion du Pion");
    fenetre->setFixedSize(UIConsts::PROMOTION_DIALOG_SIZE, UIConsts::PROMOTION_DIALOG_SIZE);

    // 2. Création des layouts et conteneurs
    auto mainLayout = make_unique<QVBoxLayout>();
    auto groupBox = make_unique<QGroupBox>("Faites votre choix");
    auto layoutChoice = make_unique<QVBoxLayout>();

    auto groupChoice = make_unique<QButtonGroup>(fenetre.get());

    auto radioQueen = make_unique<QRadioButton>("Reine");
    auto radioBitShop = make_unique<QRadioButton>("Fou");
    auto radioRook = make_unique<QRadioButton>("Tour");
    auto radioKnight = make_unique<QRadioButton>("Cavalier");

    radioQueen->setChecked(true);

    groupChoice->addButton(radioQueen.get(), 1);
    groupChoice->addButton(radioBitShop.get(), 2);
    groupChoice->addButton(radioRook.get(), 3);
    groupChoice->addButton(radioKnight.get(), 4);

    layoutChoice->addWidget(radioQueen.release());
    layoutChoice->addWidget(radioBitShop.release());
    layoutChoice->addWidget(radioRook.release());
    layoutChoice->addWidget(radioKnight.release());

    auto btnOk = make_unique<QPushButton>("Confirmer");
    connect(btnOk.get(), &QPushButton::clicked, fenetre.get(), &QDialog::accept);

    layoutChoice->addWidget(btnOk.release());

    groupBox->setLayout(layoutChoice.release());
    mainLayout->addWidget(groupBox.release());
    fenetre->setLayout(mainLayout.release());

    if (fenetre->exec() == QDialog::Accepted) {
        int choixId = groupChoice->checkedId();

        switch(choixId) {
        case 1: emit promotionPieceSelected(pos, QUEEN); break;
        case 2: emit promotionPieceSelected(pos, BISHOP); break;
        case 3: emit promotionPieceSelected(pos, ROOK); break;
        case 4: emit promotionPieceSelected(pos, KNIGHT); break;
        default: break;
        }
    }
}


void ChessBoard::onPawnPromoted(uint8_t pos, const PieceType& type, const Color& color)
{
    if (m_pieceItems[pos]) {
        m_scene->removeItem(m_pieceItems[pos].get());
        m_pieceItems[pos].reset();
    }
    addPieceToScene(pos, type, color);
}
