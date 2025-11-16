#include "gamecontroller.h"
#include "board.h"
#include "recruitmentdialog.h"
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

    playerStates[Player::Player1] = PlayerState();
    playerStates[Player::Player2] = PlayerState();
}

void GameController::initializeGame() {
    setupInitialPieces();
    board->updateAllCellDisplays();
    updateBastionProtection();
    emit turnChanged(currentPlayer, turnNumber);
    emit statusMessage("GAME START. Player 1, select a piece or click empty cell to recruit.");
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
    case PieceType::Champion: piece = new Champion(player); break;
    case PieceType::Ascendant: piece = new Ascendant(player); break;
    }
    if (piece && board) {
        PieceWidget* pieceWidget = new PieceWidget(piece);
        board->addPieceToCell(row, col, pieceWidget);
        pieces[QPoint(row, col)] = pieceWidget;
        connect(pieceWidget, &PieceWidget::clicked, [this, pieceWidget](Qt::MouseButton button) {
            QPoint currentPos = pieces.key(pieceWidget, QPoint(-1, -1));
            if (currentPos != QPoint(-1, -1)) {
                if (button == Qt::RightButton) {
                    handleBastionRehabilitation(currentPos.x(), currentPos.y());
                } else {
                    handleCellClicked(currentPos.x(), currentPos.y());
                }
            }
        });
    }
}

void GameController::handleCellClicked(int row, int col) {
    if (isAnimating) {
        emit statusMessage("ERROR: Ação bloqueada — animação em andamento.");
        return;
    }

    if (selectedPiece) {
        moveSelectedPieceTo(row, col);
    } else {
        PieceWidget* clickedPiece = board->getPieceAt(row, col);
        if (clickedPiece) {
            selectPiece(row, col);
        } else {
            handleRecruitment(row, col);
        }
    }
}

void GameController::handleRecruitment(int row, int col) {
    Cell* cell = board->getCellData(row, col);
    if (!cell || !cell->isUnderControl() || cell->getController() != currentPlayer) {
        emit statusMessage("ERROR: Can only recruit on controlled cells.");
        return;
    }

    int resources = getAdjacentResourceSum(row, col, currentPlayer);

    if (resources < 2) {
        emit statusMessage(QString("ERROR: Not enough resources in adjacent cells. Found %1, minimum is 2.").arg(resources));
        return;
    }

    QHash<PieceType, int> currentCosts;
    currentCosts[PieceType::Footman] = calculateRecruitmentCost(PieceType::Footman, currentPlayer);
    currentCosts[PieceType::Vanguard] = calculateRecruitmentCost(PieceType::Vanguard, currentPlayer);
    currentCosts[PieceType::Sentinel] = calculateRecruitmentCost(PieceType::Sentinel, currentPlayer);
    currentCosts[PieceType::Rindall] = calculateRecruitmentCost(PieceType::Rindall, currentPlayer);
    currentCosts[PieceType::Worker] = calculateRecruitmentCost(PieceType::Worker, currentPlayer);

    RecruitmentDialog dialog(
        currentPlayer,
        resources,
        currentCosts,
        playerStates[currentPlayer].workersReplaced,
        playerStates[currentPlayer].canRecruitLastLost,
        playerStates[currentPlayer].lastLostPieceType,
        board
        );

    if (dialog.exec() == QDialog::Accepted && dialog.wasConfirmed()) {
        PieceType selectedType = dialog.getSelectedPieceType();

        if (canRecruitPiece(selectedType, currentPlayer, row, col, resources)) {
            recruitPiece(selectedType, currentPlayer, row, col);
        } else {
            emit statusMessage("ERROR: Cannot recruit this piece at this location (check cost or Worker requirements).");
        }
    } else {
        emit statusMessage("Recruitment cancelled.");
    }
}

