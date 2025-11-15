#include "piece.h"

Piece::Piece(PieceType type, Player player) : type(type), player(player) {}

PieceType Piece::getType() const { return type; }
Player Piece::getPlayer() const { return player; }

QString Piece::getSymbol() const {
    switch(type) {
    case PieceType::Daick: return "K";
    case PieceType::Rindall: return "R";
    case PieceType::Sentinel: return "S";
    case PieceType::Vanguard: return "V";
    case PieceType::Worker: return "W";
    case PieceType::Bastion: return "B";
    case PieceType::Footman: return "F";
    default: return "";
    }
}

QString Piece::getDisplayText() const {
    return getSymbol();
}

QString Piece::getFullName() const {
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
