#include "gamecontroller.h"
#include "board.h"
#include <QTimer>
#include <QPropertyAnimation>
#include <cmath>
#include <QTextStream>
#include <QDir>
#include <QFileDialog>
#include <QStandardPaths>

GameController::GameController(Board* board, InfoPanel* panel, QObject* parent)
    : QObject(parent), board(board), infoPanel(panel), currentPlayer(Player::Player1), turnNumber(1)
{
    connect(board, &Board::cellClicked, this, &GameController::handleCellClicked);
    connect(infoPanel, &InfoPanel::copyLogRequested, this, &GameController::onCopyLogRequested);
    connect(infoPanel, &InfoPanel::downloadLogRequested, this, &GameController::onDownloadLogRequested);
    connect(infoPanel, &InfoPanel::downloadLogRequestedToDesktop, this, &GameController::onDownloadLogToDesktopRequested);
    connect(this, &GameController::sendLogToClipboard, infoPanel, &InfoPanel::copyToClipboard);
}

void GameController::initializeGame() {
    setupInitialPieces();
    board->updateAllCellDisplays();
    emit turnChanged(currentPlayer, turnNumber);
    emit statusMessage("GAME START. Player 1, select a piece.");
}

void GameController::setupInitialPieces() {
    pieces.clear();

    placePiece(11, 0, PieceType::Rindall, Player::Player1);
    placePiece(11, 11, PieceType::Rindall, Player::Player1);
    placePiece(11, 1, PieceType::Vanguard, Player::Player1);
    placePiece(11, 10, PieceType::Vanguard, Player::Player1);
    placePiece(11, 2, PieceType::Sentinel, Player::Player1);
    placePiece(11, 9, PieceType::Sentinel, Player::Player1);
    placePiece(11, 3, PieceType::Worker, Player::Player1);
    placePiece(11, 8, PieceType::Worker, Player::Player1);
    placePiece(11, 4, PieceType::Footman, Player::Player1);
    placePiece(11, 7, PieceType::Footman, Player::Player1);
    placePiece(11, 5, PieceType::Bastion, Player::Player1);
    placePiece(11, 6, PieceType::Daick, Player::Player1);

    for (int col = 0; col < 12; ++col) {
        placePiece(10, col, PieceType::Footman, Player::Player1);
    }

    placePiece(0, 0, PieceType::Rindall, Player::Player2);
    placePiece(0, 11, PieceType::Rindall, Player::Player2);
    placePiece(0, 1, PieceType::Vanguard, Player::Player2);
    placePiece(0, 10, PieceType::Vanguard, Player::Player2);
    placePiece(0, 2, PieceType::Sentinel, Player::Player2);
    placePiece(0, 9, PieceType::Sentinel, Player::Player2);
    placePiece(0, 3, PieceType::Worker, Player::Player2);
    placePiece(0, 8, PieceType::Worker, Player::Player2);
    placePiece(0, 4, PieceType::Footman, Player::Player2);
    placePiece(0, 7, PieceType::Footman, Player::Player2);
    placePiece(0, 5, PieceType::Bastion, Player::Player2);
    placePiece(0, 6, PieceType::Daick, Player::Player2);

    for (int col = 0; col < 12; ++col) {
        placePiece(1, col, PieceType::Footman, Player::Player2);
    }
}

void GameController::placePiece(int row, int col, PieceType type, Player player) {
    Piece* piece = nullptr;

    switch(type) {
    case PieceType::Daick: piece = new Daick(player); break;
    case PieceType::Rindall: piece = new Rindall(player); break;
    case PieceType::Sentinel: piece = new Sentinel(player); break;
    case PieceType::Vanguard: piece = new Vanguard(player); break;
    case PieceType::Worker: piece = new Worker(player); break;
    case PieceType::Bastion: piece = new Bastion(player); break;
    case PieceType::Footman: piece = new Footman(player); break;
    }

    if (piece && board) {
        PieceWidget* pieceWidget = new PieceWidget(piece);
        board->addPieceToCell(row, col, pieceWidget);
        pieces[QPoint(row, col)] = pieceWidget;

        connect(pieceWidget, &PieceWidget::clicked, [this, pieceWidget]() {
            QPoint currentPos = pieces.key(pieceWidget);
            if (!currentPos.isNull()) {
                handleCellClicked(currentPos.x(), currentPos.y());
            }
        });
    }
}

