#ifndef RECRUITMENTDIALOG_H
#define RECRUITMENTDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QKeyEvent>
#include "piece.h"

class RecruitmentDialog : public QDialog
{
    Q_OBJECT
public:
    explicit RecruitmentDialog(Player player, int availableResources,
                               const QHash<PieceType, int>& costs,
                               int workersReplaced,
                               bool canRecruitLastLost,
                               PieceType lastLostType,
                               QWidget *parent = nullptr);

    PieceType getSelectedPieceType() const { return selectedType; }
    bool wasConfirmed() const { return confirmed; }

protected:
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void onPieceButtonClicked(PieceType type);

private:
    void setupUI();
    QPushButton* createPieceButton(const QString& text, PieceType type, int cost, bool enabled);

    Player player;
    int availableResources;
    QHash<PieceType, int> costs;
    int workersReplaced;
    bool canRecruitLastLost;
    PieceType lastLostType;

    PieceType selectedType;
    bool confirmed;

    QVBoxLayout* layout;
    QLabel* titleLabel;
    QLabel* resourceLabel;
};

#endif
