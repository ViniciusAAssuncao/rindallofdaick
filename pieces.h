#ifndef PIECES_H
#define PIECES_H

#include "piece.h"

class Daick : public Piece
{
public:
    Daick(Player player) : Piece(PieceType::Daick, player) {}
    QList<QPair<int, int>> getPossibleMoves(int currentRow, int currentCol, const Board* board) const override;
    QString getFullName() const override { return "Daick (Rei)"; }
};

class Rindall : public Piece
{
public:
    Rindall(Player player) : Piece(PieceType::Rindall, player) {}
    QList<QPair<int, int>> getPossibleMoves(int currentRow, int currentCol, const Board* board) const override;
    QString getFullName() const override { return "Rindall (Elite)"; }
};

class Sentinel : public Piece
{
public:
    Sentinel(Player player) : Piece(PieceType::Sentinel, player) {}
    QList<QPair<int, int>> getPossibleMoves(int currentRow, int currentCol, const Board* board) const override;
    QString getFullName() const override { return "Sentinel (Bloqueador)"; }
};

class Vanguard : public Piece
{
public:
    Vanguard(Player player) : Piece(PieceType::Vanguard, player) {}
    QList<QPair<int, int>> getPossibleMoves(int currentRow, int currentCol, const Board* board) const override;
    QString getFullName() const override { return "Vanguard (Cavalaria)"; }
};

class Worker : public Piece
{
public:
    Worker(Player player) : Piece(PieceType::Worker, player) {}
    QList<QPair<int, int>> getPossibleMoves(int currentRow, int currentCol, const Board* board) const override;
    QString getFullName() const override { return "Worker (Trabalhador)"; }
};

class Bastion : public Piece
{
public:
    Bastion(Player player) : Piece(PieceType::Bastion, player) {
        currentDefense = 7;
    }
    QList<QPair<int, int>> getPossibleMoves(int currentRow, int currentCol, const Board* board) const override;
    QString getFullName() const override { return "Bastion (Fortaleza)"; }
};

class Footman : public Piece
{
public:
    Footman(Player player) : Piece(PieceType::Footman, player) {}
    QList<QPair<int, int>> getPossibleMoves(int currentRow, int currentCol, const Board* board) const override;
    QString getFullName() const override { return "Footman (Infantaria)"; }
};

class Champion : public Piece
{
public:
    Champion(Player player) : Piece(PieceType::Champion, player) {}
    QList<QPair<int, int>> getPossibleMoves(int currentRow, int currentCol, const Board* board) const override;
    QString getFullName() const override { return "Champion (Elite Evoluído)"; }
};

class Ascendant : public Piece
{
public:
    Ascendant(Player player) : Piece(PieceType::Ascendant, player) {}
    QList<QPair<int, int>> getPossibleMoves(int currentRow, int currentCol, const Board* board) const override;
    QString getFullName() const override { return "Ascendant (Worker Supremo)"; }
};

#endif
