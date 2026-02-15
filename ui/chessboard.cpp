#include "ui/chessboard.h"
#include "ui/consts.h"
#include "ui/helper.h"
#include <QGroupBox>
#include <QRadioButton>
#include <QButtonGroup>
#include <QDebug>

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

    m_scene->setSceneRect(0, 0, BOARD_SIZE * BOARD_TILE_SIZE, BOARD_SIZE * BOARD_TILE_SIZE);
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
    // sound_.setSource(QUrl(AUDIO_DEPLACEMENT));
    // sound_.setVolume(0.5f);
    connect(m_chessController.get(), &ChessController::moveExecuted, this, &ChessBoard::onMoveExecuted);
    connect(m_chessController.get(), &ChessController::pieceCaptured, this, &ChessBoard::onPieceCaptured);
    connect(m_chessController.get(), &ChessController::kingInCheck, this, &ChessBoard::onKingInCheck);
    connect(m_chessController.get(), &ChessController::checkMateDetected, this, &ChessBoard::onCheckMateDetected);
    connect(m_chessController.get(), &ChessController::promotionDetected, this, &ChessBoard::onPromotionDetected);
    connect(m_chessController.get(), &ChessController::pawnPromoted, this, &ChessBoard::onPawnPromoted);
    connect(this, &ChessBoard::moveRequested, m_chessController.get(), &ChessController::onMoveRequested);
    connect(this, &ChessBoard::promotionPieceSelected, m_chessController.get(), &ChessController::onPromotionPieceSelected);

    m_timer1->resume();
    m_isTimer1Active = true;
}

void ChessBoard::setupCapturedPiecesLayouts() {

    vector<QString> blackPiecesImages = {BLACK_PAWN, BLACK_KNIGHT, BLACK_BISHOP, BLACK_ROOK, BLACK_QUEEN};
    vector<QString> whitePiecesImages = {WHITE_PAWN, WHITE_KNIGHT, WHITE_BISHOP, WHITE_ROOK, WHITE_QUEEN};
    vector<PieceType> pieceTypes = {PAWN, KNIGHT, BISHOP, ROOK, QUEEN};

    for (size_t i = 0; i < blackPiecesImages.size(); ++i) {
        // --- Setup pour les pièces NOIRES capturées ---
        auto blackRow = make_unique<QHBoxLayout>();
        auto imgBlack = make_unique<QLabel>();

        m_capturedBlackLabels[pieceTypes[i]] = make_unique<QLabel>("0");

        imgBlack->setFixedSize(QSize(CAPTURED_PIECE_SIZE, CAPTURED_PIECE_SIZE));
        imgBlack->setScaledContents(true);
        imgBlack->setPixmap(QPixmap(blackPiecesImages[i]));

        blackRow->addWidget(imgBlack.release());
        blackRow->addWidget(m_capturedBlackLabels[pieceTypes[i]].get());
        blackRow->addStretch();

        // --- Setup pour les pièces BLANCHES capturées ---
        auto whiteRow = make_unique<QHBoxLayout>();
        auto imgWhite = make_unique<QLabel>();

        m_capturedWhiteLabels[pieceTypes[i]] = make_unique<QLabel>("0");

        imgWhite->setFixedSize(QSize(CAPTURED_PIECE_SIZE, CAPTURED_PIECE_SIZE));
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
        uint8_t row = i / BOARD_SIZE;
        uint8_t col = i % BOARD_SIZE;
        QString cellColor = ((row + col) % 2 == 0) ? COLOR_WHITE : COLOR_GRAY;

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
            m_cells[pos]->setHighlight(false, false);
        }
    }
    m_highlightedCellPositions.clear();
}

void ChessBoard::onCellPressed(uint8_t index){
    qDebug() << index;
    emit moveRequested(m_from, index);
}

void ChessBoard::onPiecePressed(uint8_t from){
    clearOldHighlights();
    m_from = from;
    MoveMasks moveMasks;
    auto it = m_legalMovesCache.find(from);
    if (it != m_legalMovesCache.end()) {
        moveMasks = m_legalMovesCache[from];
    } else {
        moveMasks = m_chessController->getLegalMoves(from);
        m_legalMovesCache[from] = moveMasks;
    }

    auto quietMovePos = UIHelper::getLegalMovePositions(moveMasks.quietMoves);
    auto captureMovePos = UIHelper::getLegalMovePositions(moveMasks.captureMoves);

    for(auto pos: quietMovePos){
        m_cells[pos]->setHighlight(true, false);
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
    qDebug() << "to: " << to << "\n";
    emit moveRequested(m_from, to);
}

void ChessBoard::onPieceCaptured(const PieceType& type, const Color& color) {
    auto& labels = (color == WHITE) ? m_capturedWhiteLabels : m_capturedBlackLabels;
    if (labels.find(type) == labels.end() || !labels[type]) return;
    int currentCount = labels[type]->text().toInt();
    labels[type]->setText(QString::number(currentCount + 1));
}

void ChessBoard::onKingInCheck(const Color& color)
{
    QString playerColor = (color == WHITE ? "blanc" : "noir");
    QMessageBox::warning(nullptr, "Échec", "Joueur " + playerColor + " votre roi est en échec !\n"
                                                                    "Veuillez déplacer une pièce vous permettant de sortir de l'échec.\n"
                                                                    "Aucune autre pièce que celles vous permettant de sortir de l'échec\n"
                                                                    "ne pourra être jouée.");
}

void ChessBoard::onCheckMateDetected(const Color& loserColor)
{
    QString winnerName = (loserColor == BLACK) ? "Blanc" : "Noir";

    auto dialog = make_unique<QDialog>(this);
    dialog->setWindowTitle("Échec et Mat");

    auto layout = make_unique<QVBoxLayout>();
    auto btnGroup = make_unique<QHBoxLayout>();

    auto label = make_unique<QLabel>(
        QString("Le joueur %1 a remporté la partie !\nQue voulez-vous faire ?").arg(winnerName)
    );
    label->setAlignment(Qt::AlignCenter);

    auto btnRestart = make_unique<QPushButton>("Nouvelle partie");
    auto btnMenu = make_unique<QPushButton>("Menu principal");

    connect(btnRestart.get(), &QPushButton::clicked, dialog.get(), &QDialog::accept);
    connect(btnMenu.get(), &QPushButton::clicked, dialog.get(), &QDialog::reject);

    btnGroup->addWidget(btnRestart.release());
    btnGroup->addWidget(btnMenu.release());

    layout->addWidget(label.release());
    layout->addLayout(btnGroup.release());

    dialog->setLayout(layout.release());
    if (dialog->exec() == QDialog::Accepted) {
        emit restart();
        return;
    }
    emit goToHome();
}


void ChessBoard::onPromotionDetected(uint8_t pos)
{
    auto fenetre = make_unique<QDialog>(this);
    fenetre->setWindowTitle("Promotion du Pion");
    fenetre->setFixedSize(PROMOTION_DIALOG_SIZE, PROMOTION_DIALOG_SIZE);

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
