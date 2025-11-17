#include "mainwindow.h"
#include "audiosettingsdialog.h"
#include "enginesettingsdialog.h"
#include "audiomanager.h"
#include <QApplication>
#include <QFontDatabase>
#include <QHBoxLayout>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QSettings>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    QFontDatabase::addApplicationFont(":/fonts/W95FA.otf");

    QWidget *centralWidget = new QWidget(this);
    mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    setWindowTitle("Rindall of Daick");

    board = new Board(this);
    infoPanel = new InfoPanel(this);

    gameController = new GameController(board, infoPanel, this);

    connect(gameController, &GameController::statusMessage,
            infoPanel, &InfoPanel::updateStatus);

    connect(gameController, &GameController::turnChanged,
            infoPanel, &InfoPanel::updateTurn);

    connect(gameController, &GameController::alertMessage,
            infoPanel, &InfoPanel::updateAlert);

    connect(gameController, &GameController::moveMade,
            infoPanel, &InfoPanel::addMoveToLog);

    connect(infoPanel, &InfoPanel::copyLogRequested,
            gameController, &GameController::onCopyLogRequested);

    connect(gameController, &GameController::sendLogToClipboard,
            infoPanel, &InfoPanel::copyToClipboard);

    mainLayout->addWidget(board);
    mainLayout->addWidget(infoPanel);

    centralWidget->setLayout(mainLayout);
    setCentralWidget(centralWidget);

    createMenuBar();

    resize(centralWidget->sizeHint());

    setStyleSheet("QMainWindow { background-color: white; }");

    loadEngineSettings();
    gameController->setEnginePaths(m_player1EnginePath, m_player2EnginePath);
    gameController->initializeGame();
}

MainWindow::~MainWindow()
{
}

void MainWindow::createMenuBar()
{
    QFont menuFont("MS Sans Serif", 9, QFont::Bold);

    QMenuBar* menuBar = new QMenuBar(this);
    menuBar->setFont(menuFont);
    menuBar->setStyleSheet(
        "QMenuBar {"
        "  background-color: white;"
        "  color: black;"
        "  border-bottom: 2px solid black;"
        "  padding: 2px;"
        "}"
        "QMenuBar::item {"
        "  background-color: white;"
        "  color: black;"
        "  padding: 4px 12px;"
        "}"
        "QMenuBar::item:selected {"
        "  background-color: black;"
        "  color: white;"
        "}"
        "QMenu {"
        "  background-color: white;"
        "  color: black;"
        "  border: 2px solid black;"
        "}"
        "QMenu::item {"
        "  padding: 6px 20px;"
        "}"
        "QMenu::item:selected {"
        "  background-color: black;"
        "  color: white;"
        "}"
        );

    QMenu* gameMenu = menuBar->addMenu("Menu");
    gameMenu->setFont(menuFont);

    QAction* newGameAction = gameMenu->addAction("Novo Jogo");
    newGameAction->setFont(menuFont);
    connect(newGameAction, &QAction::triggered, this, &MainWindow::startNewGame);

    QAction* engineAction = gameMenu->addAction("Engine...");
    engineAction->setFont(menuFont);
    connect(engineAction, &QAction::triggered, this, &MainWindow::openEngineSettings);

    gameMenu->addSeparator();

    QAction* exigAction = gameMenu->addAction("Sair do Jogo");
    exigAction->setFont(menuFont);
    connect(exigAction, &QAction::triggered, QApplication::instance(), &QApplication::quit);

    QMenu* audioMenu = menuBar->addMenu("Áudio");
    audioMenu->setFont(menuFont);

    QAction* settingsAction = audioMenu->addAction("Configurações de Áudio");
    settingsAction->setFont(menuFont);
    connect(settingsAction, &QAction::triggered, this, &MainWindow::openAudioSettings);

    audioMenu->addSeparator();

    QAction* toggleSFXAction = audioMenu->addAction("Ativar/Desativar Efeitos");
    toggleSFXAction->setFont(menuFont);
    connect(toggleSFXAction, &QAction::triggered, this, &MainWindow::toggleSFX);

    setMenuBar(menuBar);
}

void MainWindow::openAudioSettings()
{
    AudioSettingsDialog dialog(this);
    dialog.exec();
}

void MainWindow::toggleMusic()
{
    AudioManager& audio = AudioManager::instance();
    audio.setMusicMuted(!audio.isMusicMuted());
}

void MainWindow::toggleSFX()
{
    AudioManager& audio = AudioManager::instance();
    audio.setSFXMuted(!audio.isSFXMuted());
}

void MainWindow::startNewGame()
{
    delete gameController;
    delete infoPanel;
    delete board;

    board = new Board(this);
    infoPanel = new InfoPanel(this);

    gameController = new GameController(board, infoPanel, this);

    connect(gameController, &GameController::statusMessage,
            infoPanel, &InfoPanel::updateStatus);

    connect(gameController, &GameController::turnChanged,
            infoPanel, &InfoPanel::updateTurn);

    connect(gameController, &GameController::alertMessage,
            infoPanel, &InfoPanel::updateAlert);

    connect(gameController, &GameController::moveMade,
            infoPanel, &InfoPanel::addMoveToLog);

    connect(infoPanel, &InfoPanel::copyLogRequested,
            gameController, &GameController::onCopyLogRequested);

    connect(gameController, &GameController::sendLogToClipboard,
            infoPanel, &InfoPanel::copyToClipboard);

    mainLayout->addWidget(board);
    mainLayout->addWidget(infoPanel);

    loadEngineSettings();
    gameController->setEnginePaths(m_player1EnginePath, m_player2EnginePath);
    gameController->initializeGame();
}

void MainWindow::openEngineSettings()
{
    EngineSettingsDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        loadEngineSettings();
        emit infoPanel->updateStatus("Configurações da engine salvas. Inicie um novo jogo para aplicar.");
    }
}

void MainWindow::loadEngineSettings()
{
    QSettings settings("RindallOfDaick", "EngineSettings");
    m_player1EnginePath = settings.value("player1/enginePath").toString();
    m_player2EnginePath = settings.value("player2/enginePath").toString();
}
