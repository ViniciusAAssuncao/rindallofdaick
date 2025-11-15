#ifndef BOARD_H
#define BOARD_H
#include <QWidget>
#include <QGridLayout>
#include <QPushButton>
#include <QLabel>
#include "piecewidget.h"
#include "piece.h"
#include "cell.h"

class Board : public QWidget
{
    Q_OBJECT
public:
    explicit Board(QWidget *parent = nullptr);
    void addPieceToCell(int row, int col, PieceWidget* pieceWidget);
    void removePieceFromCell(int row, int col);
    void movePiece(int fromRow, int fromCol, int toRow, int toCol);
    PieceWidget* getPieceAt(int row, int col) const;
    void highlightCells(const QList<QPair<int, int>>& cellsToHighlight);
    void clearHighlights();
    QPushButton* getCellButton(int row, int col) const;

    Cell* getCellData(int row, int col);
    void updateCellDisplay(int row, int col);
    void updateAllCellDisplays();

signals:
    void cellClicked(int row, int col);
private slots:
    void onCellClicked();
private:
    void setupGrid();
    void setupInitialPieces();
    Piece* createPiece(PieceType type, Player player);
    QString generateCellTooltip(int row, int col) const;
    void updateCellTooltip(int row, int col);
    QGridLayout *layout;
    QPushButton* cells[12][12];
    Cell cellData[12][12];
    QHash<QPair<int, int>, PieceWidget*> pieceMap;
};
#endif