void GameController::handleCellClicked(int row, int col) {
    if (selectedPiece) {
        moveSelectedPieceTo(row, col);
    } else {
        selectPiece(row, col);
    }
}

void GameController::selectPiece(int row, int col) {
    if (auto pieceWidget = board->getPieceAt(row, col)) {
        if (pieceWidget->getPiece()->getPlayer() != currentPlayer) {
            emit statusMessage("ERROR: Not your piece.");
            return;
        }

        selectedPiece = pieceWidget;
        selectedPos = QPoint(row, col);

        auto moves = pieceWidget->getPiece()->getPossibleMoves(row, col, board);
        if (moves.isEmpty()) {
            emit statusMessage(QString("SELECTED: %1. No moves available.").arg(pieceWidget->getPiece()->getDisplayText()));
        } else {
            emit statusMessage(QString("SELECTED: %1. Choose destination.").arg(pieceWidget->getPiece()->getDisplayText()));
        }

        board->highlightCells(moves);
    } else {
        emit statusMessage("No piece selected. Click one of your pieces.");
    }
}

void GameController::moveSelectedPieceTo(int row, int col) {
    if (!selectedPiece) return;

    auto moves = selectedPiece->getPiece()->getPossibleMoves(selectedPos.x(), selectedPos.y(), board);
    bool isValidMove = false;

    for (const auto& move : std::as_const(moves)) {
        if (move.first == row && move.second == col) {
            isValidMove = true;
            break;
        }
    }

    if (isValidMove) {
        bool isCapture = false;
        PieceWidget* targetPiece = board->getPieceAt(row, col);

        if (targetPiece && targetPiece->getPiece()->getPlayer() != selectedPiece->getPiece()->getPlayer()) {
            emit statusMessage(QString("%1 captures %2!").arg(selectedPiece->getPiece()->getDisplayText()).arg(targetPiece->getPiece()->getDisplayText()));
            board->removePieceFromCell(row, col);
            pieces.remove(QPoint(row, col));
            isCapture = true;
            board->updateCellDisplay(selectedPos.x(), selectedPos.y());
            board->updateCellDisplay(row, col);
        } else {
            emit statusMessage(QString("%1 moves to %2%3.")
                                   .arg(selectedPiece->getPiece()->getDisplayText())
                                   .arg(QChar('A' + col))
                                   .arg(12 - row));
        }

        QString notation = generateMoveNotation(selectedPiece, row, col, isCapture);
        emit moveMade(notation, currentPlayer);

        performRetroAnimation(selectedPiece, row, col, isCapture);
    } else {
        emit statusMessage("ERROR: Invalid move. Selection cleared.");
        board->clearHighlights();
        selectedPiece = nullptr;
    }
}

void GameController::performRetroAnimation(PieceWidget* pieceWidget, int toRow, int toCol, bool isCapture) {
    pieceWidget->hide();

    QTimer::singleShot(100, [this, pieceWidget, toRow, toCol]() {
        pieceWidget->show();

        QTimer::singleShot(100, [this, pieceWidget, toRow, toCol]() {

            pieces.remove(selectedPos);
            pieces[QPoint(toRow, toCol)] = pieceWidget;

            board->movePiece(selectedPos.x(), selectedPos.y(), toRow, toCol);

            updateCellControl(toRow, toCol, currentPlayer);

            selectedPos = QPoint(toRow, toCol);

            board->clearHighlights();

            pieceWidget->hide();
            QTimer::singleShot(80, [this, pieceWidget]() {
                pieceWidget->show();

                selectedPiece = nullptr;
                endTurn();
            });
        });
    });
}

