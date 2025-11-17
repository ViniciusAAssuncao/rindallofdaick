#include "recruitmentdialog.h"
#include <QFont>

RecruitmentDialog::RecruitmentDialog(Player player, int availableResources,
                                     const QHash<PieceType, int>& costs,
                                     int workersReplaced,
                                     bool canRecruitLastLost,
                                     PieceType lastLostType,
                                     QWidget *parent)
    : QDialog(parent), player(player), availableResources(availableResources),
    costs(costs), workersReplaced(workersReplaced),
    canRecruitLastLost(canRecruitLastLost), lastLostType(lastLostType),
    confirmed(false)
{
    setupUI();
    setModal(true);
    setWindowTitle("Tela de Recrutamento");
    setStyleSheet("QDialog { background-color: white; color: black; border: 2px solid black; border-radius: 0; }");
}

void RecruitmentDialog::setupUI()
{
    layout = new QVBoxLayout(this);
    layout->setSpacing(5);
    layout->setContentsMargins(10, 10, 10, 10);

    QFont titleFont("MS Sans Serif", 11, QFont::Bold);
    QFont mainFont("MS Sans Serif", 9, QFont::Bold);

    titleLabel = new QLabel("MENU DE RECRUTAMENTO");
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("QLabel { padding: 5px; border-bottom: 2px solid black; color: black; }");
    layout->addWidget(titleLabel);

    resourceLabel = new QLabel(QString("Recursos Disponíveis: %1").arg(availableResources));
    resourceLabel->setFont(mainFont);
    resourceLabel->setAlignment(Qt::AlignCenter);
    resourceLabel->setStyleSheet("QLabel { padding: 5px; background-color: white; border: 2px solid black; margin: 5px; color: black; }");
    layout->addWidget(resourceLabel);

    QLabel* instructionLabel = new QLabel("Pressione a tecla ou clique para recrutar:");
    instructionLabel->setFont(mainFont);
    instructionLabel->setAlignment(Qt::AlignCenter);
    instructionLabel->setStyleSheet("QLabel { color: black; }");
    layout->addWidget(instructionLabel);

    bool canRecruitFootman = availableResources >= costs[PieceType::Footman] &&
                             (canRecruitLastLost || lastLostType != PieceType::Footman);
    layout->addWidget(createPieceButton(
        QString("[F] Footman - Custo: %1 | ATK: 2 | Movimento: 1").arg(costs[PieceType::Footman]),
        PieceType::Footman,
        costs[PieceType::Footman],
        canRecruitFootman
        ));

    bool canRecruitVanguard = availableResources >= costs[PieceType::Vanguard] &&
                              (canRecruitLastLost || lastLostType != PieceType::Vanguard);
    layout->addWidget(createPieceButton(
        QString("[V] Vanguard - Custo: %1 | ATK: 4 | Movimento: Em L").arg(costs[PieceType::Vanguard]),
        PieceType::Vanguard,
        costs[PieceType::Vanguard],
        canRecruitVanguard
        ));

    bool canRecruitSentinel = availableResources >= costs[PieceType::Sentinel] &&
                              (canRecruitLastLost || lastLostType != PieceType::Sentinel);
    layout->addWidget(createPieceButton(
        QString("[S] Sentinel - Custo: %1 | ATK: 3 | Bloqueia economia").arg(costs[PieceType::Sentinel]),
        PieceType::Sentinel,
        costs[PieceType::Sentinel],
        canRecruitSentinel
        ));

    bool canRecruitRindall = availableResources >= costs[PieceType::Rindall] &&
                             (canRecruitLastLost || lastLostType != PieceType::Rindall);
    layout->addWidget(createPieceButton(
        QString("[R] Rindall - Custo: %1 | ATK: 5 | Movimento: Ilimitado").arg(costs[PieceType::Rindall]),
        PieceType::Rindall,
        costs[PieceType::Rindall],
        canRecruitRindall
        ));

    if (workersReplaced < 2) {
        int workerCost = (workersReplaced == 0) ? 8 : 12;
        bool canRecruitWorker = availableResources >= workerCost;
        layout->addWidget(createPieceButton(
            QString("[W] Worker - Custo: %1 | Gera recursos (%2/2)").arg(workerCost).arg(workersReplaced),
            PieceType::Worker,
            workerCost,
            canRecruitWorker
            ));
    }

    QPushButton* cancelButton = new QPushButton("[ESC] Cancelar");
    cancelButton->setFont(mainFont);
    cancelButton->setStyleSheet(
        "QPushButton {"
        "  background-color: black;"
        "  color: white;"
        "  border: 2px solid black;"
        "  padding: 5px;"
        "  margin-top: 5px;"
        "  border-radius: 0;"
        "}"
        "QPushButton:hover {"
        "  background-color: white;"
        "  color: black;"
        "}"
        );

    connect(cancelButton, &QPushButton::clicked, this, &RecruitmentDialog::reject);
    layout->addWidget(cancelButton);

    setLayout(layout);
    setFixedWidth(480);
}

