#include "board.h"
#include <QFont>
#include <vector>
#include <QTimer>
Board::Board(QWidget *parent) : QWidget(parent)
{
    setupGrid();
}
void Board::setupGrid()
{
    const int cellSize = 50;
    const int labelSizeSide = 30;
    layout = new QGridLayout(this);
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);
    QFont retroFont("MS Sans Serif", 10, QFont::Bold);
    for (int col = 0; col < 12; ++col) {
        QLabel *label = new QLabel(QString(QChar('A' + col)));
        label->setAlignment(Qt::AlignCenter);
        label->setFont(retroFont);
        label->setStyleSheet("QLabel { background-color: white; color: black; border: 1px solid black; margin: 0px; padding: 0px; }");
        label->setFixedSize(cellSize, labelSizeSide);
        layout->addWidget(label, 0, col + 1);
    }
    for (int row = 0; row < 12; ++row) {
        QLabel *label = new QLabel(QString::number(12 - row));
        label->setAlignment(Qt::AlignCenter);
        label->setFont(retroFont);
        label->setStyleSheet("QLabel { background-color: white; color: black; border: 1px solid black; margin: 0px; padding: 0px; }");
        label->setFixedSize(labelSizeSide, cellSize);
        layout->addWidget(label, row + 1, 0);
    }
    for (int row = 0; row < 12; ++row) {
        for (int col = 0; col < 12; ++col) {
            QPushButton *cell = new QPushButton();
            cell->setFixedSize(cellSize, cellSize);
            cell->setStyleSheet("QPushButton { background-color: white; border: 1px solid black; margin: 0px; padding: 0px; color: black; }"
                                "QPushButton:hover { background-color: #f0f0f0; }");
            cell->setFont(retroFont);
            layout->addWidget(cell, row + 1, col + 1);
            cells[row][col] = cell;
            connect(cell, &QPushButton::clicked, this, &Board::onCellClicked);
            if (row <= 1) {
                cellData[row][col].setController(Player::Player2);
            } else if (row >= 10) {
                cellData[row][col].setController(Player::Player1);
            }
            updateCellTooltip(row, col);
        }
    }
    this->setFixedSize(labelSizeSide + 12 * cellSize, labelSizeSide + 12 * cellSize);
}
void Board::onCellClicked()
{
    QPushButton *button = qobject_cast<QPushButton*>(sender());
    if (button) {
        for (int row = 0; row < 12; ++row) {
            for (int col = 0; col < 12; ++col) {
                if (cells[row][col] == button) {
                    emit cellClicked(row, col);
                    return;
                }
            }
        }
    }
}
void Board::addPieceToCell(int row, int col, PieceWidget* pieceWidget) {
    if (row >= 0 && row < 12 && col >= 0 && col < 12) {
        layout->addWidget(pieceWidget, row + 1, col + 1);
        pieceWidget->show();
        pieceMap[{row, col}] = pieceWidget;
        updateCellTooltip(row, col);
    }
}
void Board::removePieceFromCell(int row, int col) {
    if (auto piece = getPieceAt(row, col)) {
        piece->setToolTip("");
        layout->removeWidget(piece);
        pieceMap.remove({row, col});
        updateCellTooltip(row, col);
    }
}
void Board::movePiece(int fromRow, int fromCol, int toRow, int toCol) {
    if (auto piece = getPieceAt(fromRow, fromCol)) {
        removePieceFromCell(fromRow, fromCol);
        addPieceToCell(toRow, toCol, piece);
    }
}
PieceWidget* Board::getPieceAt(int row, int col) const {
    return pieceMap.value({row, col}, nullptr);
}
void Board::highlightCells(const QList<QPair<int, int>>& cellsToHighlight) {
    clearHighlights();
    for (const auto& cell : cellsToHighlight) {
        int row = cell.first;
        int col = cell.second;
        if (row >= 0 && row < 12 && col >= 0 && col < 12) {
            cells[row][col]->setStyleSheet(
                "QPushButton { background-color: black; border: 1px solid white; margin: 0px; padding: 0px; color: white; }"
                "QPushButton:hover { background-color: #333333; border: 1px solid white; }"
                );
        }
    }
}
void Board::clearHighlights() {
    for (int row = 0; row < 12; ++row) {
        for (int col = 0; col < 12; ++col) {
            updateCellDisplay(row, col);
        }
    }
}
QPushButton* Board::getCellButton(int row, int col) const {
    if (row >= 0 && row < 12 && col >= 0 && col < 12) {
        return cells[row][col];
    }
    return nullptr;
}
Cell* Board::getCellData(int row, int col) {
    if (row >= 0 && row < 12 && col >= 0 && col < 12) {
        return &cellData[row][col];
    }
    return nullptr;
}
QString Board::generateCellTooltip(int row, int col) const {
    QString position = QString("Casa %1%2").arg(QChar('A' + col)).arg(12 - row);

    QString controller;
    if (cellData[row][col].isUnderControl()) {
        controller = (cellData[row][col].getController() == Player::Player1) ? "Player1" : "Player2";
    } else {
        controller = "Neutra";
    }

    QString resources = QString("Recursos: %1, Defesa: %2")
                            .arg(cellData[row][col].getResources())
                            .arg(cellData[row][col].getDefense());

    QString pieceInfo = "Nenhuma";
    QString pieceDetails = "";
    PieceWidget* currentPiece = getPieceAt(row, col);

    if (currentPiece) {
        pieceInfo = currentPiece->getPiece()->getDisplayText();
        pieceDetails = currentPiece->getPiece()->getFullName();
    }

    QString blockingReasons;
    if (cellData[row][col].getResources() > 0) {
        blockingReasons = "• Recursos presentes\n";
    }

    if (currentPiece) {
        PieceType type = currentPiece->getPiece()->getType();

        if (type == PieceType::Sentinel) {
            blockingReasons += "• Sentinel bloqueando área\n";
        }

        else if (type == PieceType::Worker) {
            Player workerPlayer = currentPiece->getPiece()->getPlayer();
            Player opponentPlayer = (workerPlayer == Player::Player1) ? Player::Player2 : Player::Player1;
            bool isBlocked = false;

            for (int dr = -2; dr <= 2; ++dr) {
                for (int dc = -2; dc <= 2; ++dc) {
                    if (dr == 0 && dc == 0) continue;

                    int checkRow = row + dr;
                    int checkCol = col + dc;

                    if (checkRow >= 0 && checkRow < 12 && checkCol >= 0 && checkCol < 12) {
                        PieceWidget* piece = getPieceAt(checkRow, checkCol);
                        if (piece && piece->getPiece()->getType() == PieceType::Sentinel) {
                            if (piece->getPiece()->getPlayer() == opponentPlayer) {
                                isBlocked = true;
                                break;
                            }
                        }
                    }
                }
                if (isBlocked) break;
            }

            if (isBlocked) {
                blockingReasons += "• Worker bloqueado por Sentinel\n";
            }
        }
    }

    return QString("%1\nControlador: %2\n%3\nPeça: %4 (%5)\n%6")
        .arg(position)
        .arg(controller)
        .arg(resources)
        .arg(pieceInfo)
        .arg(pieceDetails)
        .arg(blockingReasons.isEmpty() ? "Sem bloqueios" : "Bloqueios:\n" + blockingReasons);
}
void Board::updateCellTooltip(int row, int col) {
    if (row >= 0 && row < 12 && col >= 0 && col < 12) {
        QString tooltip = generateCellTooltip(row, col);
        cells[row][col]->setToolTip(tooltip);

        if (auto piece = getPieceAt(row, col)) {
            piece->setToolTip(tooltip);
        }
    }
}
void Board::updateCellDisplay(int row, int col) {
    if (row < 0 || row >= 12 || col < 0 || col >= 12) return;
    QString displayText;
    int resources = cellData[row][col].getResources();
    if (resources > 0) {
        displayText = QString::number(resources);
    }
    cells[row][col]->setText(displayText);
    QString bgColor = "white";
    QString fgColor = "black";
    if (cellData[row][col].isUnderControl()) {
        if (cellData[row][col].getController() == Player::Player1) {
            bgColor = "#e0e0e0";
        } else {
            bgColor = "#c0c0c0";
        }
    }
    QString style = QString("QPushButton { background-color: %1; border: 1px solid black; margin: 0px; padding: 0px; color: %2; font-size: 8pt; }"
                            "QPushButton:hover { background-color: #f0f0f0; }").arg(bgColor, fgColor);
    cells[row][col]->setStyleSheet(style);

    updateCellTooltip(row, col);
}
void Board::updateAllCellDisplays() {
    for (int row = 0; row < 12; ++row) {
        for (int col = 0; col < 12; ++col) {
            updateCellDisplay(row, col);
        }
    }
}