int GameController::calculateRecruitmentCost(PieceType type, Player player) {
    int baseCost = 0;
    int inflation = playerStates[player].piecesRecruited.value(type, 0);

    switch (type) {
    case PieceType::Footman: baseCost = baseCosts.footman; break;
    case PieceType::Vanguard: baseCost = baseCosts.vanguard; break;
    case PieceType::Sentinel: baseCost = baseCosts.sentinel; break;
    case PieceType::Rindall: baseCost = baseCosts.rindall; break;
    case PieceType::Bastion: baseCost = baseCosts.bastion; break;
    case PieceType::Worker:
        if (playerStates[player].workersReplaced == 0) {
            return 8;
        } else if (playerStates[player].workersReplaced == 1) {
            return 12;
        }
        return 999;
    default:
        return 0;
    }

    return baseCost + inflation;
}

bool GameController::canRecruitPiece(PieceType type, Player player, int row, int col, int availableResources) {
    Cell* cell = board->getCellData(row, col);
    if (!cell || !cell->isUnderControl() || cell->getController() != player) {
        return false;
    }

    if (!playerStates[player].canRecruitLastLost && type == playerStates[player].lastLostPieceType) {
        return false;
    }

    int cost = calculateRecruitmentCost(type, player);

    if (availableResources < cost) {
        return false;
    }

    if (type == PieceType::Worker) {
        if (playerStates[player].workersReplaced >= 2) {
            return false;
        }

        int defenseRequired = (playerStates[player].workersReplaced == 0) ? 6 : 10;

        if (cell->getDefense() < defenseRequired) {
            return false;
        }

        int adjacentAllies = 0;
        int directions[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
        for (int i = 0; i < 4; ++i) {
            int adjRow = row + directions[i][0];
            int adjCol = col + directions[i][1];
            if (adjRow >= 0 && adjRow < 12 && adjCol >= 0 && adjCol < 12) {
                PieceWidget* piece = board->getPieceAt(adjRow, adjCol);
                if (piece && piece->getPiece()->getPlayer() == player) {
                    adjacentAllies++;
                }
            }
        }

        if (adjacentAllies < 2) {
            return false;
        }
    }

    return true;
}


void GameController::recruitPiece(PieceType type, Player player, int row, int col) {
    int availableResources = getAdjacentResourceSum(row, col, player);
    if (!canRecruitPiece(type, player, row, col, availableResources)) {
        emit statusMessage("ERROR: Cannot recruit this piece here.");
        return;
    }

    int cost = calculateRecruitmentCost(type, player);
    spendResourcesFromAdjacent(row, col, player, cost);

    placePiece(row, col, type, player);

    if (type == PieceType::Worker) {
        playerStates[player].workersReplaced++;
    }

    playerStates[player].piecesRecruited[type]++;

    QString pieceSymbol;
    switch(type) {
    case PieceType::Footman: pieceSymbol = "F"; break;
    case PieceType::Vanguard: pieceSymbol = "V"; break;
    case PieceType::Sentinel: pieceSymbol = "S"; break;
    case PieceType::Rindall: pieceSymbol = "R"; break;
    case PieceType::Worker: pieceSymbol = "W"; break;
    default: pieceSymbol = "?"; break;
    }

    QString notation = QString("%1@%2%3")
                           .arg(pieceSymbol)
                           .arg(QChar('A' + col))
                           .arg(12 - row);

    emit moveMade(notation, currentPlayer);
    emit statusMessage(QString("RECRUITED: %1 at %2%3 for %4 resources.")
                           .arg(pieceSymbol)
                           .arg(QChar('A' + col))
                           .arg(12 - row)
                           .arg(cost));

    endTurn();
}


void GameController::checkAndEvolve(PieceWidget* pieceWidget, int row, int col) {
    if (!pieceWidget) return;

    Piece* piece = pieceWidget->getPiece();

    if (piece->getType() == PieceType::Footman && piece->getKillCount() >= 3) {
        evolvePiece(row, col, PieceType::Champion);
        emit alertMessage(QString("EVOLUTION! Footman evolved to Champion at %1%2!")
                              .arg(QChar('A' + col))
                              .arg(12 - row));
    }

    if (piece->getType() == PieceType::Worker) {
        bool underAttack = false;
        Player opponent = (piece->getPlayer() == Player::Player1) ? Player::Player2 : Player::Player1;

        for (int r = 0; r < 12; ++r) {
            for (int c = 0; c < 12; ++c) {
                PieceWidget* enemyPiece = board->getPieceAt(r, c);
                if (enemyPiece && enemyPiece->getPiece()->getPlayer() == opponent) {
                    auto moves = enemyPiece->getPiece()->getPossibleMoves(r, c, board);
                    for (const auto& move : moves) {
                        if (move.first == row && move.second == col) {
                            underAttack = true;
                            break;
                        }
                    }
                    if (underAttack) break;
                }
            }
            if (underAttack) break;
        }

        if (underAttack) {
            piece->incrementTurnsUnderAttack();
        } else {
            piece->resetTurnsUnderAttack();
            piece->resetResourcesAccumulated();
        }

        if (piece->getTurnsUnderAttack() >= 8 && piece->getResourcesAccumulated() >= 20) {
            if (!playerStates[piece->getPlayer()].hasAscendant) {
                evolvePiece(row, col, PieceType::Ascendant);
                playerStates[piece->getPlayer()].hasAscendant = true;
                emit alertMessage(QString("SUPREME EVOLUTION! Worker ascended to Ascendant at %1%2!")
                                      .arg(QChar('A' + col))
                                      .arg(12 - row));
            }
        }
    }
}

void GameController::evolvePiece(int row, int col, PieceType newType) {
    PieceWidget* oldPiece = board->getPieceAt(row, col);
    if (!oldPiece) return;

    Player player = oldPiece->getPiece()->getPlayer();
    int killCount = oldPiece->getPiece()->getKillCount();

    board->removePieceFromCell(row, col);
    pieces.remove(QPoint(row, col));

    placePiece(row, col, newType, player);

    PieceWidget* newPiece = board->getPieceAt(row, col);
    if (newPiece && newType == PieceType::Champion) {
        for (int i = 0; i < killCount; ++i) {
            newPiece->getPiece()->incrementKillCount();
        }
    }

    board->updateCellDisplay(row, col);
}

void GameController::selectPiece(int row, int col) {
    if (isAnimating) {
        emit statusMessage("ERROR: Não é possível selecionar durante animação.");
        return;
    }

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
        emit statusMessage("No piece selected. Click one of your pieces or empty cell to recruit.");
    }
}

