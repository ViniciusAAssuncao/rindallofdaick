#include "pieces.h"
#include "board.h"
#include "piecewidget.h"

bool isValidPos(int row, int col) {
    return row >= 0 && row < 12 && col >= 0 && col < 12;
}

QList<QPair<int, int>> Daick::getPossibleMoves(int currentRow, int currentCol, const Board* board) const {
    QList<QPair<int, int>> moves;

    int directions[8][2] = {
        {-1, -1}, {-1, 0}, {-1, 1},
        {0, -1},           {0, 1},
        {1, -1},  {1, 0}, {1, 1}
    };

    for (int i = 0; i < 8; ++i) {
        int newRow = currentRow + directions[i][0];
        int newCol = currentCol + directions[i][1];

        if (isValidPos(newRow, newCol)) {
            PieceWidget* targetPiece = board->getPieceAt(newRow, newCol);
            if (!targetPiece || targetPiece->getPiece()->getPlayer() != getPlayer()) {
                moves.append({newRow, newCol});
            }
        }
    }

    return moves;
}

QList<QPair<int, int>> Rindall::getPossibleMoves(int currentRow, int currentCol, const Board* board) const {
    QList<QPair<int, int>> moves;
    int directions[8][2] = {
        {-1, -1}, {-1, 0}, {-1, 1},
        {0, -1}, {0, 1},
        {1, -1}, {1, 0}, {1, 1}
    };
    for (int i = 0; i < 8; ++i) {
        int dr = directions[i][0];
        int dc = directions[i][1];
        int newRow = currentRow + dr;
        int newCol = currentCol + dc;
        while (isValidPos(newRow, newCol)) {
            PieceWidget* targetPiece = board->getPieceAt(newRow, newCol);
            if (!targetPiece) {
                moves.append({newRow, newCol});
            } else if (targetPiece->getPiece()->getPlayer() != getPlayer()) {
                moves.append({newRow, newCol});
                break;
            }
            newRow += dr;
            newCol += dc;
        }
    }
    return moves;
}

QList<QPair<int, int>> Sentinel::getPossibleMoves(int currentRow, int currentCol, const Board* board) const {
    QList<QPair<int, int>> moves;

    int directions[4][2] = {
        {-1, 0}, {1, 0}, {0, -1}, {0, 1}
    };

    for (int i = 0; i < 4; ++i) {
        for (int distance = 1; distance <= 2; ++distance) {
            int newRow = currentRow + directions[i][0] * distance;
            int newCol = currentCol + directions[i][1] * distance;

            if (isValidPos(newRow, newCol)) {
                PieceWidget* targetPiece = board->getPieceAt(newRow, newCol);

                if (!targetPiece) {
                    moves.append({newRow, newCol});
                } else if (targetPiece->getPiece()->getPlayer() != getPlayer()) {
                    moves.append({newRow, newCol});
                    break;
                } else {
                    break;
                }
            } else {
                break;
            }
        }
    }

    return moves;
}

QList<QPair<int, int>> Vanguard::getPossibleMoves(int currentRow, int currentCol, const Board* board) const {
    QList<QPair<int, int>> moves;

    int lMoves[8][2] = {
        {-2, -1}, {-2, 1}, {-1, -2}, {-1, 2},
        {1, -2},  {1, 2},  {2, -1},  {2, 1}
    };

    auto isClear = [&](int r, int c) {
        return isValidPos(r, c) && !board->getPieceAt(r, c);
    };

    for (int i = 0; i < 8; ++i) {
        int dr = lMoves[i][0];
        int dc = lMoves[i][1];
        int newRow = currentRow + dr;
        int newCol = currentCol + dc;

        if (!isValidPos(newRow, newCol)) {
            continue;
        }

        bool pathA_clear = false;
        bool pathB_clear = false;

        if (dr == -2 && dc == -1) {
            pathA_clear = isClear(currentRow - 1, currentCol) && isClear(currentRow - 2, currentCol);
            pathB_clear = isClear(currentRow, currentCol - 1) && isClear(currentRow - 1, currentCol - 1);
        } else if (dr == -2 && dc == 1) {
            pathA_clear = isClear(currentRow - 1, currentCol) && isClear(currentRow - 2, currentCol);
            pathB_clear = isClear(currentRow, currentCol + 1) && isClear(currentRow - 1, currentCol + 1);
        } else if (dr == -1 && dc == -2) {
            pathA_clear = isClear(currentRow - 1, currentCol) && isClear(currentRow - 1, currentCol - 1);
            pathB_clear = isClear(currentRow, currentCol - 1) && isClear(currentRow, currentCol - 2);
        } else if (dr == -1 && dc == 2) {
            pathA_clear = isClear(currentRow - 1, currentCol) && isClear(currentRow - 1, currentCol + 1);
            pathB_clear = isClear(currentRow, currentCol + 1) && isClear(currentRow, currentCol + 2);
        } else if (dr == 1 && dc == -2) {
            pathA_clear = isClear(currentRow + 1, currentCol) && isClear(currentRow + 1, currentCol - 1);
            pathB_clear = isClear(currentRow, currentCol - 1) && isClear(currentRow, currentCol - 2);
        } else if (dr == 1 && dc == 2) {
            pathA_clear = isClear(currentRow + 1, currentCol) && isClear(currentRow + 1, currentCol + 1);
            pathB_clear = isClear(currentRow, currentCol + 1) && isClear(currentRow, currentCol + 2);
        } else if (dr == 2 && dc == -1) {
            pathA_clear = isClear(currentRow + 1, currentCol) && isClear(currentRow + 2, currentCol);
            pathB_clear = isClear(currentRow, currentCol - 1) && isClear(currentRow + 1, currentCol - 1);
        } else if (dr == 2 && dc == 1) {
            pathA_clear = isClear(currentRow + 1, currentCol) && isClear(currentRow + 2, currentCol);
            pathB_clear = isClear(currentRow, currentCol + 1) && isClear(currentRow + 1, currentCol + 1);
        }

        if (pathA_clear || pathB_clear) {
            PieceWidget* targetPiece = board->getPieceAt(newRow, newCol);
            if (!targetPiece || targetPiece->getPiece()->getPlayer() != getPlayer()) {
                moves.append({newRow, newCol});
            }
        }
    }

    return moves;
}

