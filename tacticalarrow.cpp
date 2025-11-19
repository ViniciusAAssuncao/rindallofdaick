#include "tacticalarrow.h"
#include <QPen>
#include <cmath>

TacticalArrow::TacticalArrow(QPoint start, QPoint end, ArrowType type, QWidget *parent)
    : QWidget(parent), startCell(start), endCell(end), m_type(type)
{
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setAttribute(Qt::WA_TranslucentBackground);
}

TacticalArrow::ArrowType TacticalArrow::getType() const
{
    return m_type;
}

void TacticalArrow::setType(ArrowType type)
{
    m_type = type;
    update();
}

QPoint TacticalArrow::calculateCellCenter(int row, int col) const
{
    const int cellSize = 50;
    const int labelSize = 30;

    int x = labelSize + col * cellSize + cellSize / 2;
    int y = labelSize + row * cellSize + cellSize / 2;

    return QPoint(x, y);
}

void TacticalArrow::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, false);

    QPoint startPixel = calculateCellCenter(startCell.x(), startCell.y());
    QPoint endPixel = calculateCellCenter(endCell.x(), endCell.y());

    QColor arrowColor = (m_type == PreMove) ? Qt::red : Qt::black;

    QPen pen(arrowColor, 3);
    pen.setStyle(Qt::SolidLine);
    painter.setPen(pen);

    painter.drawLine(startPixel, endPixel);

    double angle = std::atan2(endPixel.y() - startPixel.y(),
                              endPixel.x() - startPixel.x());

    const int arrowSize = 12;
    QPoint arrowP1(endPixel.x() - arrowSize * std::cos(angle - M_PI / 6),
                   endPixel.y() - arrowSize * std::sin(angle - M_PI / 6));
    QPoint arrowP2(endPixel.x() - arrowSize * std::cos(angle + M_PI / 6),
                   endPixel.y() - arrowSize * std::sin(angle + M_PI / 6));

    painter.drawLine(endPixel, arrowP1);
    painter.drawLine(endPixel, arrowP2);
}