void GameController::endTurn() {
    QString alertStr = checkResourceGeneration();
    QString passivityStr = checkPassivity();

    if (!passivityStr.isEmpty()) {
        if (!alertStr.isEmpty()) alertStr += "\n";
        alertStr += passivityStr;
    }

    emit alertMessage(alertStr);

    currentPlayer = (currentPlayer == Player::Player1) ? Player::Player2 : Player::Player1;
    turnNumber++;

    emit turnChanged(currentPlayer, turnNumber);

    QString playerStr = (currentPlayer == Player::Player1) ? "Player 1" : "Player 2";
    if (alertStr.isEmpty()) {
        emit statusMessage(QString("TURN %1: %2's turn. Select a piece.").arg(turnNumber).arg(playerStr));
    }

    board->updateAllCellDisplays();
}

QString GameController::checkResourceGeneration() {
    bool resourceBlocked = false;
    bool resourceGained = false;
    bool defenseFull = false;
    Player opponent = (currentPlayer == Player::Player1) ? Player::Player2 : Player::Player1;

    for (int row = 0; row < 12; ++row) {
        for (int col = 0; col < 12; ++col) {
            PieceWidget* piece = board->getPieceAt(row, col);
            if (piece && piece->getPiece()->getType() == PieceType::Worker && piece->getPiece()->getPlayer() == currentPlayer) {

                Cell* cell = board->getCellData(row, col);

                if (cell && cell->isUnderControl() && cell->getController() == currentPlayer) {
                    if (cell->getDefense() >= 5) {
                        defenseFull = true;
                    } else if (isSentinelBlockingCell(row, col, opponent)) {
                        resourceBlocked = true;
                    } else {
                        cell->addResource();
                        resourceGained = true;
                    }
                }
            }
        }
    }

    if (resourceBlocked) {
        return "ALERT: Worker blocked by enemy Sentinel!";
    } else if (defenseFull) {
        return "ALERT: Cell defense is full (5).";
    } else if (resourceGained) {
        emit statusMessage("RESOURCE: Worker generated 1 resource.");
    }

    return "";
}

QString GameController::checkPassivity()
{
    int maxResources = 0;
    QPoint highCell;
    bool cellFound = false;

    for (int row = 0; row < 12; ++row) {
        for (int col = 0; col < 12; ++col) {
            if (Cell* cell = board->getCellData(row, col)) {
                if (cell->getResources() > maxResources) {
                    maxResources = cell->getResources();
                    highCell = QPoint(row, col);
                    cellFound = true;
                }
            }
        }
    }

    if (cellFound && maxResources >= 8) {
        return QString("PASSIVITY: High resources (%1) at %2%3. Unit spawn advised.")
        .arg(maxResources)
            .arg(QChar('A' + highCell.y()))
            .arg(12 - highCell.x());
    }
    return "";
}

