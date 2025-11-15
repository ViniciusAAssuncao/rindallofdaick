
#include "piecewidget.h"
#include <QPixmap>
#include <QFont>
#include <QMouseEvent>

PieceWidget::PieceWidget(Piece* piece, QWidget *parent)
    : QLabel(parent), piece(piece)
{
    setAlignment(Qt::AlignCenter);
    setFixedSize(50, 50);
    updateAppearance();
}
PieceWidget::~PieceWidget() {
    delete piece;
}
Piece* PieceWidget::getPiece() const {
    return piece;
}
void PieceWidget::updateAppearance() {
    QFont retroFont("MS Sans Serif", 16, QFont::Bold);
    setFont(retroFont);
    QString style = "QLabel { background-color: %1; color: %2; border: 1px solid black; margin: 0px; padding: 0px; }";
    QString bg, fg;
    if (piece->getPlayer() == Player::Player1) {
        bg = "white";
        fg = "black";
    } else {
        bg = "black";
        fg = "white";
    }
    setStyleSheet(style.arg(bg, fg));
    QString imagePath;
    switch (piece->getType()) {
    case PieceType::Worker:
        imagePath = ":/images/worker.png";
        break;
    case PieceType::Daick:
        imagePath = ":/images/daick.png";
        break;
    case PieceType::Rindall:
        imagePath = ":/images/rindall.png";
        break;
    case PieceType::Footman:
        imagePath = ":/images/footman.png";
        break;
    case PieceType::Sentinel:
        imagePath = ":/images/sentinel.png";
        break;
    case PieceType::Vanguard:
        imagePath = ":/images/vanguard.png";
        break;
    case PieceType::Bastion:
        imagePath = ":/images/bastion.png";
        break;
    default:
        imagePath = "";
        break;
    }
    if (!imagePath.isEmpty()) {
        QPixmap pixmap(imagePath);
        if (!pixmap.isNull()) {
            if (piece->getPlayer() == Player::Player2) {
                QImage image = pixmap.toImage();
                image.invertPixels();
                pixmap = QPixmap::fromImage(image);
            }
            setPixmap(pixmap.scaled(size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
            setText("");
        } else {
            setText(piece->getDisplayText());
        }
    } else {
        setText(piece->getDisplayText());
    }
}

void PieceWidget::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        emit clicked();
    }
    QLabel::mousePressEvent(event);
}
