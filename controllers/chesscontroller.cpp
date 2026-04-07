#include "chesscontroller.h"
#include "controllers/consts.h"
#include <QtConcurrent>


ChessController::ChessController(bool machine, QObject *parent) :
    QObject(parent),
    m_moveController(),
    m_machine(machine),
    m_currentPlayer(WHITE),
    m_chessAi(7, m_currentPlayer == WHITE ? BLACK : WHITE)
{}

PieceType ChessController::getPieceType(uint8_t pos) const
{
    return m_bitBoard.getPieceType(pos);
}

Color ChessController::getPieceColor(uint8_t pos) const
{
    return m_bitBoard.getPieceColor(pos);
}

MoveMasks ChessController::getLegalMoves(uint8_t from) {
    MoveMasks finalMoves = {};

    // Récupérer les coups "Pseudo-Légaux" (Géométrie + Obstacles)
    // Ce sont les coups possibles physiquement sans se soucier de la sécurité du roi
    MoveMasks pseudoMoves = m_moveController.getLegalMoves(from, m_bitBoard);

    auto filterMask = [&](uint64_t mask) -> uint64_t {
        uint64_t validMask = 0ULL;

        while (mask) {
            // On récupère l'index du prochain coup à tester (bit de poids faible)
            uint8_t to = __builtin_ctzll(mask);

            // --- SIMULATION ---
            // On joue le coup sur la copie
            m_bitBoard.update(from, to);
            // Si mon Roi survit sur ce plateau simulé, le coup est valide
            if (!isKingInCheck(m_currentPlayer, m_bitBoard)) {
                validMask |= (1ULL << to);
            }
            // on fait un undo
            m_bitBoard.undo();
            // On supprime le bit traité pour passer au suivant
            mask &= (mask - 1);
        }
        return validMask;
    };

    finalMoves.quietMoves = filterMask(pseudoMoves.quietMoves);
    finalMoves.captureMoves = filterMask(pseudoMoves.captureMoves);
    return finalMoves;
}

MoveMasks ChessController::getCurrentPlayerLegalMoves(uint8_t from) {
    if (m_isAiTurn || !isCurrentPlayerPiece(from)) return {};
    return getLegalMoves(from);
}

bool ChessController::isValidMove(uint8_t from, uint8_t to)
{
    MoveMasks moveMasks = getLegalMoves(from);
    return ((1ULL << to) & (moveMasks.quietMoves | moveMasks.captureMoves)) != 0;
}

bool ChessController::isCurrentPlayerPiece(uint8_t pos) const
{
    return m_currentPlayer == m_bitBoard.getPieceColor(pos);
}

bool ChessController::isKingInCheck(Color color, const ChessBitBoard& board) const {
    Color enemyColor = (color == WHITE) ? BLACK : WHITE;
    uint8_t myKingPos = board.getKingPosition(color);
    MoveMasks enemyMoves = m_moveController.getAllLegalMoves(enemyColor, board);
    return (enemyMoves.captureMoves & (1ULL << myKingPos)) != 0;
}

bool ChessController::isCheckmate(Color color, const ChessBitBoard& board) {
    if (!isKingInCheck(color, board)) {
        return false;
    }

    uint64_t myPieces = board.getOccupancy(color);
    while (myPieces) {
        uint8_t pos = __builtin_ctzll(myPieces);
        MoveMasks moves = getLegalMoves(pos);
        if (moves.quietMoves || moves.captureMoves) {
            return false;
        }
        // On supprime le bit traité pour passer à la pièce suivante
        myPieces &= (myPieces - 1);
    }
    // Si on a testé toutes les pièces et qu'aucune ne peut bouger alors qu'on est en échec et mat ...
    return true;
}

void ChessController::onPromotionPieceSelected(uint8_t pos, PieceType newType) {
    m_bitBoard.promotePawn(pos, newType, m_currentPlayer);
    emit pawnPromoted(pos, newType, m_currentPlayer);
    endTurn();
    if (isCheckmate(m_currentPlayer, m_bitBoard)) {
        m_soundController.playSound(SoundType::GAME_END);
        emit checkMateDetected(m_currentPlayer);
    } else if (isKingInCheck(m_currentPlayer, m_bitBoard)) {
        m_soundController.playSound(SoundType::CHECK);
    } else {
        m_soundController.playSound(SoundType::PROMOTION);
    }
    if (m_isAiTurn) {
        QTimer::singleShot(300, this, &ChessController::handleAiTurnIfNeeded);
    }
}

