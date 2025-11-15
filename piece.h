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
    Footman
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
    virtual QList<QPair<int, int>> getPossibleMoves(int currentRow, int currentCol, const class Board* board) const = 0;
protected:
    PieceType type;
    Player player;
};
#endif
