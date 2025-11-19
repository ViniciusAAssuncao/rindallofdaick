#ifndef TACTICALARROW_H
#define TACTICALARROW_H

#include <QWidget>
#include <QPainter>
#include <QPoint>

class TacticalArrow : public QWidget
{
    Q_OBJECT
public:
    enum ArrowType {
        Tactical,
        PreMove
    };

    explicit TacticalArrow(QPoint start, QPoint end, ArrowType type, QWidget *parent = nullptr);

    QPoint getStart() const { return startCell; }
    QPoint getEnd() const { return endCell; }
    ArrowType getType() const;
    void setType(ArrowType type);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QPoint startCell;
    QPoint endCell;
    ArrowType m_type;
    QPoint calculateCellCenter(int row, int col) const;
};

#endif