void ChessController::onMoveRequested(uint8_t from, uint8_t to)
{
    if(m_isAiTurn || !isCurrentPlayerPiece(from)) return;
    if(!isValidMove(from, to)) return;

    PieceType capturedType = m_bitBoard.getPieceType(to);
    PieceType movingType = m_bitBoard.getPieceType(from);
    Color capturedColor = (m_currentPlayer == WHITE) ? BLACK : WHITE;
    bool isEnPassantCapture = m_bitBoard.isEnPassantMove(from, to);
    uint8_t enPassantCaptureSquare = ChessBitBoard::NO_EN_PASSANT;

    if (isEnPassantCapture) {
        capturedType = PAWN;
        enPassantCaptureSquare = m_bitBoard.getEnPassantCaptureSquare(to, m_currentPlayer);
    }

    // Detect castling before update()
    uint8_t dist = (from > to) ? (from - to) : (to - from);
    bool castling = (movingType == KING && dist == 2);

    m_bitBoard.update(from, to);
    emit moveExecuted(from, to);
    if (isEnPassantCapture) {
        emit enPassantCapturePerformed(enPassantCaptureSquare);
    }

    // Castling: notify the UI rook move
    if (castling) {
        uint8_t rookFrom, rookTo;
        if (to > from) { 
            // Petit roque
            rookFrom = (m_currentPlayer == WHITE) ? 7 : 63;
            rookTo   = (m_currentPlayer == WHITE) ? 5 : 61;
        } else { 
            // Grand roque
            rookFrom = (m_currentPlayer == WHITE) ? 0 : 56;
            rookTo   = (m_currentPlayer == WHITE) ? 3 : 59;
        }
        emit castleRookMoved(rookFrom, rookTo);
    }

    if (capturedType != PieceType::NONE) {
        emit pieceCaptured(capturedType, capturedColor);
    }
    // Déterminer et jouer le bon son
    bool isWhitePromo = (m_currentPlayer == WHITE && movingType == PAWN && ((1ULL << to) & ControllerConst::RANK_8));
    bool isBlackPromo = (m_currentPlayer == BLACK && movingType == PAWN && ((1ULL << to) & ControllerConst::RANK_1));

    if (isWhitePromo || isBlackPromo) {
        emit promotionDetected(to);
        return;
    }

    endTurn(); // Change de joueur d'abord

    // Jouer le son APRÈS avoir vérifié l'échec
    if (isCheckmate(m_currentPlayer, m_bitBoard)) {
        m_soundController.playSound(SoundType::GAME_END);
        emit checkMateDetected(m_currentPlayer);
    } else if (isKingInCheck(m_currentPlayer, m_bitBoard)) {
        m_soundController.playSound(SoundType::CHECK);
    } else if (castling) {
        m_soundController.playSound(SoundType::CASTLE);
    } else if (capturedType != PieceType::NONE) {
        m_soundController.playSound(SoundType::CAPTURE);
    } else {
        m_soundController.playSound(SoundType::MOVE);
    }
    if (m_isAiTurn) {
        QTimer::singleShot(300, this, &ChessController::handleAiTurnIfNeeded);
    }
}

void ChessController::endTurn() {
    m_currentPlayer = (m_currentPlayer == WHITE) ? BLACK : WHITE;
    m_isAiTurn = m_machine && !m_isAiTurn;
}

void ChessController::handleAiTurnIfNeeded()
{
    if (!m_isAiTurn) return;

    // Lance le minimax dans un thread séparé
    QFuture<AIHelper::Move> future = QtConcurrent::run([this]() {
        return m_chessAi.getBestMove(m_bitBoard);
    });

    m_aiWatcher.setFuture(future);

    connect(&m_aiWatcher, &QFutureWatcher<AIHelper::Move>::finished, this, &ChessController::onAiMoveReady, Qt::SingleShotConnection);
}

void ChessController::onAiMoveReady()
{
    const AIHelper::Move move = m_aiWatcher.result();
    if (move.from == 255 && move.to == 255) return;

    PieceType capturedType = m_bitBoard.getPieceType(move.to);
    Color     capturedColor = (m_currentPlayer == WHITE) ? BLACK : WHITE;
    PieceType movingType    = m_bitBoard.getPieceType(move.from);
    bool isEnPassantCapture    = m_bitBoard.isEnPassantMove(move.from, move.to);
    uint8_t enPassantCaptureSquare = ChessBitBoard::NO_EN_PASSANT;

    if (isEnPassantCapture) {
        capturedType = PAWN;
        enPassantCaptureSquare = m_bitBoard.getEnPassantCaptureSquare(move.to, m_currentPlayer);
    }

    // Detect castling before update()
    uint8_t dist = (move.from > move.to) ? (move.from - move.to) : (move.to - move.from);
    bool castling = (movingType == KING && dist == 2);

    m_bitBoard.update(move.from, move.to);
    emit moveExecuted(move.from, move.to);
    if (isEnPassantCapture) {
        emit enPassantCapturePerformed(enPassantCaptureSquare);
    }

    // Castling: notify the UI rook move
    if (castling) {
        uint8_t rookFrom, rookTo;
        if (move.to > move.from) { // Petit roque
            rookFrom = (m_currentPlayer == WHITE) ? 7 : 63;
            rookTo   = (m_currentPlayer == WHITE) ? 5 : 61;
        } else { // Grand roque
            rookFrom = (m_currentPlayer == WHITE) ? 0 : 56;
            rookTo   = (m_currentPlayer == WHITE) ? 3 : 59;
        }
        emit castleRookMoved(rookFrom, rookTo);
    }

    if (capturedType != PieceType::NONE)
        emit pieceCaptured(capturedType, capturedColor);

    bool isWhitePromo = (m_currentPlayer == WHITE && movingType == PAWN
                         && ((1ULL << move.to) & ControllerConst::RANK_8));
    bool isBlackPromo = (m_currentPlayer == BLACK && movingType == PAWN
                         && ((1ULL << move.to) & ControllerConst::RANK_1));

    if (isWhitePromo || isBlackPromo) {
        m_bitBoard.promotePawn(move.to, PieceType::QUEEN, m_currentPlayer);
        emit pawnPromoted(move.to, PieceType::QUEEN, m_currentPlayer);
    }

    endTurn();

    if (isCheckmate(m_currentPlayer, m_bitBoard)) {
        m_soundController.playSound(SoundType::GAME_END);
        emit checkMateDetected(m_currentPlayer);
    } else if (isKingInCheck(m_currentPlayer, m_bitBoard)) {
        m_soundController.playSound(SoundType::CHECK);
    } else if (castling) {
        m_soundController.playSound(SoundType::CASTLE);
    } else if (capturedType != PieceType::NONE) {
        m_soundController.playSound(SoundType::CAPTURE);
    } else if (isWhitePromo || isBlackPromo) {
        m_soundController.playSound(SoundType::PROMOTION);
    } else {
        m_soundController.playSound(SoundType::MOVE);
    }
}
