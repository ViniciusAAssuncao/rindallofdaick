#include "gamecontroller.h"
#include "audiomanager.h"
#include "board.h"
#include "recruitmentdialog.h"
#include <QTimer>
#include <QPropertyAnimation>
#include <cmath>
#include <QTextStream>
#include <QDir>
#include <QFileDialog>
#include <QStandardPaths>
#include <QDebug>
#include <QRegularExpression>

GameController::GameController(Board* board, InfoPanel* panel, QObject* parent)
    : QObject(parent), board(board), infoPanel(panel),
    selectedPiece(nullptr),
    player1(nullptr), player2(nullptr), activePlayer(nullptr),
    turnNumber(1), isAnimating(false), gameActive(true)
{
    connect(board, &Board::cellClicked, this, &GameController::handleCellClicked);
    connect(infoPanel, &InfoPanel::copyLogRequested, this, &GameController::onCopyLogRequested);
    connect(infoPanel, &InfoPanel::downloadLogRequested, this, &GameController::onDownloadLogRequested);
    connect(infoPanel, &InfoPanel::downloadLogRequestedToDesktop, this, &GameController::onDownloadLogToDesktopRequested);
    connect(this, &GameController::sendLogToClipboard, infoPanel, &InfoPanel::copyToClipboard);

    playerStates[Player::Player1] = PlayerState();
    playerStates[Player::Player2] = PlayerState();
}

void GameController::setEnginePaths(const QString& p1Path, const QString& p2Path)
{
    m_player1EnginePath = p1Path;
    m_player2EnginePath = p2Path;
}

void GameController::initializeGame() {
    AudioManager::instance().initialize();
    AudioManager::instance().playBackgroundMusic();
    AudioManager::instance().playSoundEffect(SoundEffect::GameStart);

    setupInitialPieces();
    board->updateAllCellDisplays();
    updateBastionProtection();

    if (m_player1EnginePath.isEmpty()) {
        player1 = new HumanPlayer(Player::Player1, this);
        qDebug() << "Player 1 (White) is HUMAN";
    } else {
        player1 = new EnginePlayer(Player::Player1, m_player1EnginePath, this);
        qDebug() << "Player 1 (White) is ENGINE:" << m_player1EnginePath;
    }

    if (m_player2EnginePath.isEmpty()) {
        player2 = new HumanPlayer(Player::Player2, this);
        qDebug() << "Player 2 (Black) is HUMAN";
    } else {
        player2 = new EnginePlayer(Player::Player2, m_player2EnginePath, this);
        qDebug() << "Player 2 (Black) is ENGINE:" << m_player2EnginePath;
    }

    activePlayer = player1;

    connect(player1, &IPlayer::moveReady, this, &GameController::onMoveReceived);
    connect(player2, &IPlayer::moveReady, this, &GameController::onMoveReceived);

    emit turnChanged(activePlayer->getPlayer(), turnNumber);
    emit statusMessage("GAME START. Player 1, select a piece or click empty cell to recruit.");

    activePlayer->requestMove(generateFullGameStateNotation());
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
    HumanPlayer* human = dynamic_cast<HumanPlayer*>(activePlayer);
    if (!human) {
        if (gameActive) {
            emit statusMessage("ERROR: Not your turn (AI is thinking).");
        }
        return;
    }

    if (!gameActive) {
        emit statusMessage("GAME OVER: No more actions allowed.");
        return;
    }

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
    if (!gameActive) {
        emit statusMessage("GAME OVER: No more actions allowed.");
        return;
    }

    Cell* cell = board->getCellData(row, col);
    if (!cell || !cell->isUnderControl() || cell->getController() != activePlayer->getPlayer()) {
        emit statusMessage("ERROR: Can only recruit on controlled cells.");
        return;
    }

    int resources = getAdjacentResourceSum(row, col, activePlayer->getPlayer());

    if (resources < 2) {
        emit statusMessage(QString("ERROR: Not enough resources in adjacent cells. Found %1, minimum is 2.").arg(resources));
        return;
    }

    QHash<PieceType, int> currentCosts;
    currentCosts[PieceType::Footman] = calculateRecruitmentCost(PieceType::Footman, activePlayer->getPlayer());
    currentCosts[PieceType::Vanguard] = calculateRecruitmentCost(PieceType::Vanguard, activePlayer->getPlayer());
    currentCosts[PieceType::Sentinel] = calculateRecruitmentCost(PieceType::Sentinel, activePlayer->getPlayer());
    currentCosts[PieceType::Rindall] = calculateRecruitmentCost(PieceType::Rindall, activePlayer->getPlayer());
    currentCosts[PieceType::Worker] = calculateRecruitmentCost(PieceType::Worker, activePlayer->getPlayer());

    RecruitmentDialog dialog(
        activePlayer->getPlayer(),
        resources,
        currentCosts,
        playerStates[activePlayer->getPlayer()].workersReplaced,
        playerStates[activePlayer->getPlayer()].canRecruitLastLost,
        playerStates[activePlayer->getPlayer()].lastLostPieceType,
        board
        );

    if (dialog.exec() == QDialog::Accepted && dialog.wasConfirmed()) {
        PieceType selectedType = dialog.getSelectedPieceType();

        if (canRecruitPiece(selectedType, activePlayer->getPlayer(), row, col, resources)) {
            recruitPiece(selectedType, activePlayer->getPlayer(), row, col);
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
    if (playerStates[player].isInSize) {
        return false;
    }

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

    QString pieceSymbol;
    switch(type) {
    case PieceType::Footman: pieceSymbol = "F"; break;
    case PieceType::Vanguard: pieceSymbol = "V"; break;
    case PieceType::Sentinel: pieceSymbol = "S"; break;
    case PieceType::Rindall: pieceSymbol = "R"; break;
    case PieceType::Worker: pieceSymbol = "W"; break;
    default: pieceSymbol = "?"; break;
    }

    QString notation = QString("%1@%2")
                           .arg(pieceSymbol)
                           .arg(posToString(row, col));

    HumanPlayer* human = dynamic_cast<HumanPlayer*>(activePlayer);
    if (human) {
        human->processHumanMove(notation);
    }
}


void GameController::checkAndEvolve(PieceWidget* pieceWidget, int row, int col) {
    if (!pieceWidget) return;

    Piece* piece = pieceWidget->getPiece();

    if (piece->getType() == PieceType::Footman && piece->getKillCount() >= 3) {
        evolvePiece(row, col, PieceType::Champion);
        emit alertMessage(QString("EVOLUTION! Footman evolved to Champion at %1!")
                              .arg(posToString(row, col)));
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
                emit alertMessage(QString("SUPREME EVOLUTION! Worker ascended to Ascendant at %1!")
                                      .arg(posToString(row, col)));
            }
        }
    }
}

