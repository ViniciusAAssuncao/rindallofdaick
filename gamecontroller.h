#ifndef GAMECONTROLLER_H
#define GAMECONTROLLER_H

#include <QObject>
#include <QHash>
#include <QPoint>
#include "pieces.h"
#include "piecewidget.h"
#include "infopanel.h"
#include "abstractplayer.h"
#include "humanplayer.h"
#include "engineplayer.h"
#include "tacticalmanager.h"

class Board;

struct BaseCosts {
    const int footman = 2;
    const int vanguard = 4;
    const int sentinel = 3;
    const int rindall = 10;
    const int bastion = 6;
};

struct PlayerState {
    int workersReplaced = 0;
    bool hasAscendant = false;
    PieceType lastLostPieceType = PieceType::Footman;
    bool canRecruitLastLost = true;
    QHash<PieceType, int> piecesRecruited;
    int bastionRepairs = 0;
    bool isInSize = false;
    int turnsInSize = 0;
    int passiveTurns = 0;
    int lastTurnResourceCount = 0;
    bool footmanToWorkerUsed = false;

    PlayerState()
        : workersReplaced(0),
        hasAscendant(false),
        lastLostPieceType(PieceType::Footman),
        canRecruitLastLost(true),
        bastionRepairs(0),
        footmanToWorkerUsed(false)
    {
    }
};

class GameController : public QObject
{
    Q_OBJECT

public:
    explicit GameController(Board* board, InfoPanel* panel, QObject* parent = nullptr);
    void initializeGame();
    void setEnginePaths(const QString& p1Path, const QString& p2Path);
    void placePiece(int row, int col, PieceType type, Player player);
    void onCopyLogRequested();
    void onDownloadLogRequested();
    void onDownloadLogToDesktopRequested();

signals:
    void statusMessage(const QString& message);
    void turnChanged(Player player, int turn);
    void alertMessage(const QString& message);
    void moveMade(const QString& moveNotation, Player player);
    void sendLogToClipboard(const QString& text);

public slots:
    void handleCellClicked(int row, int col);
    void handleRightClickAction(int row, int col);
    void onMoveReceived(const QString& moveNotation);
    void handleRightClickOnEmptyCell(int row, int col);
private:
    void handleRecruitment(int row, int col);
    int calculateRecruitmentCost(PieceType type, Player player);
    bool canRecruitPiece(PieceType type, Player player, int row, int col, int availableResources);
    void recruitPiece(PieceType type, Player player, int row, int col);
    void checkAndEvolve(PieceWidget* pieceWidget, int row, int col);
    void evolvePiece(int row, int col, PieceType newType);
    QString checkEvolutionConditions();

    TacticalManager* tacticalManager;

    bool vanguardBonusMovePending;
    PieceWidget* vanguardBonusPiece;
    QPoint vanguardBonusStartPos;

    bool isValidPos(int row, int col) const;
    void highlightVanguardBonusMoves(int row, int col);
    void handleVanguardBonusMove(int toRow, int toCol);

    void setupInitialPieces();
    void selectPiece(int row, int col);
    void moveSelectedPieceTo(int row, int col);
    void performRetroAnimation(PieceWidget* pieceWidget, int fromRow, int fromCol, int toRow, int toCol, bool isCapture, bool endTurnAfterMove);
    void performDestructionAnimation(PieceWidget* pieceWidget, int row, int col);

    bool canAttack(PieceWidget* attacker, int targetRow, int targetCol);
    bool isProtectedByBastion(int row, int col, Player defender);
    int calculateTotalDefense(int row, int col);
    bool executeAttack(PieceWidget* attacker, int attackerRow, int attackerCol, int targetRow, int targetCol);
    void consumeCellResources(int row, int col, int attackPower);

    void endTurn();
    QString checkResourceGeneration();
    QString checkPassivity();
    bool isSentinelBlockingCell(int row, int col, Player resourcePlayer);
    void updateCellControl(int row, int col, Player player);
    void updateBastionProtection();
    QString generateMoveNotation(PieceWidget* piece, int fromRow, int fromCol, int toRow, int toCol, bool isCapture) const;
    QString generateFullGameStateNotation() const;

    int getAdjacentResourceSum(int row, int col, Player player) const;
    void spendResourcesFromAdjacent(int row, int col, Player player, int cost);

    void checkAndSendBastionRepair(int row, int col);
    void checkAndSendFootmanConversion(int row, int col);
    bool hasAdjacentWorker(int row, int col, Player player) const;
    bool hasAdjacentSentinel(int row, int col, Player player) const;
    int calculateBastionRepairCost(int pointsToRepair, Player player) const;

    void checkVictoryConditions();
    bool checkDaicksFall();
    bool checkCountdownToSize(Player winner);
    void checkSizeCondition(Player player);
    bool hasWorkers(Player player) const;
    bool hasSentinels(Player player) const;
    bool canPlayerMove(Player player) const;
    int countPlayerPieces(Player player) const;
    int getTotalPlayerResources(Player player) const;
    void endGame(Player winner, const QString& victoryType);
    void displayVictoryScreen(Player winner, const QString& victoryType);

    bool parseAndExecuteMove(const QString& notation);
    bool executeMove(PieceWidget* piece, int fromRow, int fromCol, int toRow, int toCol, bool isCapture);
    bool executeRecruitment(PieceType type, int row, int col);
    bool executeBastionRepair(int row, int col);
    bool executeFootmanConversion(int row, int col);

    QPoint stringToPos(QString pos) const;
    QString posToString(int row, int col) const;

    QHash<Player, PlayerState> playerStates;
    BaseCosts baseCosts;

    Board* board;
    InfoPanel* infoPanel;
    QHash<QPoint, PieceWidget*> pieces;
    PieceWidget* selectedPiece = nullptr;
    QPoint selectedPos;

    IPlayer* player1;
    IPlayer* player2;
    IPlayer* activePlayer;

    QString m_player1EnginePath;
    QString m_player2EnginePath;

    int turnNumber;
    bool isAnimating = false;
    bool gameActive = true;
};

#endif
