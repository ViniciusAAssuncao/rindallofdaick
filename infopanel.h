#ifndef INFOPANEL_H
#define INFOPANEL_H

#include <QWidget>
#include <QLabel>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QPushButton>
#include <QToolButton>
#include "piece.h"

class InfoPanel : public QWidget
{
    Q_OBJECT
public:
    explicit InfoPanel(QWidget *parent = nullptr);
    QString getMoveLogText() const;

public slots:
    void updateStatus(const QString& message);
    void updateTurn(Player player, int turn);
    void updateAlert(const QString& message);
    void addMoveToLog(const QString& moveNotation, Player player);
    void copyToClipboard(const QString& text);

signals:
    void copyLogRequested();
    void downloadLogRequested();
    void downloadLogRequestedToDesktop();

private:
    QLabel *turnLabel;
    QLabel *statusLabel;
    QLabel *alertLabel;
    QTextEdit *moveLogView;
    QPushButton *copyLogButton;
    QPushButton *downloadLogButton;

    int moveCount;
};

#endif