bool GameController::canAttack(PieceWidget* attacker, int targetRow, int targetCol) {
    if (!attacker) return false;

    int attackPower = attacker->getPiece()->getAttackPower();
    if (attackPower == 0) return false;

    PieceWidget* target = board->getPieceAt(targetRow, targetCol);
    if (!target) return false;

    if (target->getPiece()->getPlayer() == attacker->getPiece()->getPlayer()) return false;

    if (isProtectedByBastion(targetRow, targetCol, target->getPiece()->getPlayer())) {
        return false;
    }

    return true;
}

bool GameController::isProtectedByBastion(int row, int col, Player defender) {
    int directions[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};

    for (int i = 0; i < 4; ++i) {
        int checkRow = row + directions[i][0];
        int checkCol = col + directions[i][1];

        if (checkRow >= 0 && checkRow < 12 && checkCol >= 0 && checkCol < 12) {
            PieceWidget* adjacentPiece = board->getPieceAt(checkRow, checkCol);
            if (adjacentPiece &&
                adjacentPiece->getPiece()->getType() == PieceType::Bastion &&
                adjacentPiece->getPiece()->getPlayer() == defender) {
                return true;
            }
        }
    }
    return false;
}

int GameController::calculateTotalDefense(int row, int col) {
    Cell* cell = board->getCellData(row, col);
    if (!cell) return 0;

    PieceWidget* piece = board->getPieceAt(row, col);
    if (!piece) return cell->getDefense();

    Player pieceOwner = piece->getPiece()->getPlayer();
    int cellDefense = cell->getDefense(pieceOwner);
    int pieceDefense = piece->getPiece()->getCurrentDefense();

    return cellDefense + pieceDefense;
}

