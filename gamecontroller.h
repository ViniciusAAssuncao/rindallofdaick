#ifndef GAMECONTROLLER_H
#define GAMECONTROLLER_H

#include <QObject>
#include <QHash>
#include <QPoint>
#include "pieces.h"
#include "piecewidget.h"
#include "infopanel.h"

class Board;

class GameController : public QObject
{
    Q_OBJECT
public:
    explicit GameController(Board* board, InfoPanel* panel, QObject* parent = nullptr);
    void initializeGame();
    void placePiece(int row, int col, PieceType type, Player player);
    void onCopyLogRequested();
    void onDownloadLogRequested();
    void onDownloadLogToDesktopRequested();

signals:
    void statusMessage(const QString& message);
    void turnChanged(Player player, int turn);
    void alertMessage(const QString& message);
    void moveMade(const QString& moveNotation, Player player);
    void sendLogToClipboard(const QString& text);

private slots:
    void handleCellClicked(int row, int col);

private:
    void setupInitialPieces();
    void selectPiece(int row, int col);
    void moveSelectedPieceTo(int row, int col);
    void performRetroAnimation(PieceWidget* pieceWidget, int toRow, int toCol, bool isCapture);

    void endTurn();
    QString checkResourceGeneration();
    QString checkPassivity();
    bool isSentinelBlockingCell(int row, int col, Player resourcePlayer);
    void updateCellControl(int row, int col, Player player);
    QString generateMoveNotation(PieceWidget* piece, int toRow, int toCol, bool isCapture) const;
    QString generateFullGameStateNotation() const;

    Board* board;
    InfoPanel* infoPanel;
    QHash<QPoint, PieceWidget*> pieces;
    PieceWidget* selectedPiece = nullptr;
    QPoint selectedPos;

    Player currentPlayer;
    int turnNumber;
};

#endif
