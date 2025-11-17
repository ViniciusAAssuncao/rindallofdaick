#ifndef PIECE_H
#define PIECE_H
#include <QString>
#include <QList>
#include <QPair>

enum class PieceType {
    Daick,
    Rindall,
    Sentinel,
    Vanguard,
    Worker,
    Bastion,
    Footman,
    Champion,
    Ascendant
};

enum class Player {
    Player1,
    Player2
};

class Piece
{
public:
    Piece(PieceType type, Player player);
    virtual ~Piece() = default;

    PieceType getType() const;
    Player getPlayer() const;
    QString getSymbol() const;
    QString getDisplayText() const;
    virtual QString getFullName() const;

    virtual int getAttackPower() const;
    virtual int getDefensePower() const;

    int getCurrentDefense() const;
    void setCurrentDefense(int def);

    virtual QList<QPair<int, int>> getPossibleMoves(int currentRow, int currentCol, const class Board* board) const = 0;

    int getKillCount() const { return killCount; }
    void incrementKillCount() { killCount++; }

    int getTurnsUnderAttack() const { return turnsUnderAttack; }
    void setTurnsUnderAttack(int turns) { turnsUnderAttack = turns; }
    void incrementTurnsUnderAttack() { turnsUnderAttack++; }
    void resetTurnsUnderAttack() { turnsUnderAttack = 0; }

    int getResourcesAccumulated() const { return resourcesAccumulated; }
    void addResourcesAccumulated(int res) { resourcesAccumulated += res; }
    void resetResourcesAccumulated() { resourcesAccumulated = 0; }

protected:
    PieceType type;
    Player player;
    int killCount = 0;
    int turnsUnderAttack = 0;
    int resourcesAccumulated = 0;

    int currentDefense = 0;
};

#endif