void GameController::consumeCellResources(int row, int col, int amountToConsume) {
    Cell* cell = board->getCellData(row, col);
    if (!cell) return;

    int resourcesNeeded = amountToConsume;
    while (resourcesNeeded > 0 && cell->getResources() > 0) {
        cell->removeResource();
        resourcesNeeded--;
    }
}

bool GameController::executeAttack(PieceWidget* attacker, int attackerRow, int attackerCol, int targetRow, int targetCol) {
    if (!canAttack(attacker, targetRow, targetCol)) {
        return false;
    }

    int attackPower = attacker->getPiece()->getAttackPower();
    int totalDefense = calculateTotalDefense(targetRow, targetCol);

    PieceWidget* target = board->getPieceAt(targetRow, targetCol);
    Cell* targetCell = board->getCellData(targetRow, targetCol);

    if (!target || !targetCell) return false;

    int cellDef = targetCell->getDefense();
    int pieceDef = target->getPiece()->getCurrentDefense();

    if (attackPower > totalDefense) {
        emit statusMessage(QString("COMBAT: %1 (ATK:%2) vs %3 (DEF TOTAL:%4) - ATTACKER WINS!")
                               .arg(attacker->getPiece()->getDisplayText())
                               .arg(attackPower)
                               .arg(target->getPiece()->getDisplayText())
                               .arg(totalDefense));

        consumeCellResources(targetRow, targetCol, cellDef);

        target->getPiece()->setCurrentDefense(0);

        attacker->getPiece()->incrementKillCount();

        PieceType lostType = target->getPiece()->getType();
        Player loserPlayer = target->getPiece()->getPlayer();
        playerStates[loserPlayer].lastLostPieceType = lostType;
        playerStates[loserPlayer].canRecruitLastLost = false;

        if (lostType == PieceType::Ascendant) {
        }

        isAnimating = true;
        performDestructionAnimation(target, targetRow, targetCol);

        return true;
    } else {
        emit statusMessage(QString("COMBAT: %1 (ATK:%2) vs %3 (DEF TOTAL:%4) - ATTACK FAILED!")
                               .arg(attacker->getPiece()->getDisplayText())
                               .arg(attackPower)
                               .arg(target->getPiece()->getDisplayText())
                               .arg(totalDefense));

        int damageToApply = attackPower;

        int damageToCell = qMin(damageToApply, cellDef);
        if (damageToCell > 0) {
            consumeCellResources(targetRow, targetCol, damageToCell);
            damageToApply -= damageToCell;
            emit statusMessage(QString("...Defesa da casa %1%2 absorveu %3 de dano.")
                                   .arg(QChar('A' + targetCol)).arg(12 - targetRow).arg(damageToCell));
        }

        if (damageToApply > 0 && pieceDef > 0) {
            int newPieceDef = qMax(0, pieceDef - damageToApply);
            target->getPiece()->setCurrentDefense(newPieceDef);
            emit statusMessage(QString("...%1 sofreu %2 de dano em sua defesa! (Defesa restante: %3)")
                                   .arg(target->getPiece()->getFullName())
                                   .arg(damageToApply)
                                   .arg(newPieceDef));

            if (newPieceDef == 0 && target->getPiece()->getType() == PieceType::Bastion) {
                emit alertMessage(QString("A defesa do Bastion em %1%2 foi quebrada!").arg(QChar('A' + targetCol)).arg(12 - targetRow));
            }
        }

        board->updateCellDisplay(targetRow, targetCol);
        return false;
    }
}