QList<QPair<int, int>> Worker::getPossibleMoves(int currentRow, int currentCol, const Board* board) const {
    QList<QPair<int, int>> moves;

    int directions[8][2] = {
        {-1, -1}, {-1, 0}, {-1, 1},
        {0, -1},           {0, 1},
        {1, -1},  {1, 0}, {1, 1}
    };

    for (int i = 0; i < 8; ++i) {
        int newRow = currentRow + directions[i][0];
        int newCol = currentCol + directions[i][1];

        if (isValidPos(newRow, newCol)) {
            PieceWidget* targetPiece = board->getPieceAt(newRow, newCol);
            if (!targetPiece || targetPiece->getPiece()->getPlayer() != getPlayer()) {
                moves.append({newRow, newCol});
            }
        }
    }

    return moves;
}

QList<QPair<int, int>> Bastion::getPossibleMoves(int currentRow, int currentCol, const Board* board) const {
    QList<QPair<int, int>> moves;

    int directions[4][2] = {
        {-1, 0}, {1, 0}, {0, -1}, {0, 1}
    };

    for (int i = 0; i < 4; ++i) {
        int newRow = currentRow + directions[i][0];
        int newCol = currentCol + directions[i][1];

        if (isValidPos(newRow, newCol)) {
            PieceWidget* targetPiece = board->getPieceAt(newRow, newCol);
            if (!targetPiece || targetPiece->getPiece()->getPlayer() != getPlayer()) {
                moves.append({newRow, newCol});
            }
        }
    }

    return moves;
}

QList<QPair<int, int>> Footman::getPossibleMoves(int currentRow, int currentCol, const Board* board) const {
    QList<QPair<int, int>> moves;

    int directions[8][2] = {
        {-1, -1}, {-1, 0}, {-1, 1},
        {0, -1},           {0, 1},
        {1, -1},  {1, 0}, {1, 1}
    };

    for (int i = 0; i < 8; ++i) {
        int newRow = currentRow + directions[i][0];
        int newCol = currentCol + directions[i][1];

        if (isValidPos(newRow, newCol)) {
            PieceWidget* targetPiece = board->getPieceAt(newRow, newCol);
            if (!targetPiece || targetPiece->getPiece()->getPlayer() != getPlayer()) {
                moves.append({newRow, newCol});
            }
        }
    }

    return moves;
}

QList<QPair<int, int>> Champion::getPossibleMoves(int currentRow, int currentCol, const Board* board) const {
    QList<QPair<int, int>> moves;

    int directions[8][2] = {
        {-1, -1}, {-1, 0}, {-1, 1},
        {0, -1},           {0, 1},
        {1, -1},  {1, 0}, {1, 1}
    };

    for (int i = 0; i < 8; ++i) {
        int dr = directions[i][0];
        int dc = directions[i][1];
        int newRow = currentRow + dr;
        int newCol = currentCol + dc;

        while (isValidPos(newRow, newCol)) {
            PieceWidget* targetPiece = board->getPieceAt(newRow, newCol);

            if (!targetPiece) {
                moves.append({newRow, newCol});
            } else if (targetPiece->getPiece()->getPlayer() != getPlayer()) {
                moves.append({newRow, newCol});
            }

            newRow += dr;
            newCol += dc;
        }
    }

    return moves;
}

QList<QPair<int, int>> Ascendant::getPossibleMoves(int currentRow, int currentCol, const Board* board) const {
    QList<QPair<int, int>> moves;

    int directions[8][2] = {
        {-1, -1}, {-1, 0}, {-1, 1},
        {0, -1},           {0, 1},
        {1, -1},  {1, 0}, {1, 1}
    };

    for (int i = 0; i < 8; ++i) {
        int newRow = currentRow + directions[i][0];
        int newCol = currentCol + directions[i][1];

        if (isValidPos(newRow, newCol)) {
            PieceWidget* targetPiece = board->getPieceAt(newRow, newCol);
            if (!targetPiece || targetPiece->getPiece()->getPlayer() != getPlayer()) {
                moves.append({newRow, newCol});
            }
        }
    }

    return moves;
}
