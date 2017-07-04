#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->dirModel = new QFileSystemModel(this);
    this->dirModel->setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
    this->dirModel->setRootPath("");
    ui->dirView->setModel(dirModel);
}

MainWindow::~MainWindow()
{
    delete ui;
}
