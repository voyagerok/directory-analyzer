#ifndef DIRECTORYWALKER_H
#define DIRECTORYWALKER_H

#include <QString>
#include <QObject>
#include <QThread>
#include <QDir>

class DirectoryTreeItem;
class DirectoryTreeModel;

class DirectoryTreeBuilder: public QThread {
    Q_OBJECT
public:
    DirectoryTreeBuilder(const QString &rootDir, QObject *parent = Q_NULLPTR):
        QThread(parent), rootDir {rootDir} {}
protected:
    void run() override;
private:
    void buildTree(const QDir &rootDir);
    QString rootDir;
signals:
    void nodeConstructed(DirectoryTreeItem *node);
    void treeConstructed(QVector<DirectoryTreeItem *> leafs);
};

class DirectoryAnalyzerThread: public QThread {
    Q_OBJECT
public:
    DirectoryAnalyzerThread(QVector<DirectoryTreeItem *> *leafs, QObject *parent = Q_NULLPTR):
        QThread {parent}, leafs {leafs} {}
protected:
    void run() override;
private:
    void analyzeLeaf(DirectoryTreeItem *leaf);
    QVector<DirectoryTreeItem *> *leafs;
signals:
    void nodeUpdated(DirectoryTreeItem *node, qint64 sizeInc, int filesCountInc);
};

class DirectoryAnalyzer: public QObject
{
    Q_OBJECT
public:
    DirectoryAnalyzer(QObject *parent = Q_NULLPTR):
        QObject {parent} {}
    ~DirectoryAnalyzer();
    void beginScanning(const QString &rootDirectory);
    void stopScanning();
    bool isRunning() const;
private:
    DirectoryTreeBuilder *builderThread = Q_NULLPTR;
};

#endif // DIRECTORYWALKER_H