void GameController::evolvePiece(int row, int col, PieceType newType) {
    AudioManager::instance().playSoundEffect(SoundEffect::PieceEvolved);

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
        if (pieceWidget->getPiece()->getPlayer() != activePlayer->getPlayer()) {
            emit statusMessage("ERROR: Not your piece.");
            return;
        }

        if (playerStates[activePlayer->getPlayer()].isInSize) {
            PieceType type = pieceWidget->getPiece()->getType();
            if (type == PieceType::Daick) {
                emit statusMessage("SIZE MODE: Only Daick can move to escape danger.");
            } else {
                int totalPieces = countPlayerPieces(activePlayer->getPlayer());
                int allowedMoves = (totalPieces + 1) / 2;

                emit statusMessage(QString("SIZE MODE: Limited movement (%1/%2 pieces can move)").arg(allowedMoves).arg(totalPieces));
            }
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

        performDestructionAnimation(target, targetRow, targetCol);
        AudioManager::instance().playSoundEffect(SoundEffect::PieceAttacked);

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
            emit statusMessage(QString("...Defesa da casa %1 absorveu %2 de dano.")
                                   .arg(posToString(targetRow, targetCol)).arg(damageToCell));
        }

        if (damageToApply > 0 && pieceDef > 0) {
            int newPieceDef = qMax(0, pieceDef - damageToApply);
            target->getPiece()->setCurrentDefense(newPieceDef);
            emit statusMessage(QString("...%1 sofreu %2 de dano em sua defesa! (Defesa restante: %3)")
                                   .arg(target->getPiece()->getFullName())
                                   .arg(damageToApply)
                                   .arg(newPieceDef));

            if (newPieceDef == 0 && target->getPiece()->getType() == PieceType::Bastion) {
                emit alertMessage(QString("A defesa do Bastion em %1 foi quebrada!").arg(posToString(targetRow, targetCol)));
            }
        }

        board->updateCellDisplay(targetRow, targetCol);
        AudioManager::instance().playSoundEffect(SoundEffect::PieceAttacked);
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
        PieceType destroyedType = pieceWidget->getPiece()->getType();
        Player destroyedPlayer = pieceWidget->getPiece()->getPlayer();

        board->removePieceFromCell(row, col);
        pieces.remove(QPoint(row, col));
        board->updateCellDisplay(row, col);

        if (destroyedType == PieceType::Daick) {
            Player winner = (destroyedPlayer == Player::Player1) ? Player::Player2 : Player::Player1;
            endGame(winner, "Daick's Fall");
        } else if (destroyedType == PieceType::Ascendant) {
            Player opponent = (destroyedPlayer == Player::Player1) ? Player::Player2 : Player::Player1;
            for (int r = 0; r < 12; ++r) {
                for (int c = 0; c < 12; ++c) {
                    Cell* cell = board->getCellData(r, c);
                    if (cell && cell->isUnderControl() && cell->getController() == opponent) {
                        for (int i = 0; i < 10; ++i) {
                            cell->addResource(opponent);
                        }
                    }
                }
            }
            emit alertMessage("ASCENDANT DESTROYED! Opponent gains 10 resources in all controlled cells!");
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
        PieceWidget* targetPiece = board->getPieceAt(row, col);
        bool isCapture = (targetPiece && targetPiece->getPiece()->getPlayer() != selectedPiece->getPiece()->getPlayer());

        if (isCapture && isProtectedByBastion(row, col, targetPiece->getPiece()->getPlayer())) {
            emit statusMessage("ERROR: Target protected by Bastion! Attack the Bastion first.");
            board->clearHighlights();
            selectedPiece = nullptr;
            return;
        }

        QString notation = generateMoveNotation(selectedPiece, selectedPos.x(), selectedPos.y(), row, col, isCapture);

        HumanPlayer* human = dynamic_cast<HumanPlayer*>(activePlayer);
        if (human) {
            human->processHumanMove(notation);
        }

    } else {
        emit statusMessage("ERROR: Invalid move. Selection cleared.");
    }

    board->clearHighlights();
    selectedPiece = nullptr;
}

