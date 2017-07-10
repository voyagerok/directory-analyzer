#ifndef DIRECTORYANALYZER_H
#define DIRECTORYANALYZER_H

#include <QObject>
#include <QRunnable>
#include <QMutex>
#include <unordered_set>
#include <QThread>
#include <QThreadPool>

#include "directory-tree-item.h"
//class DirectoryTreeItem;
Q_DECLARE_METATYPE(FileTypesInfoStorage)

//class DirectoryWalker: public QObject, public QRunnable {
//    Q_OBJECT
//public:
//    DirectoryWalker(DirectoryTreeItem *node, bool &interruptFlag, QObject *parent = Q_NULLPTR):
//        QObject{parent}, node {node}, interruptFlag{interruptFlag} {}
//    void run() override;
//private:
//    void calcStats();
//    DirectoryTreeItem *node;
//    bool &interruptFlag;
//signals:
//    void directoryUpdated(DirectoryTreeItem *node, qint64 newSize, int newFilesCount);
//};

//class DirectoryAnalyzer : public QObject
//{
//    Q_OBJECT
//public:
//    explicit DirectoryAnalyzer(QObject *parent = nullptr): QObject{parent}, pool {new QThreadPool(this)} {
////        pool->setMaxThreadCount(3);
//    }
//    ~DirectoryAnalyzer() { requestInterruption(); }
//    void updateDirectoryNode(DirectoryTreeItem *node);
//private:
//    void requestInterruption();
//    bool interruptFlag = false;
//    QMutex mutex;
//    std::unordered_set<DirectoryTreeItem *> activeNodes;
//    QThreadPool *pool;
//signals:
//    void nodeUpdated(DirectoryTreeItem *node);
//public slots:
//    void onNodeUpdated(DirectoryTreeItem *node, qint64 newSize, int newFilesCount);
//};

class DirectoryTreeBuilderThread: public QThread {
    Q_OBJECT
public:
    DirectoryTreeBuilderThread(DirectoryTreeItem *node, QObject *parent = Q_NULLPTR): QThread {parent}, node{node} {}
protected:
    void run() override;
private:
    void buildDirectoryTree(DirectoryTreeItem *root);
    DirectoryTreeItem *node;
signals:
    void buildingCompleted(DirectoryTreeItem *item);
    void buildFinished(DirectoryTreeItem  *root);
    void statusChanged(QString status);
};

class DirectoryTreeSizeCalculator: public QObject, public QRunnable {
    Q_OBJECT
public:
    DirectoryTreeSizeCalculator(DirectoryTreeItem *root, bool &interruptFlag, QObject *parent = Q_NULLPTR):
        QObject {parent}, interruptFlag {interruptFlag}, root {root} {}
    void run() override;
signals:
    void doneCalculation(DirectoryTreeItem *node, FileTypesInfoStorage storage);
    void done();
    void statusChanged(QString status);
private:
//    void startCalculation();
    FileTypesInfoStorage calcSize(DirectoryTreeItem *node);
    bool &interruptFlag;
    DirectoryTreeItem *root;
};

class DirectoryTreeAnalyzer: public QObject {
    Q_OBJECT
public:
    DirectoryTreeAnalyzer(QObject *parent = Q_NULLPTR): QObject{parent} { qRegisterMetaType<FileTypesInfoStorage>(); }
    ~DirectoryTreeAnalyzer() { stopBuilderThread(); stopCalculationThreads(); }
    void start(DirectoryTreeItem *root);
private:
    void stopBuilderThread();
    void stopCalculationThreads();
    DirectoryTreeBuilderThread *builderThread = Q_NULLPTR;
    QMutex mutex;
    bool poolInterruptionFlag = false;
signals:
    void buildCompleted(DirectoryTreeItem *node);
    void sizeUpdated(DirectoryTreeItem *node);
    void statusChanged(QString status);
    void done();
private slots:
    void startSizeCalculating(DirectoryTreeItem *root);
    void onSizeUpdated(DirectoryTreeItem *node, FileTypesInfoStorage storage);
};

#endif // DIRECTORYANALYZER_H
