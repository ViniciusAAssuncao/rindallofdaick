#ifndef ENGINESETTINGSDIALOG_H
#define ENGINESETTINGSDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>

class EngineSettingsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit EngineSettingsDialog(QWidget *parent = nullptr);

private slots:
    void onBrowsePlayer1();
    void onBrowsePlayer2();
    void onClearPlayer1();
    void onClearPlayer2();
    void onAccept();

private:
    void setupUI();
    void loadSettings();
    void saveSettings();
    QWidget* createEngineGroup(const QString& title, QLineEdit** lineEdit,
                               QPushButton** browseButton, QPushButton** clearButton);

    QLineEdit* player1PathEdit;
    QLineEdit* player2PathEdit;
};

#endif