void GameController::performRetroAnimation(PieceWidget* pieceWidget, int fromRow, int fromCol, int toRow, int toCol, bool isCapture) {
    pieceWidget->hide();

    QTimer::singleShot(100, [this, pieceWidget, fromRow, fromCol, toRow, toCol]() {
        pieceWidget->show();

        QTimer::singleShot(100, [this, pieceWidget, fromRow, fromCol, toRow, toCol]() {
            pieces.remove(QPoint(fromRow, fromCol));
            pieces[QPoint(toRow, toCol)] = pieceWidget;

            board->movePiece(fromRow, fromCol, toRow, toCol);

            updateCellControl(toRow, toCol, activePlayer->getPlayer());

            board->clearHighlights();

            pieceWidget->hide();
            QTimer::singleShot(80, [this, pieceWidget, toRow, toCol]() {
                pieceWidget->show();

                checkAndEvolve(pieceWidget, toRow, toCol);

                selectedPiece = nullptr;
                updateBastionProtection();
                isAnimating = false;
                endTurn();
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
    if (!gameActive) return;

    playerStates[activePlayer->getPlayer()].canRecruitLastLost = true;

    QString alertStr = checkResourceGeneration();
    QString evolutionStr = checkEvolutionConditions();

    if (!evolutionStr.isEmpty()) {
        if (!alertStr.isEmpty()) alertStr += "\n";
        alertStr += evolutionStr;
    }

    checkVictoryConditions();

    if (!gameActive) return;

    emit alertMessage(alertStr);

    activePlayer = (activePlayer == player1) ? player2 : player1;

    if (activePlayer == player1) {
        turnNumber++;
    }

    emit turnChanged(activePlayer->getPlayer(), turnNumber);

    QString playerStr = (activePlayer->getPlayer() == Player::Player1) ? "Player 1" : "Player 2";
    if (alertStr.isEmpty()) {
        emit statusMessage(QString("TURN %1: %2's turn. Select a piece or recruit.").arg(turnNumber).arg(playerStr));
    }

    board->updateAllCellDisplays();

    activePlayer->requestMove(generateFullGameStateNotation());
}

QString GameController::checkResourceGeneration() {
    if (playerStates[activePlayer->getPlayer()].isInSize) {
        for (int row = 0; row < 12; ++row) {
            for (int col = 0; col < 12; ++col) {
                Cell* cell = board->getCellData(row, col);
                if (cell && cell->hasOwner() && cell->getResourceOwner() == activePlayer->getPlayer()) {
                    cell->removeResource();
                }
            }
        }
        return "SIZE DETERIORATION: Lost 1 resource per cell.";
    }

    bool resourceBlocked = false;
    bool resourceGained = false;
    bool defenseFull = false;

    Player opponent = (activePlayer->getPlayer() == Player::Player1) ? Player::Player2 : Player::Player1;

    for (int row = 0; row < 12; ++row) {
        for (int col = 0; col < 12; ++col) {
            PieceWidget* piece = board->getPieceAt(row, col);

            if (!piece || piece->getPiece()->getPlayer() != activePlayer->getPlayer()) {
                continue;
            }

            PieceType type = piece->getPiece()->getType();

            if (type == PieceType::Worker || type == PieceType::Ascendant) {
                Cell* cell = board->getCellData(row, col);

                if (cell && cell->isUnderControl() && cell->getController() == activePlayer->getPlayer()) {
                    if (cell->getDefense() >= 5) {
                        defenseFull = true;
                    } else if (isSentinelBlockingCell(row, col, opponent)) {
                        resourceBlocked = true;
                    } else {
                        cell->addResource(activePlayer->getPlayer());
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

QString GameController::checkEvolutionConditions()
{
    QString alerts;

    for (int row = 0; row < 12; ++row) {
        for (int col = 0; col < 12; ++col) {
            PieceWidget* piece = board->getPieceAt(row, col);
            if (piece && piece->getPiece()->getPlayer() == activePlayer->getPlayer()) {
                if (piece->getPiece()->getType() == PieceType::Footman &&
                    piece->getPiece()->getKillCount() >= 3) {
                    if (!alerts.isEmpty()) alerts += "\n";
                    alerts += QString("EVOLUTION READY: Footman at %1 can evolve to Champion (3 kills)!")
                                  .arg(posToString(row, col));
                }

                if (piece->getPiece()->getType() == PieceType::Worker) {
                    if (piece->getPiece()->getTurnsUnderAttack() >= 8 &&
                        piece->getPiece()->getResourcesAccumulated() >= 20 &&
                        !playerStates[activePlayer->getPlayer()].hasAscendant) {
                        if (!alerts.isEmpty()) alerts += "\n";
                        alerts += QString("ASCENSION READY: Worker at %1 can become Ascendant!")
                                      .arg(posToString(row, col));
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

QString GameController::generateMoveNotation(PieceWidget* piece, int fromRow, int fromCol, int toRow, int toCol, bool isCapture) const
{
    QString pieceSymbol = piece->getPiece()->getSymbol();
    QString fromStr = posToString(fromRow, fromCol);
    QString captureMark = isCapture ? "x" : "";
    QString toStr = posToString(toRow, toCol);

    return QString("%1%2%3%4").arg(pieceSymbol).arg(fromStr).arg(captureMark).arg(toStr);
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
    stream << "Jogador Atual: " << (activePlayer->getPlayer() == Player::Player1 ? "Player 1 (Brancas)" : "Player 2 (Pretas)") << "\n";
    stream << "Status do Jogo: " << (gameActive ? "Em Andamento" : "FINALIZADO") << "\n\n";

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
                QString cellPos = posToString(r, c);
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
        stream << QString(" - Estado SIZE: %1\n").arg(playerStates[player].isInSize ? "SIM" : "Não");
        if (playerStates[player].isInSize) {
            stream << QString(" - Turnos em SIZE: %1/3\n").arg(playerStates[player].turnsInSize);
        }
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

    if (bastion->getPiece()->getPlayer() != activePlayer->getPlayer()) {
        emit statusMessage("ERROR: Not your piece.");
        return;
    }

    if (bastion->getPiece()->getCurrentDefense() >= bastion->getPiece()->getDefensePower()) {
        emit statusMessage("BASTION: Already at full defense. No rehabilitation needed.");
        return;
    }

    if (!hasAdjacentWorker(row, col, activePlayer->getPlayer())) {
        emit statusMessage("ERROR: Reabilitação falhou. Requer um Worker aliado adjacente.");
        return;
    }

    int defenseMissing = bastion->getPiece()->getDefensePower() - bastion->getPiece()->getCurrentDefense();
    int repairCost = calculateBastionRepairCost(defenseMissing, activePlayer->getPlayer());

    int availableResources = getAdjacentResourceSum(row, col, activePlayer->getPlayer());
    if (availableResources < repairCost) {
        emit statusMessage(QString("ERROR: Reabilitação falhou. Recursos insuficientes. (Requer: %1, Disponível: %2)")
                               .arg(repairCost)
                               .arg(availableResources));
        return;
    }

    QString notation = QString("B_REPAIR@%1").arg(posToString(row, col));

    HumanPlayer* human = dynamic_cast<HumanPlayer*>(activePlayer);
    if (human) {
        human->processHumanMove(notation);
    }
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

void GameController::checkVictoryConditions() {
    if (!gameActive) return;

    if (checkDaicksFall()) {
        return;
    }

    if (checkCountdownToSize(Player::Player1)) {
        endGame(Player::Player1, "Countdown to Size");
        return;
    }
    if (checkCountdownToSize(Player::Player2)) {
        endGame(Player::Player2, "Countdown to Size");
        return;
    }


    Player opponent = (activePlayer->getPlayer() == Player::Player1) ? Player::Player2 : Player::Player1;

    checkSizeCondition(activePlayer->getPlayer());
    checkSizeCondition(opponent);

    if (playerStates[opponent].isInSize) {
        playerStates[opponent].turnsInSize++;

        if (playerStates[opponent].turnsInSize >= 5) {
            endGame(activePlayer->getPlayer(), "Size's Victory");
            return;
        } else {
            emit alertMessage(QString("WARNING: %1 is in SIZE state! (%2/5 turns)")
                                  .arg(opponent == Player::Player1 ? "Player 1" : "Player 2")
                                  .arg(playerStates[opponent].turnsInSize));
        }
    }

    int currentResources = getTotalPlayerResources(activePlayer->getPlayer());

    playerStates[activePlayer->getPlayer()].lastTurnResourceCount = currentResources;
}

bool GameController::checkDaicksFall() {
    bool player1HasDaick = false;
    bool player2HasDaick = false;

    for (auto it = pieces.constBegin(); it != pieces.constEnd(); ++it) {
        PieceWidget* piece = it.value();
        if (piece && piece->getPiece()->getType() == PieceType::Daick) {
            if (piece->getPiece()->getPlayer() == Player::Player1) {
                player1HasDaick = true;
            } else {
                player2HasDaick = true;
            }
        }
    }

    if (!player1HasDaick) {
        endGame(Player::Player2, "Daick's Fall");
        return true;
    }

    if (!player2HasDaick) {
        endGame(Player::Player1, "Daick's Fall");
        return true;
    }

    return false;
}

bool GameController::checkCountdownToSize(Player winner) {
    Player loser = (winner == Player::Player1) ? Player::Player2 : Player::Player1;

    if (!playerStates[loser].isInSize) {
        return false;
    }

    bool hasRindall = false;
    bool hasVanguard = false;
    for (auto it = pieces.constBegin(); it != pieces.constEnd(); ++it) {
        PieceWidget* piece = it.value();
        if (piece && piece->getPiece()->getPlayer() == loser) {
            PieceType type = piece->getPiece()->getType();
            if (type == PieceType::Rindall) hasRindall = true;
            if (type == PieceType::Vanguard) hasVanguard = true;

            if (hasRindall && hasVanguard) break;
        }
    }

    if (hasRindall || hasVanguard) {
        return false;
    }

    int rindallCost = calculateRecruitmentCost(PieceType::Rindall, loser);
    int vanguardCost = calculateRecruitmentCost(PieceType::Vanguard, loser);

    int minCostToRebuild = qMin(rindallCost, vanguardCost);

    int totalEconomy = getTotalPlayerResources(loser);

    if (totalEconomy >= minCostToRebuild) {
        return false;
    }

    return true;
}

void GameController::checkSizeCondition(Player player) {
    bool hasWorker = hasWorkers(player);
    bool hasSentinel = hasSentinels(player);

    if (!hasWorker && !hasSentinel) {
        if (!playerStates[player].isInSize) {
            playerStates[player].isInSize = true;
            playerStates[player].turnsInSize = 0;
            AudioManager::instance().playSoundEffect(SoundEffect::AlertSize);
            emit alertMessage(QString("CRITICAL: %1 entered SIZE state! No Workers or Sentinels remaining!")
                                  .arg(player == Player::Player1 ? "Player 1" : "Player 2"));
            emit alertMessage(QString("CRITICAL: %1 entered SIZE state! No Workers or Sentinels remaining!")
                                  .arg(player == Player::Player1 ? "Player 1" : "Player 2"));
        }
    } else {
        if (playerStates[player].isInSize) {
            playerStates[player].isInSize = false;
            playerStates[player].turnsInSize = 0;
            AudioManager::instance().playSoundEffect(SoundEffect::AlertGeneric);
            emit alertMessage(QString("%1 escaped SIZE state!")
                                  .arg(player == Player::Player1 ? "Player 1" : "Player 2"));
            emit alertMessage(QString("%1 escaped SIZE state!")
                                  .arg(player == Player::Player1 ? "Player 1" : "Player 2"));
        }
    }
}

bool GameController::hasWorkers(Player player) const {
    for (auto it = pieces.constBegin(); it != pieces.constEnd(); ++it) {
        PieceWidget* piece = it.value();
        if (piece && piece->getPiece()->getPlayer() == player) {
            PieceType type = piece->getPiece()->getType();
            if (type == PieceType::Worker || type == PieceType::Ascendant) {
                return true;
            }
        }
    }
    return false;
}

bool GameController::hasSentinels(Player player) const {
    for (auto it = pieces.constBegin(); it != pieces.constEnd(); ++it) {
        PieceWidget* piece = it.value();
        if (piece && piece->getPiece()->getPlayer() == player &&
            piece->getPiece()->getType() == PieceType::Sentinel) {
            return true;
        }
    }
    return false;
}

bool GameController::canPlayerMove(Player player) const {
    int movablePieces = 0;

    for (auto it = pieces.constBegin(); it != pieces.constEnd(); ++it) {
        PieceWidget* piece = it.value();
        if (piece && piece->getPiece()->getPlayer() == player) {
            QPoint pos = it.key();
            auto moves = piece->getPiece()->getPossibleMoves(pos.x(), pos.y(), board);
            if (!moves.isEmpty()) {
                movablePieces++;
            }
        }
    }

    return movablePieces > 0;
}

int GameController::countPlayerPieces(Player player) const {
    int count = 0;
    for (auto it = pieces.constBegin(); it != pieces.constEnd(); ++it) {
        if (it.value() && it.value()->getPiece()->getPlayer() == player) {
            count++;
        }
    }
    return count;
}

int GameController::getTotalPlayerResources(Player player) const {
    int total = 0;
    for (int row = 0; row < 12; ++row) {
        for (int col = 0; col < 12; ++col) {
            Cell* cell = board->getCellData(row, col);
            if (cell && cell->isUnderControl() && cell->getController() == player) {
                if (cell->hasOwner() && cell->getResourceOwner() == player) {
                    total += cell->getResources();
                }
            }
        }
    }
    return total;
}

void GameController::endGame(Player winner, const QString& victoryType) {
    if (!gameActive) return;

    gameActive = false;

    AudioManager::instance().playSoundEffect(SoundEffect::AlertVictory);

    board->clearHighlights();
    selectedPiece = nullptr;

    QString notation = QString("GAME_END:%1_WINS_BY_%2")
                           .arg(winner == Player::Player1 ? "WHITE" : "BLACK")
                           .arg(victoryType.toUpper().replace(" ", "_"));

    emit moveMade(notation, winner);

    displayVictoryScreen(winner, victoryType);
}

void GameController::displayVictoryScreen(Player winner, const QString& victoryType) {
    QString winnerName = (winner == Player::Player1) ? "PLAYER 1 (BRANCAS)" : "PLAYER 2 (PRETAS)";

    QString victoryMessage = QString("═══════════════════════════════\n"
                                     "     VITÓRIA!\n"
                                     "═══════════════════════════════\n\n"
                                     "%1\n"
                                     "VENCEU POR:\n"
                                     "%2\n\n"
                                     "Turno Final: %3\n"
                                     "═══════════════════════════════")
                                 .arg(winnerName)
                                 .arg(victoryType.toUpper())
                                 .arg(turnNumber);

    emit alertMessage(victoryMessage);
    emit statusMessage("GAME OVER - Match concluded.");

    for (int blink = 0; blink < 6; ++blink) {
        QTimer::singleShot(blink * 200, [this, blink]() {
            for (int row = 0; row < 12; ++row) {
                for (int col = 0; col < 12; ++col) {
                    QPushButton* cell = board->getCellButton(row, col);
                    if (cell) {
                        if (blink % 2 == 0) {
                            cell->setStyleSheet("QPushButton { background-color: black; color: white; border: 1px solid white; }");
                        } else {
                            board->updateCellDisplay(row, col);
                        }
                    }
                }
            }
        });
    }

    QTimer::singleShot(1200, [this]() {
        board->updateAllCellDisplays();
    });
}

void GameController::onMoveReceived(const QString& moveNotation)
{
    if (!gameActive || isAnimating) {
        return;
    }

    qDebug() << "GameController: Received move" << moveNotation << "from" << (activePlayer->getPlayer() == Player::Player1 ? "P1" : "P2");

    bool parseSuccess = parseAndExecuteMove(moveNotation);

    if (parseSuccess) {
        emit moveMade(moveNotation, activePlayer->getPlayer());
    } else {
        emit statusMessage(QString("ERROR: Invalid move received: %1").arg(moveNotation));
        qWarning() << "Invalid move received:" << moveNotation;
        activePlayer->requestMove(generateFullGameStateNotation());
    }
}

bool GameController::parseAndExecuteMove(const QString& notation)
{
    if (notation.contains('@')) {
        QRegularExpression re("([A-Z])@([A-L][0-9]{1,2})");
        QRegularExpressionMatch match = re.match(notation);

        if (notation.startsWith("B_REPAIR@")) {
            QPoint pos = stringToPos(notation.mid(9));
            return executeBastionRepair(pos.x(), pos.y());
        }
        else if (match.hasMatch()) {
            QString pieceSymbol = match.captured(1);
            QPoint pos = stringToPos(match.captured(2));

            PieceType type;
            if (pieceSymbol == "F") type = PieceType::Footman;
            else if (pieceSymbol == "V") type = PieceType::Vanguard;
            else if (pieceSymbol == "S") type = PieceType::Sentinel;
            else if (pieceSymbol == "R") type = PieceType::Rindall;
            else if (pieceSymbol == "W") type = PieceType::Worker;
            else return false;

            return executeRecruitment(type, pos.x(), pos.y());
        }
    }
    else {
        QRegularExpression re("([A-Z])([A-L][0-9]{1,2})(x?)([A-L][0-9]{1,2})");
        QRegularExpressionMatch match = re.match(notation);

        if (match.hasMatch()) {
            QString pieceSymbol = match.captured(1);
            QPoint fromPos = stringToPos(match.captured(2));
            bool isCapture = !match.captured(3).isEmpty();
            QPoint toPos = stringToPos(match.captured(4));

            PieceWidget* piece = board->getPieceAt(fromPos.x(), fromPos.y());

            if (!piece || piece->getPiece()->getPlayer() != activePlayer->getPlayer() || piece->getPiece()->getSymbol() != pieceSymbol) {
                qWarning() << "Move parse error: Piece mismatch" << notation;
                return false;
            }

            auto moves = piece->getPiece()->getPossibleMoves(fromPos.x(), fromPos.y(), board);
            bool isValidMove = false;
            for (const auto& move : std::as_const(moves)) {
                if (move.first == toPos.x() && move.second == toPos.y()) {
                    isValidMove = true;
                    break;
                }
            }

            if (!isValidMove) {
                qWarning() << "Move parse error: Not a valid move" << notation;
                return false;
            }

            return executeMove(piece, fromPos.x(), fromPos.y(), toPos.x(), toPos.y(), isCapture);
        }
    }

    qWarning() << "Move parse error: Unrecognized format" << notation;
    return false;
}

bool GameController::executeMove(PieceWidget* piece, int fromRow, int fromCol, int toRow, int toCol, bool isCapture)
{
    isAnimating = true;

    if (isCapture) {
        bool attackSuccess = executeAttack(piece, fromRow, fromCol, toRow, toCol);

        if (attackSuccess) {
            QTimer::singleShot(500, [this, piece, fromRow, fromCol, toRow, toCol, isCapture]() {
                performRetroAnimation(piece, fromRow, fromCol, toRow, toCol, isCapture);
            });
            return true;
        } else {
            isAnimating = false;
            endTurn();
            return true;
        }
    } else {
        performRetroAnimation(piece, fromRow, fromCol, toRow, toCol, isCapture);
        return true;
    }
}

bool GameController::executeRecruitment(PieceType type, int row, int col)
{
    Player player = activePlayer->getPlayer();
    int availableResources = getAdjacentResourceSum(row, col, player);

    if (!canRecruitPiece(type, player, row, col, availableResources)) {
        qWarning() << "executeRecruitment: canRecruitPiece check failed.";
        return false;
    }

    int cost = calculateRecruitmentCost(type, player);
    spendResourcesFromAdjacent(row, col, player, cost);

    placePiece(row, col, type, player);

    AudioManager::instance().playSoundEffect(SoundEffect::RecruitPiece);

    if (type == PieceType::Worker) {
        playerStates[player].workersReplaced++;
    }

    playerStates[player].piecesRecruited[type]++;

    emit statusMessage(QString("RECRUITED: %1 at %2 for %3 resources.")
                           .arg(posToString(row, col))
                           .arg(cost));

    endTurn();
    return true;
}

bool GameController::executeBastionRepair(int row, int col)
{
    Player player = activePlayer->getPlayer();
    PieceWidget* bastion = board->getPieceAt(row, col);

    if (!bastion || bastion->getPiece()->getType() != PieceType::Bastion || bastion->getPiece()->getPlayer() != player) {
        qWarning() << "executeBastionRepair: Not a valid bastion.";
        return false;
    }

    if (bastion->getPiece()->getCurrentDefense() >= bastion->getPiece()->getDefensePower()) {
        emit statusMessage("BASTION: Already at full defense. No rehabilitation needed.");
        return false;
    }

    if (!hasAdjacentWorker(row, col, player)) {
        emit statusMessage("ERROR: Reabilitação falhou. Requer um Worker aliado adjacente.");
        return false;
    }

    int defenseMissing = bastion->getPiece()->getDefensePower() - bastion->getPiece()->getCurrentDefense();
    int repairCost = calculateBastionRepairCost(defenseMissing, player);

    int availableResources = getAdjacentResourceSum(row, col, player);
    if (availableResources < repairCost) {
        emit statusMessage(QString("ERROR: Reabilitação falhou. Recursos insuficientes. (Requer: %1, Disponível: %2)")
                               .arg(repairCost)
                               .arg(availableResources));
        return false;
    }

    spendResourcesFromAdjacent(row, col, player, repairCost);
    bastion->getPiece()->setCurrentDefense(bastion->getPiece()->getDefensePower());
    playerStates[player].bastionRepairs++;

    board->updateCellDisplay(row, col);

    emit statusMessage(QString("AÇÃO: Bastion reabilitado por %1 recursos! (Inflação de reparo agora: %2)")
                           .arg(repairCost)
                           .arg(playerStates[player].bastionRepairs));

    endTurn();
    return true;
}

QPoint GameController::stringToPos(QString pos) const
{
    if (pos.isEmpty()) return QPoint(-1, -1);
    int col = pos[0].toUpper().toLatin1() - 'A';
    int row = 12 - pos.mid(1).toInt();
    return QPoint(row, col);
}

QString GameController::posToString(int row, int col) const
{
    return QString("%1%2").arg(QChar('A' + col)).arg(12 - row);
}