bool GameController::isSentinelBlockingCell(int row, int col, Player opponentPlayer) {
    for (int dr = -2; dr <= 2; ++dr) {
        for (int dc = -2; dc <= 2; ++dc) {
            if (dr == 0 && dc == 0) continue;

            int checkRow = row + dr;
            int checkCol = col + dc;

            if (checkRow >= 0 && checkRow < 12 && checkCol >= 0 && checkCol < 12) {
                PieceWidget* piece = board->getPieceAt(checkRow, checkCol);
                if (piece && piece->getPiece()->getType() == PieceType::Sentinel) {
                    if (piece->getPiece()->getPlayer() == opponentPlayer) {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

void GameController::updateCellControl(int row, int col, Player player) {
    Cell* cell = board->getCellData(row, col);
    if (cell) {
        cell->setController(player);
    }
}

QString GameController::generateMoveNotation(PieceWidget* piece, int toRow, int toCol, bool isCapture) const
{
    QString pieceSymbol = piece->getPiece()->getSymbol();
    QString captureMark = isCapture ? "x" : "";
    QString colStr = QChar('A' + toCol);
    QString rowStr = QString::number(12 - toRow);

    return QString("%1%2%3%4").arg(pieceSymbol).arg(captureMark).arg(colStr).arg(rowStr);
}

void GameController::onCopyLogRequested()
{
    QString fullLog = generateFullGameStateNotation();
    emit sendLogToClipboard(fullLog);
}

QString GameController::generateFullGameStateNotation() const
{
    QString notation;
    QTextStream stream(&notation);

    stream << "--- Rindall of Daick: Relatório de Partida ---\n\n";
    stream << "Turno: " << turnNumber << "\n";
    stream << "Jogador Atual: " << (currentPlayer == Player::Player1 ? "Player 1 (Brancas)" : "Player 2 (Pretas)") << "\n\n";

    stream << "--- Estado do Tabuleiro ---\n";
    stream << QString("%1 | %2 | %3 | %4\n")
                  .arg("Casa", -4)
                  .arg("Peça", -10)
                  .arg("Controlador", -11)
                  .arg("Rec/Def");
    stream << "-------------------------------------------\n";

    for (int r = 0; r < 12; ++r) {
        for (int c = 0; c < 12; ++c) {
            Cell* cell = board->getCellData(r, c);
            PieceWidget* piece = board->getPieceAt(r, c);
            if (!cell) continue;

            if (piece || cell->isUnderControl() || cell->getResources() > 0)
            {
                QString cellPos = QString("%1%2").arg(QChar('A' + c)).arg(12 - r);
                QString pieceStr = "Vazio";
                if (piece) {
                    pieceStr = QString("%1 (%2)")
                    .arg(piece->getPiece()->getSymbol())
                        .arg(piece->getPiece()->getPlayer() == Player::Player1 ? "P1" : "P2");
                }

                QString controlStr = "Neutra";
                if (cell->isUnderControl()) {
                    controlStr = (cell->getController() == Player::Player1 ? "P1" : "P2");
                }

                QString econStr = QString("%1/%2")
                                      .arg(cell->getResources())
                                      .arg(cell->getDefense());

                stream << QString("%1 | %2 | %3 | %4\n")
                              .arg(cellPos, -4)
                              .arg(pieceStr, -10)
                              .arg(controlStr, -11)
                              .arg(econStr);
            }
        }
    }

    stream << "\n--- Log de Movimentos ---\n";
    stream << infoPanel->getMoveLogText();

    return notation;
}

void GameController::onDownloadLogRequested()
{
    qInfo() << "Download log requested.";
    QString fullLog = generateFullGameStateNotation();
    QString defaultFileName = QString("matchlog.txt").arg(turnNumber);

    QString defaultPath = QDir::homePath() + "/" + defaultFileName;

    QString fileName = QFileDialog::getSaveFileName(infoPanel,
                                                    "Salvar Log da Partida",
                                                    defaultPath,
                                                    "Text Files (*.txt);;All Files (*)");

    if (fileName.isEmpty()) {
        emit statusMessage("Save cancelled.");
        return;
    }

    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << fullLog;
        file.close();
        emit statusMessage(QString("Log salvo em %1").arg(QDir::toNativeSeparators(fileName)));
    } else {
        emit statusMessage(QString("ERROR: Não foi possível salvar o log: %1").arg(file.errorString()));
    }
}

void GameController::onDownloadLogToDesktopRequested()
{
    QString fullLog = generateFullGameStateNotation();
    QString defaultFileName = QString("matchlog.txt").arg(turnNumber);

    QFile file(defaultFileName);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << fullLog;
        file.close();
        emit statusMessage(QString("Log salvo em %1").arg(QDir::toNativeSeparators(defaultFileName)));
    } else {
        emit statusMessage(QString("ERROR: Não foi possível salvar o log: %1").arg(file.errorString()));
    }
}
