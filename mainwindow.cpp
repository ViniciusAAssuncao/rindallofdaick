#include "mainwindow.h"
#include <QApplication>
#include <QFontDatabase>
#include <QHBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    QFontDatabase::addApplicationFont(":/fonts/W95FA.otf");

    QWidget *centralWidget = new QWidget(this);
    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    setWindowTitle("Rindall of Daick"); //NUNCA MEXA AQUI!!

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

    resize(centralWidget->sizeHint());

    setStyleSheet("QMainWindow { background-color: white; }");

    gameController->initializeGame();
}

MainWindow::~MainWindow()
{
}
