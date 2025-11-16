#include "piece.h"

Piece::Piece(PieceType type, Player player) : type(type), player(player), currentDefense(0) {}

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
    case PieceType::Champion: return "C";
    case PieceType::Ascendant: return "A";
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
    case PieceType::Champion: return "Champion (Elite Evoluído)";
    case PieceType::Ascendant: return "Ascendant (Worker Supremo)";
    default: return "Desconhecida";
    }
}

int Piece::getAttackPower() const {
    switch(type) {
    case PieceType::Daick: return 0;
    case PieceType::Rindall: return 5;
    case PieceType::Sentinel: return 3;
    case PieceType::Vanguard: return 4;
    case PieceType::Worker: return 1;
    case PieceType::Bastion: return 0;
    case PieceType::Footman: return 2;
    case PieceType::Champion: return 3;
    case PieceType::Ascendant: return 4;
    default: return 0;
    }
}

int Piece::getDefensePower() const {
    switch(type) {
    case PieceType::Bastion: return 7;
    default: return 0;
    }
}

int Piece::getCurrentDefense() const {
    return currentDefense;
}

void Piece::setCurrentDefense(int def) {
    if (def < 0) {
        currentDefense = 0;
    } else if (def > getDefensePower()) {
        currentDefense = getDefensePower();
    } else {
        currentDefense = def;
    }
}
