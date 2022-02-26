#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    setWindowIcon(QIcon(":/Images/GitHub.png"));
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

