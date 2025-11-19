#include "tacticalmanager.h"
#include "board.h"
#include "gamecontroller.h"
#include "piecewidget.h"
#include <QDebug>

TacticalManager::TacticalManager(Board* board, QObject* parent)
    : QObject(parent), board(board), gameController(nullptr),
    waitingForSecondClick(false)
{
}

TacticalManager::~TacticalManager()
{
    clearAllArrows();
}

void TacticalManager::setGameController(GameController* controller)
{
    gameController = controller;
}

void TacticalManager::handleMiddleClick(int row, int col)
{
    if (!waitingForSecondClick) {
        firstClick = QPoint(row, col);
        waitingForSecondClick = true;
        qDebug() << "TacticalManager: First click at" << row << col;
    } else {
        QPoint secondClick(row, col);
        waitingForSecondClick = false;

        if (firstClick == secondClick) {
            qDebug() << "TacticalManager: Same cell clicked, clearing all arrows.";
            clearAllArrows();
            return;
        }

        TacticalArrow* existingArrow = findArrow(firstClick, secondClick);

        if (existingArrow) {
            if (existingArrow->getType() == TacticalArrow::Tactical) {
                if (isPotentialPreMove(firstClick, secondClick)) {
                    existingArrow->setType(TacticalArrow::PreMove);
                    addPendingMove(firstClick, secondClick);
                    emit statusMessage(QString("Pré-movimento definido: %1%2")
                                           .arg(posToString(firstClick.x(), firstClick.y()))
                                           .arg(posToString(secondClick.x(), secondClick.y())));
                } else {
                    removeArrow(firstClick, secondClick);
                }
            } else {
                existingArrow->setType(TacticalArrow::Tactical);
                removePendingMove(firstClick, secondClick);
                emit statusMessage(QString("Pré-movimento cancelado: %1%2")
                                       .arg(posToString(firstClick.x(), firstClick.y()))
                                       .arg(posToString(secondClick.x(), secondClick.y())));
            }
        } else {
            createArrow(firstClick, secondClick, TacticalArrow::Tactical);
        }
    }
}

TacticalArrow* TacticalManager::findArrow(QPoint start, QPoint end)
{
    for (TacticalArrow* arrow : arrows) {
        if (arrow->getStart() == start && arrow->getEnd() == end) {
            return arrow;
        }
    }
    return nullptr;
}

void TacticalManager::removeArrow(QPoint start, QPoint end)
{
    for (int i = arrows.size() - 1; i >= 0; --i) {
        if (arrows[i]->getStart() == start && arrows[i]->getEnd() == end) {
            arrows[i]->deleteLater();
            arrows.removeAt(i);
            removePendingMove(start, end);
            return;
        }
    }
}

void TacticalManager::createArrow(QPoint start, QPoint end, TacticalArrow::ArrowType type)
{
    TacticalArrow* arrow = new TacticalArrow(start, end, type, board);
    arrow->setGeometry(board->rect());
    arrow->show();
    arrow->raise();
    arrows.append(arrow);

    qDebug() << "TacticalManager: Created arrow from" << start << "to" << end;
}

bool TacticalManager::isPotentialPreMove(QPoint from, QPoint to)
{
    PieceWidget* piece = board->getPieceAt(from.x(), from.y());
    if (!piece) {
        return false;
    }
    return isValidMove(from, to, piece->getPiece()->getPlayer());
}

void TacticalManager::addPendingMove(QPoint from, QPoint to)
{
    PieceWidget* piece = board->getPieceAt(from.x(), from.y());
    if (!piece) return;

    removePendingMove(from, to);

    PendingMove move;
    move.from = from;
    move.to = to;
    move.player = piece->getPiece()->getPlayer();
    pendingMoves.append(move);
}

void TacticalManager::removePendingMove(QPoint from, QPoint to)
{
    for (int i = pendingMoves.size() - 1; i >= 0; --i) {
        if (pendingMoves[i].from == from && pendingMoves[i].to == to) {
            pendingMoves.removeAt(i);
            return;
        }
    }
}


