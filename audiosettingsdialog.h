#ifndef AUDIOSETTINGSDIALOG_H
#define AUDIOSETTINGSDIALOG_H

#include <QDialog>
#include <QSlider>
#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

class AudioSettingsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit AudioSettingsDialog(QWidget *parent = nullptr);

private slots:
    void onMasterVolumeChanged(int value);
    void onSFXVolumeChanged(int value);
    void onMasterMutedToggled(bool checked);
    void onSFXMutedToggled(bool checked);
    void onTestSFXClicked();
    void updateLabels();

private:
    void setupUI();
    QWidget* createVolumeControl(const QString& labelText, QSlider** slider, QLabel** valueLabel, QCheckBox** muteCheckbox);

    QSlider* masterVolumeSlider;
    QSlider* sfxVolumeSlider;

    QLabel* masterVolumeLabel;
    QLabel* sfxVolumeLabel;

    QCheckBox* masterMuteCheckbox;
    QCheckBox* sfxMuteCheckbox;

    QPushButton* testSFXButton;
    QPushButton* closeButton;
};

#endif
