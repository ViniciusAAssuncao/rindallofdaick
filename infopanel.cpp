#include "infopanel.h"
#include <QFont>
#include <QScrollBar>
#include <QHBoxLayout>
#include <QApplication>
#include <QClipboard>
#include <QIcon>
#include <qdir.h>
#include <QToolButton>
#include <QMenu>
#include <QAction>

InfoPanel::InfoPanel(QWidget *parent)
    : QWidget(parent), moveCount(1)
{
    setFixedWidth(300);
    setStyleSheet("QWidget { background-color: white; color: black; border: none; }");
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setSpacing(5);
    layout->setContentsMargins(5, 5, 5, 5);
    QFont retroFont("MS Sans Serif", 10, QFont::Bold);
    QFont logFont("MS Sans Serif", 9);
    turnLabel = new QLabel("Turn 1: Player 1");
    turnLabel->setFont(retroFont);
    turnLabel->setWordWrap(true);
    turnLabel->setMinimumHeight(30);
    turnLabel->setStyleSheet("border-bottom: 1px solid black; padding-bottom: 5px;");
    statusLabel = new QLabel("GAME START. Player 1, select a piece.");
    statusLabel->setFont(logFont);
    statusLabel->setWordWrap(true);
    statusLabel->setMinimumHeight(40);
    statusLabel->setAlignment(Qt::AlignTop);
    alertLabel = new QLabel("");
    alertLabel->setFont(retroFont);
    alertLabel->setWordWrap(true);
    alertLabel->setMinimumHeight(30);
    alertLabel->setStyleSheet("QLabel { color: black; background-color: #f0f0f0; border: 1px solid black; padding: 3px; }");
    alertLabel->setAlignment(Qt::AlignCenter);
    alertLabel->hide();
    QHBoxLayout *logHeaderLayout = new QHBoxLayout();
    QLabel *logTitle = new QLabel("Log de Movimentos");
    logTitle->setFont(retroFont);
    logTitle->setStyleSheet("margin-top: 10px; padding-bottom: 5px;");
    QString buttonStyle =
        "QPushButton {"
        " margin-top: 10px;"
        "}"
        "QPushButton:hover {"
        " background-color: #e0e0e0;"
        "}";
    copyLogButton = new QPushButton();
    copyLogButton->setFont(retroFont);
    copyLogButton->setIcon(QIcon(":/images/copyicon.png"));
    copyLogButton->setIconSize(QSize(27, 27));
    copyLogButton->setCursor(Qt::PointingHandCursor);
    copyLogButton->setToolTip("Copia o log de movimentos e o estado atual completo do tabuleiro para a área de transferência.");
    copyLogButton->setStyleSheet(buttonStyle);

    downloadLogButton = new QPushButton();
    downloadLogButton->setFont(retroFont);
    downloadLogButton->setIcon(QIcon(":/images/downloadicon.png"));
    downloadLogButton->setIconSize(QSize(27, 27));
    downloadLogButton->setCursor(Qt::PointingHandCursor);
    downloadLogButton->setToolTip("Baixar o log de movimentos como .txt");
    downloadLogButton->setStyleSheet(buttonStyle);
    connect(downloadLogButton, &QPushButton::clicked, this, &InfoPanel::downloadLogRequested);

    logHeaderLayout->addWidget(logTitle);
    logHeaderLayout->addStretch();
    logHeaderLayout->addWidget(downloadLogButton);
    logHeaderLayout->addWidget(copyLogButton);
    moveLogView = new QTextEdit();
    moveLogView->setReadOnly(true);
    moveLogView->setFont(logFont);
    moveLogView->setStyleSheet(
        "QTextEdit {"
        " background-color: white;"
        " color: black;"
        " border: 1px solid black;"
        " padding: 2px;"
        "}"
        "QScrollBar:vertical {"
        " border: 1px solid black;"
        " background: white;"
        " width: 16px;"
        " margin: 0px 0px 0px 0px;"
        "}"
        "QScrollBar::handle:vertical {"
        " background: #c0c0c0;"
        " border: 1px solid black;"
        " min-height: 20px;"
        "}"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
        " border: 1px solid black;"
        " background: #e0e0e0;"
        " height: 15px;"
        " subcontrol-origin: margin;"
        "}"
        "QScrollBar::sub-line:vertical {"
        " subcontrol-position: top;"
        "}"
        "QScrollBar::add-line:vertical {"
        " subcontrol-position: bottom;"
        "}"
        );
    layout->addWidget(turnLabel);
    layout->addWidget(statusLabel);
    layout->addWidget(alertLabel);
    layout->addLayout(logHeaderLayout);
    layout->addWidget(moveLogView);
    setLayout(layout);
    connect(copyLogButton, &QPushButton::clicked, this, &InfoPanel::copyLogRequested);
}

void InfoPanel::updateStatus(const QString& message) {
    statusLabel->setText(message);
}
void InfoPanel::updateTurn(Player player, int turn) {
    QString playerStr = (player == Player::Player1) ? "Player 1 (Brancas)" : "Player 2 (Pretas)";
    turnLabel->setText(QString("Turno %1: %2").arg(turn).arg(playerStr));
}
void InfoPanel::updateAlert(const QString& message) {
    if (message.isEmpty() || message.isNull()) {
        alertLabel->hide();
    } else {
        alertLabel->setText(message);
        alertLabel->show();
    }
}
void InfoPanel::addMoveToLog(const QString& moveNotation, Player player) {
    QString playerLabel;

    if (player == Player::Player1) {
        playerLabel = "Brancas";
    } else {
        playerLabel = "Pretas";
        moveCount++;
    }

    QString moveEntry = QString("%1: %2").arg(playerLabel).arg(moveNotation);
    moveLogView->append(moveEntry);
    moveLogView->verticalScrollBar()->setValue(moveLogView->verticalScrollBar()->maximum());
}
QString InfoPanel::getMoveLogText() const
{
    return moveLogView->toPlainText();
}

void InfoPanel::copyToClipboard(const QString &text)
{
    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setText(text);
    updateStatus("Log copiado para a área de transferência.");
}
