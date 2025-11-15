#ifndef PIECES_H
#define PIECES_H

#include "piece.h"

class Daick : public Piece
{
public:
    Daick(Player player) : Piece(PieceType::Daick, player) {}
    QList<QPair<int, int>> getPossibleMoves(int currentRow, int currentCol, const Board* board) const override;
};

class Rindall : public Piece
{
public:
    Rindall(Player player) : Piece(PieceType::Rindall, player) {}
    QList<QPair<int, int>> getPossibleMoves(int currentRow, int currentCol, const Board* board) const override;
};

class Sentinel : public Piece
{
public:
    Sentinel(Player player) : Piece(PieceType::Sentinel, player) {}
    QList<QPair<int, int>> getPossibleMoves(int currentRow, int currentCol, const Board* board) const override;
};

class Vanguard : public Piece
{
public:
    Vanguard(Player player) : Piece(PieceType::Vanguard, player) {}
    QList<QPair<int, int>> getPossibleMoves(int currentRow, int currentCol, const Board* board) const override;
};

class Worker : public Piece
{
public:
    Worker(Player player) : Piece(PieceType::Worker, player) {}
    QList<QPair<int, int>> getPossibleMoves(int currentRow, int currentCol, const Board* board) const override;
};

class Bastion : public Piece
{
public:
    Bastion(Player player) : Piece(PieceType::Bastion, player) {}
    QList<QPair<int, int>> getPossibleMoves(int currentRow, int currentCol, const Board* board) const override;
};

class Footman : public Piece
{
public:
    Footman(Player player) : Piece(PieceType::Footman, player) {}
    QList<QPair<int, int>> getPossibleMoves(int currentRow, int currentCol, const Board* board) const override;

    virtual QString getFullName() const {
        switch(type) {
        case PieceType::Daick: return "Daick (Rei)";
        case PieceType::Rindall: return "Rindall (Elite)";
        case PieceType::Sentinel: return "Sentinel (Bloqueador)";
        case PieceType::Vanguard: return "Vanguard (Cavalaria)";
        case PieceType::Worker: return "Worker (Trabalhador)";
        case PieceType::Bastion: return "Bastion (Fortaleza)";
        case PieceType::Footman: return "Footman (Infantaria)";
        default: return "Desconhecida";
        }
    }
};

#endif
