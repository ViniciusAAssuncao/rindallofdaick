#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <qpushbutton.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    setWindowTitle("Rindall of Daick");
    resize(1024, 768);

    QPushButton *button = new QPushButton("Embaralhar", this);
    button->setGeometry(10, 10, 100, 30);
}

MainWindow::~MainWindow()
{
    delete ui;
}
