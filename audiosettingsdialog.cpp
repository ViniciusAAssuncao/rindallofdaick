#include "audiosettingsdialog.h"
#include "audiomanager.h"
#include <QFont>
#include <QHBoxLayout>
#include <QGroupBox>

AudioSettingsDialog::AudioSettingsDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUI();
    setModal(true);
    setWindowTitle("Configurações de Áudio");
    setFixedWidth(450);
    setStyleSheet("QDialog { background-color: white; color: black; border: 2px solid black; }");

    updateLabels();
}

void AudioSettingsDialog::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(15, 15, 15, 15);

    QFont titleFont("MS Sans Serif", 11, QFont::Bold);
    QFont mainFont("MS Sans Serif", 9, QFont::Bold);

    QLabel* titleLabel = new QLabel("CONFIGURAÇÕES DE ÁUDIO");
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("QLabel { color: black; padding: 8px; border-bottom: 2px solid black; }");
    mainLayout->addWidget(titleLabel);

    QGroupBox* masterGroup = new QGroupBox("VOLUME MASTER");
    masterGroup->setFont(mainFont);
    masterGroup->setStyleSheet(
        "QGroupBox { color: black; border: 2px solid black; margin-top: 10px; padding-top: 10px; background-color: white; }"
        "QGroupBox::title { color: black; subcontrol-origin: margin; left: 10px; padding: 0 5px; }"
        );
    QVBoxLayout* masterLayout = new QVBoxLayout(masterGroup);
    masterLayout->addWidget(createVolumeControl("Master:", &masterVolumeSlider, &masterVolumeLabel, &masterMuteCheckbox));
    mainLayout->addWidget(masterGroup);

    QGroupBox* detailGroup = new QGroupBox("VOLUMES INDIVIDUAIS");
    detailGroup->setFont(mainFont);
    detailGroup->setStyleSheet(
        "QGroupBox { color: black; border: 2px solid black; margin-top: 10px; padding-top: 10px; background-color: white; }"
        "QGroupBox::title { color: black; subcontrol-origin: margin; left: 10px; padding: 0 5px; }"
        );
    QVBoxLayout* detailLayout = new QVBoxLayout(detailGroup);
    detailLayout->addWidget(createVolumeControl("Efeitos Sonoros:", &sfxVolumeSlider, &sfxVolumeLabel, &sfxMuteCheckbox));
    mainLayout->addWidget(detailGroup);

    testSFXButton = new QPushButton("[ Testar Efeito Sonoro ]");
    testSFXButton->setFont(mainFont);
    testSFXButton->setStyleSheet(
        "QPushButton {"
        "  background-color: white;"
        "  color: black;"
        "  border: 2px solid black;"
        "  padding: 8px;"
        "  margin: 5px 0;"
        "}"
        "QPushButton:hover {"
        "  background-color: black;"
        "  color: white;"
        "}"
        );
    connect(testSFXButton, &QPushButton::clicked, this, &AudioSettingsDialog::onTestSFXClicked);
    mainLayout->addWidget(testSFXButton);

    closeButton = new QPushButton("[ Fechar ]");
    closeButton->setFont(mainFont);
    closeButton->setStyleSheet(
        "QPushButton {"
        "  background-color: black;"
        "  color: white;"
        "  border: 2px solid black;"
        "  padding: 8px;"
        "  margin-top: 5px;"
        "}"
        "QPushButton:hover {"
        "  background-color: white;"
        "  color: black;"
        "}"
        );
    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);
    mainLayout->addWidget(closeButton);

    setLayout(mainLayout);

    AudioManager& audio = AudioManager::instance();
    masterVolumeSlider->setValue(audio.getMasterVolume());
    sfxVolumeSlider->setValue(audio.getSFXVolume());
    masterMuteCheckbox->setChecked(audio.isMasterMuted());
    sfxMuteCheckbox->setChecked(audio.isSFXMuted());

    connect(masterVolumeSlider, &QSlider::valueChanged, this, &AudioSettingsDialog::onMasterVolumeChanged);
    connect(sfxVolumeSlider, &QSlider::valueChanged, this, &AudioSettingsDialog::onSFXVolumeChanged);
    connect(masterMuteCheckbox, &QCheckBox::toggled, this, &AudioSettingsDialog::onMasterMutedToggled);
    connect(sfxMuteCheckbox, &QCheckBox::toggled, this, &AudioSettingsDialog::onSFXMutedToggled);
}

QWidget* AudioSettingsDialog::createVolumeControl(const QString& labelText, QSlider** slider, QLabel** valueLabel, QCheckBox** muteCheckbox)
{
    QWidget* widget = new QWidget();
    QHBoxLayout* layout = new QHBoxLayout(widget);
    layout->setSpacing(8);
    layout->setContentsMargins(0, 5, 0, 5);

    QFont mainFont("MS Sans Serif", 9, QFont::Bold);

    QLabel* label = new QLabel(labelText);
    label->setFont(mainFont);
    label->setFixedWidth(120);
    label->setStyleSheet("color: black;");
    layout->addWidget(label);

    *slider = new QSlider(Qt::Horizontal);
    (*slider)->setRange(0, 100);
    (*slider)->setStyleSheet(
        "QSlider::groove:horizontal {"
        "  border: 1px solid black;"
        "  height: 8px;"
        "  background: white;"
        "}"
        "QSlider::handle:horizontal {"
        "  background: black;"
        "  border: 1px solid black;"
        "  width: 16px;"
        "  margin: -4px 0;"
        "}"
        "QSlider::handle:horizontal:hover {"
        "  background: #333333;"
        "}"
        );
    layout->addWidget(*slider);

    *valueLabel = new QLabel("100");
    (*valueLabel)->setFont(mainFont);
    (*valueLabel)->setFixedWidth(35);
    (*valueLabel)->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    (*valueLabel)->setStyleSheet("color: black;");
    layout->addWidget(*valueLabel);

    *muteCheckbox = new QCheckBox("Mutar");
    (*muteCheckbox)->setFont(mainFont);
    (*muteCheckbox)->setStyleSheet(
        "QCheckBox {"
        "  color: black;"
        "  spacing: 5px;"
        "}"
        "QCheckBox::indicator {"
        "  width: 16px;"
        "  height: 16px;"
        "  border: 2px solid black;"
        "  background: white;"
        "}"
        "QCheckBox::indicator:checked {"
        "  background: black;"
        "}"
        );
    layout->addWidget(*muteCheckbox);

    widget->setLayout(layout);
    return widget;
}

void AudioSettingsDialog::onMasterVolumeChanged(int value)
{
    AudioManager::instance().setMasterVolume(value);
    updateLabels();
}

void AudioSettingsDialog::onSFXVolumeChanged(int value)
{
    AudioManager::instance().setSFXVolume(value);
    updateLabels();
}

void AudioSettingsDialog::onMasterMutedToggled(bool checked)
{
    AudioManager::instance().setMasterMuted(checked);
}


void AudioSettingsDialog::onSFXMutedToggled(bool checked)
{
    AudioManager::instance().setSFXMuted(checked);
}

void AudioSettingsDialog::onTestSFXClicked()
{
    AudioManager::instance().playSoundEffect(SoundEffect::AlertGeneric);
}

void AudioSettingsDialog::updateLabels()
{
    masterVolumeLabel->setText(QString::number(masterVolumeSlider->value()));
    sfxVolumeLabel->setText(QString::number(sfxVolumeSlider->value()));
}
