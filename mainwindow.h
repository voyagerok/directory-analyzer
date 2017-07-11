#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileSystemModel>
#include <QStandardItemModel>

class DirectoryTreeModel;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;

private:
    Ui::MainWindow *ui;

    QFileSystemModel *dirModel;
    DirectoryTreeModel *customDirModel;
    QStandardItemModel *fileTypesModel;

public slots:
    void onDirSelected(const QModelIndex &index);
    void handleProgressStarted();
    void handleProgressFinished();
    void handleStatusChanged(QString status);

    void startScanning();
    void stopScanning();
};

#endif // MAINWINDOW_H