QPushButton* RecruitmentDialog::createPieceButton(const QString& text, PieceType type, int cost, bool enabled)
{
    QPushButton* button = new QPushButton(text);
    QFont buttonFont("MS Sans Serif", 9, QFont::Bold);
    button->setFont(buttonFont);

    if (enabled) {
        button->setStyleSheet(
            "QPushButton {"
            "  background-color: white;"
            "  border: 2px solid black;"
            "  padding: 8px;"
            "  text-align: left;"
            "  border-radius: 0;"
            "  color: black;"
            "}"
            "QPushButton:hover {"
            "  background-color: black;"
            "  color: white;"
            "}"
            );
        connect(button, &QPushButton::clicked, [this, type]() {
            onPieceButtonClicked(type);
        });
    } else {
        button->setStyleSheet(
            "QPushButton {"
            "  background-color: white;"
            "  border: 2px solid black;"
            "  padding: 8px;"
            "  text-align: left;"
            "  color: #555555;"
            "  border-radius: 0;"
            "}"
            );
        button->setEnabled(false);
    }
    return button;
}

void RecruitmentDialog::onPieceButtonClicked(PieceType type)
{
    selectedType = type;
    confirmed = true;
    accept();
}

void RecruitmentDialog::keyPressEvent(QKeyEvent *event)
{
    switch(event->key()) {
    case Qt::Key_F:
        if (availableResources >= costs[PieceType::Footman] &&
            (canRecruitLastLost || lastLostType != PieceType::Footman)) {
            onPieceButtonClicked(PieceType::Footman);
        }
        break;
    case Qt::Key_V:
        if (availableResources >= costs[PieceType::Vanguard] &&
            (canRecruitLastLost || lastLostType != PieceType::Vanguard)) {
            onPieceButtonClicked(PieceType::Vanguard);
        }
        break;
    case Qt::Key_S:
        if (availableResources >= costs[PieceType::Sentinel] &&
            (canRecruitLastLost || lastLostType != PieceType::Sentinel)) {
            onPieceButtonClicked(PieceType::Sentinel);
        }
        break;
    case Qt::Key_R:
        if (availableResources >= costs[PieceType::Rindall] &&
            (canRecruitLastLost || lastLostType != PieceType::Rindall)) {
            onPieceButtonClicked(PieceType::Rindall);
        }
        break;
    case Qt::Key_W:
        if (workersReplaced < 2) {
            int workerCost = (workersReplaced == 0) ? 8 : 12;
            if (availableResources >= workerCost) {
                onPieceButtonClicked(PieceType::Worker);
            }
        }
        break;
    case Qt::Key_Escape:
        reject();
        break;
    default:
        QDialog::keyPressEvent(event);
    }
}
