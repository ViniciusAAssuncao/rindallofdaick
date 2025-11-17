#ifndef TACTICALMANAGER_H
#define TACTICALMANAGER_H

#include <QObject>
#include <QList>
#include <QPoint>
#include "piece.h"
#include "tacticalarrow.h"

class Board;
class GameController;

struct PendingMove {
    QPoint from;
    QPoint to;
    Player player;
};

class TacticalManager : public QObject
{
    Q_OBJECT

public:
    explicit TacticalManager(Board* board, QObject* parent = nullptr);
    ~TacticalManager();

    void handleMiddleClick(int row, int col);
    void clearAllArrows();
    void checkAndExecutePendingMoves(Player currentPlayer);
    void setGameController(GameController* controller);

signals:
    void sendTacticalMove(const QString& moveNotation);
    void statusMessage(const QString& message);

private:
    Board* board;
    GameController* gameController;
    QList<TacticalArrow*> arrows;
    QList<PendingMove> pendingMoves;
    QPoint firstClick;
    bool waitingForSecondClick;

    void createArrow(QPoint start, QPoint end, TacticalArrow::ArrowType type);
    bool isValidMove(QPoint from, QPoint to, Player player);
    bool hasPieceAtPosition(QPoint pos, Player player);

    TacticalArrow* findArrow(QPoint start, QPoint end);
    void removeArrow(QPoint start, QPoint end);
    bool isPotentialPreMove(QPoint from, QPoint to);
    void addPendingMove(QPoint from, QPoint to);
    void removePendingMove(QPoint from, QPoint to);
    void clearPendingMoves(Player player);

    QString posToString(int row, int col) const;
};

#endif