bool TacticalManager::hasPieceAtPosition(QPoint pos, Player player)
{
    PieceWidget* piece = board->getPieceAt(pos.x(), pos.y());
    return (piece && piece->getPiece()->getPlayer() == player);
}

bool TacticalManager::isValidMove(QPoint from, QPoint to, Player player)
{
    PieceWidget* piece = board->getPieceAt(from.x(), from.y());
    if (!piece || piece->getPiece()->getPlayer() != player) {
        return false;
    }

    auto possibleMoves = piece->getPiece()->getPossibleMoves(from.x(), from.y(), board);

    for (const auto& move : possibleMoves) {
        if (move.first == to.x() && move.second == to.y()) {
            return true;
        }
    }

    return false;
}

void TacticalManager::clearPendingMoves(Player player)
{
    for (int i = pendingMoves.size() - 1; i >= 0; --i) {
        if (pendingMoves[i].player == player) {
            pendingMoves.removeAt(i);
        }
    }
    for (TacticalArrow* arrow : arrows) {
        PieceWidget* piece = board->getPieceAt(arrow->getStart().x(), arrow->getStart().y());
        if (piece && piece->getPiece()->getPlayer() == player) {
            if (arrow->getType() == TacticalArrow::PreMove) {
                arrow->setType(TacticalArrow::Tactical);
            }
        }
    }
}


void TacticalManager::checkAndExecutePendingMoves(Player currentPlayer)
{
    if (!gameController) return;

    Player opponent = (currentPlayer == Player::Player1) ? Player::Player2 : Player::Player1;
    clearPendingMoves(opponent);

    for (int i = 0; i < pendingMoves.size(); ++i) {
        const PendingMove& move = pendingMoves[i];

        if (move.player != currentPlayer) {
            continue;
        }

        PieceWidget* piece = board->getPieceAt(move.from.x(), move.from.y());
        if (!piece || piece->getPiece()->getPlayer() != currentPlayer) {
            pendingMoves.removeAt(i);
            i--;
            continue;
        }

        if (isValidMove(move.from, move.to, currentPlayer)) {
            qDebug() << "TacticalManager: Executing pending move from"
                     << move.from << "to" << move.to;

            PieceWidget* targetPiece = board->getPieceAt(move.to.x(), move.to.y());
            bool isCapture = (targetPiece && targetPiece->getPiece()->getPlayer() != currentPlayer);

            QString pieceSymbol = piece->getPiece()->getSymbol();
            QString fromStr = posToString(move.from.x(), move.from.y());
            QString captureMark = isCapture ? "x" : "";
            QString toStr = posToString(move.to.x(), move.to.y());

            QString notation = QString("%1%2%3%4")
                                   .arg(pieceSymbol)
                                   .arg(fromStr)
                                   .arg(captureMark)
                                   .arg(toStr);

            pendingMoves.removeAt(i);
            clearAllArrows();

            emit sendTacticalMove(notation);
            return;
        } else {
            pendingMoves.removeAt(i);
            i--;
        }
    }

    for (int i = arrows.size() - 1; i >= 0; --i) {
        TacticalArrow* arrow = arrows[i];
        if (arrow->getType() == TacticalArrow::PreMove) {
            if (!isPotentialPreMove(arrow->getStart(), arrow->getEnd())) {
                arrow->setType(TacticalArrow::Tactical);
                removePendingMove(arrow->getStart(), arrow->getEnd());
            }
        }
    }
}

void TacticalManager::clearAllArrows()
{
    for (TacticalArrow* arrow : arrows) {
        arrow->deleteLater();
    }
    arrows.clear();
    pendingMoves.clear();
    waitingForSecondClick = false;

    qDebug() << "TacticalManager: Cleared all arrows and pending moves";
}

QString TacticalManager::posToString(int row, int col) const
{
    return QString("%1%2").arg(QChar('A' + col)).arg(12 - row);
}
