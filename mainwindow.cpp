#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "directory-tree-model.h"
#include "utils.h"

#include <QContextMenuEvent>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->actionExit, &QAction::triggered, QApplication::instance(), &QApplication::quit);
    connect(ui->actionStart, &QAction::triggered, this, &MainWindow::startScanning);
    connect(ui->actionStop, &QAction::triggered, this, &MainWindow::stopScanning);

    ui->progressBar->setValue(0);

//    dirModel = new QFileSystemModel(this);
//    dirModel->setRootPath(QDir::rootPath());
//    ui->dirView->setModel(dirModel);


    this->customDirModel = new DirectoryTreeModel(QDir::rootPath(), this);
    connect(customDirModel, &DirectoryTreeModel::statusChanged, this, &MainWindow::handleStatusChanged);
    connect(customDirModel, &DirectoryTreeModel::progressStarted, this, &MainWindow::handleProgressStarted);
    connect(customDirModel, &DirectoryTreeModel::progressFinished, this, &MainWindow::handleProgressFinished);
    ui->dirView->setModel(customDirModel);
    customDirModel->setDefaultPath();
//    customDirModel->setRootPath(QDir::rootPath());
//    customDirModel->buildIndex(QDir::rootPath());

    fileTypesModel = new QStandardItemModel(this);
    fileTypesModel->setHorizontalHeaderLabels({"File Type", "Average Size"});
    ui->filesInfoView->setModel(fileTypesModel);
    ui->filesInfoView->verticalHeader()->hide();

    connect(ui->dirView, &QTreeView::clicked, this, &MainWindow::onDirSelected);
}

void MainWindow::onDirSelected(const QModelIndex &index) {
    auto fileTypesInfo = customDirModel->getFileTypesInfo(index);
    fileTypesModel->setRowCount(fileTypesInfo.size());
    fileTypesModel->setColumnCount(2);

    for (int i = 0; i < fileTypesInfo.size(); ++i) {
        std::tuple<QString, qint64> &record = fileTypesInfo[i];
        QStandardItem *fileTypeItem = new QStandardItem(std::get<0>(record ));
        QStandardItem *fileTypeAvgSizeItem = new QStandardItem(Utils::size_human(std::get<1>(record)));
        fileTypesModel->setItem(i, 0, fileTypeItem);
        fileTypesModel->setItem(i, 1, fileTypeAvgSizeItem);
    }
}

void MainWindow::handleStatusChanged(QString status) {
    ui->statusBar->showMessage(status);
}

void MainWindow::handleProgressFinished() {
    ui->progressBar->setMaximum(1);
    ui->progressBar->setValue(1);
//    ui->statusBar->clearMessage();
    ui->statusBar->showMessage("Done", 2000);
}

void MainWindow::handleProgressStarted() {
    ui->progressBar->setRange(0,0);
}

void MainWindow::startScanning() {
//    QItemSelectionModel *model = ui->dirView->selectionModel();
//    QModelIndexList selectedIndexes = model->selectedIndexes();
//    if (selectedIndexes.empty()) {
//        ui->statusBar->showMessage("You should select directory before starting scanning", 4000);
//        return;
//    }

    QModelIndex index = ui->dirView->currentIndex();
    if (!index.isValid()) {
        ui->statusBar->showMessage("You should select directory before starting scanning", 4000);
        return;
    }

    customDirModel->buildIndex(index);
}

void MainWindow::stopScanning() {
    customDirModel->stopProgress();
}

void MainWindow::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu(this);
    menu.addAction(ui->actionStart);
    menu.exec(event->globalPos());
}

MainWindow::~MainWindow()
{
    delete ui;
}
