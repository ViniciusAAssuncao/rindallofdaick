#ifndef PIECEWIDGET_H
#define PIECEWIDGET_H
#include <QWidget>
#include <QLabel>
#include "piece.h"
class PieceWidget : public QLabel
{
    Q_OBJECT
public:
    explicit PieceWidget(Piece* piece, QWidget *parent = nullptr);
    ~PieceWidget();
    Piece* getPiece() const;
    void updateAppearance();
protected:
    void mousePressEvent(QMouseEvent *event) override;
signals:
    void clicked();
private:
    Piece* piece;
};
#endif