void GameController::performDestructionAnimation(PieceWidget* pieceWidget, int row, int col) {

    for (int i = 0; i < 3; ++i) {
        QTimer::singleShot(i * 150, [pieceWidget]() {
            pieceWidget->hide();
        });
        QTimer::singleShot(i * 150 + 75, [pieceWidget]() {
            pieceWidget->show();
        });
    }

    QTimer::singleShot(450, [this, pieceWidget, row, col]() {
        board->removePieceFromCell(row, col);
        pieces.remove(QPoint(row, col));
        board->updateCellDisplay(row, col);

        if (pieceWidget->getPiece()->getType() == PieceType::Daick) {
            Player winner = pieceWidget->getPiece()->getPlayer() == Player::Player1
                                ? Player::Player2 : Player::Player1;
            emit alertMessage(QString("GAME OVER! %1 WINS by Daick's Fall!")
                                  .arg(winner == Player::Player1 ? "Player 1" : "Player 2"));
        }
    });
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
        isAnimating = true;
        PieceWidget* targetPiece = board->getPieceAt(row, col);
        bool isCapture = false;

        if (targetPiece && targetPiece->getPiece()->getPlayer() != selectedPiece->getPiece()->getPlayer()) {
            if (isProtectedByBastion(row, col, targetPiece->getPiece()->getPlayer())) {
                emit statusMessage("ERROR: Target protected by Bastion! Attack the Bastion first.");
                board->clearHighlights();
                selectedPiece = nullptr;
                isAnimating = false;
                return;
            }

            bool attackSuccess = executeAttack(selectedPiece, selectedPos.x(), selectedPos.y(), row, col);

            if (attackSuccess) {
                isAnimating = true;

                PieceWidget* attackerCopy = selectedPiece;

                QTimer::singleShot(500, [this, attackerCopy, row, col]() {
                    QString notation = generateMoveNotation(attackerCopy, row, col, true);
                    emit moveMade(notation, currentPlayer);
                    performRetroAnimation(attackerCopy, row, col, true);
                });
                return;
            } else {
                board->clearHighlights();
                selectedPiece = nullptr;
                endTurn();
                isAnimating = false;
                return;
            }
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
            QTimer::singleShot(80, [this, pieceWidget, toRow, toCol]() {
                pieceWidget->show();

                checkAndEvolve(pieceWidget, toRow, toCol);

                selectedPiece = nullptr;
                updateBastionProtection();
                endTurn();
                isAnimating = false;
            });
        });
    });
}

