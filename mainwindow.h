// MODIFICAR mainwindow.h:
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QHBoxLayout>
#include <QWidget>
#include "board.h"
#include "gamecontroller.h"
#include "infopanel.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Board *board;
    GameController *gameController;
    InfoPanel *infoPanel;
};

#endif
