#include "enginesettingsdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QSettings>
#include <QFileDialog>
#include <QFont>

EngineSettingsDialog::EngineSettingsDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUI();
    loadSettings();
    setModal(true);
    setWindowTitle("Configurações da Engine de IA");
    setFixedWidth(600);
    setStyleSheet("QDialog { background-color: white; color: black; border: 2px solid black; }");
}

void EngineSettingsDialog::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(15, 15, 15, 15);

    QFont titleFont("MS Sans Serif", 11, QFont::Bold);
    QFont mainFont("MS Sans Serif", 9, QFont::Bold);

    QLabel* titleLabel = new QLabel("CONFIGURAÇÕES DA ENGINE");
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("QLabel { color: black; padding: 8px; border-bottom: 2px solid black; }");
    mainLayout->addWidget(titleLabel);

    QPushButton *browseP1, *clearP1, *browseP2, *clearP2;

    mainLayout->addWidget(createEngineGroup("Player 1 (Brancas)", &player1PathEdit, &browseP1, &clearP1));
    mainLayout->addWidget(createEngineGroup("Player 2 (Pretas)", &player2PathEdit, &browseP2, &clearP2));

    connect(browseP1, &QPushButton::clicked, this, &EngineSettingsDialog::onBrowsePlayer1);
    connect(browseP2, &QPushButton::clicked, this, &EngineSettingsDialog::onBrowsePlayer2);
    connect(clearP1, &QPushButton::clicked, this, &EngineSettingsDialog::onClearPlayer1);
    connect(clearP2, &QPushButton::clicked, this, &EngineSettingsDialog::onClearPlayer2);

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    QString buttonStyle =
        "QPushButton {"
        "  background-color: white;"
        "  color: black;"
        "  border: 2px solid black;"
        "  padding: 8px 16px;"
        "}"
        "QPushButton:hover {"
        "  background-color: black;"
        "  color: white;"
        "}";

    QPushButton* okButton = new QPushButton("[ OK ]");
    okButton->setFont(mainFont);
    okButton->setStyleSheet(buttonStyle);
    connect(okButton, &QPushButton::clicked, this, &EngineSettingsDialog::onAccept);

    QPushButton* cancelButton = new QPushButton("[ Cancelar ]");
    cancelButton->setFont(mainFont);
    cancelButton->setStyleSheet(buttonStyle);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);

    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);
    mainLayout->addLayout(buttonLayout);
}

QWidget* EngineSettingsDialog::createEngineGroup(const QString& title, QLineEdit** lineEdit,
                                                 QPushButton** browseButton, QPushButton** clearButton)
{
    QFont mainFont("MS Sans Serif", 9, QFont::Bold);
    QGroupBox* groupBox = new QGroupBox(title);
    groupBox->setFont(mainFont);
    groupBox->setStyleSheet(
        "QGroupBox { color: black; border: 2px solid black; margin-top: 10px; padding-top: 10px; background-color: white; }"
        "QGroupBox::title { color: black; subcontrol-origin: margin; left: 10px; padding: 0 5px; }");

    QVBoxLayout* groupLayout = new QVBoxLayout(groupBox);
    QLabel* infoLabel = new QLabel("Deixe em branco para Humano, ou selecione o executável (.exe) da IA.");
    infoLabel->setFont(mainFont);
    infoLabel->setStyleSheet("color: black; border: none;");
    groupLayout->addWidget(infoLabel);

    QHBoxLayout* pathLayout = new QHBoxLayout();
    *lineEdit = new QLineEdit();
    (*lineEdit)->setFont(mainFont);
    (*lineEdit)->setReadOnly(true);
    (*lineEdit)->setStyleSheet("QLineEdit { background-color: #f0f0f0; color: black; border: 1px solid black; padding: 4px; }");
    pathLayout->addWidget(*lineEdit);

    *browseButton = new QPushButton("Procurar...");
    (*browseButton)->setFont(mainFont);
    (*browseButton)->setStyleSheet(
        "QPushButton { background-color: white; color: black; border: 2px solid black; padding: 4px 8px; }"
        "QPushButton:hover { background-color: black; color: white; }");
    pathLayout->addWidget(*browseButton);

    *clearButton = new QPushButton("Limpar");
    (*clearButton)->setFont(mainFont);
    (*clearButton)->setStyleSheet(
        "QPushButton { background-color: white; color: black; border: 2px solid black; padding: 4px 8px; }"
        "QPushButton:hover { background-color: black; color: white; }");
    pathLayout->addWidget(*clearButton);

    groupLayout->addLayout(pathLayout);
    return groupBox;
}

void EngineSettingsDialog::loadSettings()
{
    QSettings settings("RindallOfDaick", "EngineSettings");
    player1PathEdit->setText(settings.value("player1/enginePath").toString());
    player2PathEdit->setText(settings.value("player2/enginePath").toString());
}

void EngineSettingsDialog::saveSettings()
{
    QSettings settings("RindallOfDaick", "EngineSettings");
    settings.setValue("player1/enginePath", player1PathEdit->text());
    settings.setValue("player2/enginePath", player2PathEdit->text());
}

void EngineSettingsDialog::onBrowsePlayer1()
{
    QString filePath = QFileDialog::getOpenFileName(this, "Selecionar Engine Player 1", "", "Executáveis (*.exe);;Todos os Arquivos (*)");
    if (!filePath.isEmpty()) {
        player1PathEdit->setText(filePath);
    }
}

void EngineSettingsDialog::onBrowsePlayer2()
{
    QString filePath = QFileDialog::getOpenFileName(this, "Selecionar Engine Player 2", "", "Executáveis (*.exe);;Todos os Arquivos (*)");
    if (!filePath.isEmpty()) {
        player2PathEdit->setText(filePath);
    }
}

void EngineSettingsDialog::onClearPlayer1()
{
    player1PathEdit->clear();
}

void EngineSettingsDialog::onClearPlayer2()
{
    player2PathEdit->clear();
}

void EngineSettingsDialog::onAccept()
{
    saveSettings();
    QDialog::accept();
}