void GameController::updateBastionProtection() {
    for (int row = 0; row < 12; ++row) {
        for (int col = 0; col < 12; ++col) {
            Cell* cell = board->getCellData(row, col);
            if (cell) {
                cell->setBastionDefense(0);
            }
        }
    }

    for (int row = 0; row < 12; ++row) {
        for (int col = 0; col < 12; ++col) {
            PieceWidget* bastionPiece = board->getPieceAt(row, col);
            if (bastionPiece && bastionPiece->getPiece()->getType() == PieceType::Bastion) {
                int bastionDefense = bastionPiece->getPiece()->getDefensePower();
                Player bastionPlayer = bastionPiece->getPiece()->getPlayer();

                int directions[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
                for (int i = 0; i < 4; ++i) {
                    int adjRow = row + directions[i][0];
                    int adjCol = col + directions[i][1];

                    if (adjRow >= 0 && adjRow < 12 && adjCol >= 0 && adjCol < 12) {
                        PieceWidget* protectedPiece = board->getPieceAt(adjRow, adjCol);

                        if (protectedPiece && protectedPiece->getPiece()->getPlayer() == bastionPlayer) {
                            Cell* adjCell = board->getCellData(adjRow, adjCol);
                            if (adjCell) {
                                adjCell->setBastionDefense(bastionDefense);
                            }
                        }
                    }
                }
            }
        }
    }
}

void GameController::endTurn() {
    playerStates[currentPlayer].canRecruitLastLost = true;

    QString alertStr = checkResourceGeneration();
    QString passivityStr = checkPassivity();
    QString evolutionStr = checkEvolutionConditions();

    if (!passivityStr.isEmpty()) {
        if (!alertStr.isEmpty()) alertStr += "\n";
        alertStr += passivityStr;
    }

    if (!evolutionStr.isEmpty()) {
        if (!alertStr.isEmpty()) alertStr += "\n";
        alertStr += evolutionStr;
    }

    emit alertMessage(alertStr);

    currentPlayer = (currentPlayer == Player::Player1) ? Player::Player2 : Player::Player1;
    turnNumber++;

    emit turnChanged(currentPlayer, turnNumber);

    QString playerStr = (currentPlayer == Player::Player1) ? "Player 1" : "Player 2";
    if (alertStr.isEmpty()) {
        emit statusMessage(QString("TURN %1: %2's turn. Select a piece or recruit.").arg(turnNumber).arg(playerStr));
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

            if (!piece || piece->getPiece()->getPlayer() != currentPlayer) {
                continue;
            }

            PieceType type = piece->getPiece()->getType();

            if (type == PieceType::Worker || type == PieceType::Ascendant) {
                Cell* cell = board->getCellData(row, col);

                if (cell && cell->isUnderControl() && cell->getController() == currentPlayer) {
                    if (cell->getResources() >= 5) {
                        defenseFull = true;
                    } else if (isSentinelBlockingCell(row, col, opponent)) {
                        resourceBlocked = true;
                    } else {
                        cell->addResource(currentPlayer);
                        resourceGained = true;

                        if (type == PieceType::Worker) {
                            piece->getPiece()->addResourcesAccumulated(1);
                        }
                    }
                }
            }
        }
    }

    if (resourceBlocked) {
        return "ALERT: Worker blocked by enemy Sentinel!";
    } else if (defenseFull) {
        return "ALERT: Cell resource limit is full (5).";
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

QString GameController::checkEvolutionConditions()
{
    QString alerts;

    for (int row = 0; row < 12; ++row) {
        for (int col = 0; col < 12; ++col) {
            PieceWidget* piece = board->getPieceAt(row, col);
            if (piece && piece->getPiece()->getPlayer() == currentPlayer) {
                if (piece->getPiece()->getType() == PieceType::Footman &&
                    piece->getPiece()->getKillCount() >= 3) {
                    if (!alerts.isEmpty()) alerts += "\n";
                    alerts += QString("EVOLUTION READY: Footman at %1%2 can evolve to Champion (3 kills)!")
                                  .arg(QChar('A' + col))
                                  .arg(12 - row);
                }

                if (piece->getPiece()->getType() == PieceType::Worker) {
                    if (piece->getPiece()->getTurnsUnderAttack() >= 8 &&
                        piece->getPiece()->getResourcesAccumulated() >= 20 &&
                        !playerStates[currentPlayer].hasAscendant) {
                        if (!alerts.isEmpty()) alerts += "\n";
                        alerts += QString("ASCENSION READY: Worker at %1%2 can become Ascendant!")
                                      .arg(QChar('A' + col))
                                      .arg(12 - row);
                    }
                }
            }
        }
    }

    return alerts;
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
    stream << QString("%1 | %2 | %3 | %4 | %5\n")
                  .arg("Casa", -4)
                  .arg("Peça", -10)
                  .arg("Controlador", -11)
                  .arg("Rec/Def", -7)
                  .arg("ATK/DEF", -7);
    stream << "-----------------------------------------------------------\n";

    for (int r = 0; r < 12; ++r) {
        for (int c = 0; c < 12; ++c) {
            Cell* cell = board->getCellData(r, c);
            PieceWidget* piece = board->getPieceAt(r, c);
            if (!cell) continue;

            if (piece || cell->isUnderControl() || cell->getResources() > 0)
            {
                QString cellPos = QString("%1%2").arg(QChar('A' + c)).arg(12 - r);
                QString pieceStr = "Vazio";
                QString atkDefStr = "-";

                if (piece) {
                    pieceStr = QString("%1 (%2)")
                    .arg(piece->getPiece()->getSymbol())
                        .arg(piece->getPiece()->getPlayer() == Player::Player1 ? "P1" : "P2");

                    atkDefStr = QString("%1/%2")
                                    .arg(piece->getPiece()->getAttackPower())
                                    .arg(piece->getPiece()->getDefensePower());

                    if (piece->getPiece()->getKillCount() > 0) {
                        pieceStr += QString(" [%1K]").arg(piece->getPiece()->getKillCount());
                    }
                }

                QString controlStr = "Neutra";
                if (cell->isUnderControl()) {
                    controlStr = (cell->getController() == Player::Player1 ? "P1" : "P2");
                }

                QString econStr = QString("%1/%2")
                                      .arg(cell->getResources())
                                      .arg(cell->getDefense());

                stream << QString("%1 | %2 | %3 | %4 | %5\n")
                              .arg(cellPos, -4)
                              .arg(pieceStr, -10)
                              .arg(controlStr, -11)
                              .arg(econStr, -7)
                              .arg(atkDefStr, -7);
            }
        }
    }

    stream << "\n--- Estado dos Jogadores ---\n";
    for (int p = 0; p < 2; ++p) {
        Player player = (p == 0) ? Player::Player1 : Player::Player2;
        QString playerStr = (player == Player::Player1) ? "Player 1 (Brancas)" : "Player 2 (Pretas)";

        stream << QString("\n%1:\n").arg(playerStr);
        stream << QString(" - Peças Recrutadas:\n");

        for (auto it = playerStates[player].piecesRecruited.constBegin(); it != playerStates[player].piecesRecruited.constEnd(); ++it) {
            QString pieceName;
            switch(it.key()) {
            case PieceType::Footman: pieceName = "Footman"; break;
            case PieceType::Vanguard: pieceName = "Vanguard"; break;
            case PieceType::Sentinel: pieceName = "Sentinel"; break;
            case PieceType::Rindall: pieceName = "Rindall"; break;
            case PieceType::Worker: pieceName = "Worker"; break;
            case PieceType::Bastion: pieceName = "Bastion"; break;
            default: pieceName = "Unknown"; break;
            }
            stream << QString("     * %1: %2\n").arg(pieceName).arg(it.value());
        }
    }

    stream << "\n--- Log de Movimentos ---\n";
    stream << infoPanel->getMoveLogText();

    return notation;
}

void GameController::onDownloadLogRequested()
{
    QString fullLog = generateFullGameStateNotation();
    QString defaultFileName = QString("matchlog_turn%1.txt").arg(turnNumber);

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
    QString defaultFileName = QString("matchlog_turn%1.txt").arg(turnNumber);

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


int GameController::getAdjacentResourceSum(int row, int col, Player player) const {
    int totalResources = 0;

    int directions[5][2] = {
        {0, 0},
        {-1, 0},
        {1, 0},
        {0, -1},
        {0, 1}
    };

    for (int i = 0; i < 5; ++i) {
        int r = row + directions[i][0];
        int c = col + directions[i][1];

        if (r >= 0 && r < 12 && c >= 0 && c < 12) {
            Cell* adjCell = board->getCellData(r, c);
            if (adjCell && adjCell->isUnderControl() && adjCell->getController() == player) {
                if (adjCell->hasOwner() && adjCell->getResourceOwner() == player) {
                    totalResources += adjCell->getResources();
                }
            }
        }
    }
    return totalResources;
}

void GameController::spendResourcesFromAdjacent(int row, int col, Player player, int cost) {
    int remainingCost = cost;

    QList<Cell*> resourcePool;
    int directions[5][2] = {{0, 0}, {-1, 0}, {1, 0}, {0, -1}, {0, 1}};
    QList<QPoint> affectedCells;

    for (int i = 0; i < 5; ++i) {
        int r = row + directions[i][0];
        int c = col + directions[i][1];
        affectedCells.append(QPoint(r, c));

        if (r >= 0 && r < 12 && c >= 0 && c < 12) {
            Cell* adjCell = board->getCellData(r, c);
            if (adjCell && adjCell->isUnderControl() && adjCell->getController() == player && adjCell->getResources() > 0) {
                resourcePool.append(adjCell);
            }
        }
    }

    while (remainingCost > 0 && !resourcePool.isEmpty()) {
        Cell* richestCell = nullptr;
        int maxRes = 0;
        for (Cell* cell : resourcePool) {
            if (cell->getResources() > maxRes) {
                maxRes = cell->getResources();
                richestCell = cell;
            }
        }

        if (richestCell == nullptr) {
            break;
        }

        richestCell->removeResource();
        remainingCost--;

        if (richestCell->getResources() == 0) {
            resourcePool.removeOne(richestCell);
        }
    }

    for (const QPoint& p : affectedCells) {
        if (p.x() >= 0 && p.x() < 12 && p.y() >= 0 && p.y() < 12) {
            board->updateCellDisplay(p.x(), p.y());
        }
    }
}

void GameController::handleBastionRehabilitation(int row, int col) {
    PieceWidget* bastion = board->getPieceAt(row, col);
    if (!bastion) {
        return;
    }

    if (bastion->getPiece()->getType() != PieceType::Bastion) {
        return;
    }

    if (bastion->getPiece()->getPlayer() != currentPlayer) {
        emit statusMessage("ERROR: Not your piece.");
        return;
    }

    if (bastion->getPiece()->getCurrentDefense() >= bastion->getPiece()->getDefensePower()) {
        emit statusMessage("BASTION: Already at full defense. No rehabilitation needed.");
        return;
    }

    if (!hasAdjacentWorker(row, col, currentPlayer)) {
        emit statusMessage("ERROR: Reabilitação falhou. Requer um Worker aliado adjacente.");
        return;
    }

    int defenseMissing = bastion->getPiece()->getDefensePower() - bastion->getPiece()->getCurrentDefense();
    int repairCost = calculateBastionRepairCost(defenseMissing, currentPlayer);

    int availableResources = getAdjacentResourceSum(row, col, currentPlayer);
    if (availableResources < repairCost) {
        emit statusMessage(QString("ERROR: Reabilitação falhou. Recursos insuficientes. (Requer: %1, Disponível: %2)")
                               .arg(repairCost)
                               .arg(availableResources));
        return;
    }

    spendResourcesFromAdjacent(row, col, currentPlayer, repairCost);

    bastion->getPiece()->setCurrentDefense(bastion->getPiece()->getDefensePower());

    playerStates[currentPlayer].bastionRepairs++;

    board->updateCellDisplay(row, col);

    QString notation = QString("B_REPAIR@%1%2")
                           .arg(QChar('A' + col))
                           .arg(12 - row);

    emit moveMade(notation, currentPlayer);
    emit statusMessage(QString("AÇÃO: Bastion reabilitado por %1 recursos! (Inflação de reparo agora: %2)")
                           .arg(repairCost)
                           .arg(playerStates[currentPlayer].bastionRepairs));

    endTurn();
}

bool GameController::hasAdjacentWorker(int row, int col, Player player) const {
    int directions[8][2] = {
        {-1, -1}, {-1, 0}, {-1, 1},
        {0, -1},           {0, 1},
        {1, -1},  {1, 0},  {1, 1}
    };

    for (int i = 0; i < 8; ++i) {
        int checkRow = row + directions[i][0];
        int checkCol = col + directions[i][1];

        if (checkRow >= 0 && checkRow < 12 && checkCol >= 0 && checkCol < 12) {
            PieceWidget* piece = board->getPieceAt(checkRow, checkCol);
            if (piece &&
                piece->getPiece()->getPlayer() == player &&
                piece->getPiece()->getType() == PieceType::Worker)
            {
                return true;
            }
        }
    }
    return false;
}

int GameController::calculateBastionRepairCost(int pointsToRepair, Player player) const {
    const int baseCostPerPoint = 1;

    int inflation = playerStates.value(player).bastionRepairs;

    int costPerPoint = baseCostPerPoint + inflation;

    return costPerPoint * pointsToRepair;
}
