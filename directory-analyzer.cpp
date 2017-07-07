#include <QFileInfo>
#include <tuple>
#include <queue>

#include "directory-analyzer.h"
#include "directory-tree-model.h"
#include "directory-tree-item.h"

void DirectoryTreeBuilder::run() {
    buildTree(rootDir);
}

void DirectoryTreeBuilder::buildTree(const QDir &rootDir) {
    std::queue<DirectoryTreeItem *> nodes;
    QVector<DirectoryTreeItem *> leafs;

    DirectoryTreeItem *root = new DirectoryTreeItem(rootDir);
    nodes.push(root);

    while(!nodes.empty()) {
        DirectoryTreeItem *currentNode = nodes.front();
        nodes.pop();

        QFileInfoList dirContents = currentNode->getDirectory().entryInfoList(QDir::Filter::Dirs | QDir::Filter::NoDotAndDotDot);
        for (const auto &fileInfo : dirContents) {
            if (fileInfo.isDir()) {
                DirectoryTreeItem *childItem = new DirectoryTreeItem(fileInfo.absoluteDir(), currentNode);
                currentNode->appendChild(childItem);
                nodes.push(childItem);
            }
        }
        if (currentNode->isLeaf()) {
            leafs.push_back(currentNode);
        }
        emit nodeConstructed(currentNode);
    }

    emit treeConstructed(leafs);
}

void DirectoryAnalyzerThread::run() {
    for (DirectoryTreeItem *leaf : *leafs) {
        analyzeNode(leaf);
    }
}

void DirectoryAnalyzerThread::analyzeNode(DirectoryTreeItem *node) {
    qint64 dirSize = 0;
    int filesCount = 0;
    QFileInfoList dirContent = node->getDirectory().entryInfoList(QDir::Filter::Files);
    for (const auto &fileInfo : dirContent) {
        if (fileInfo.isFile()) {
            ++ filesCount;
            dirSize += fileInfo.size();
        }
    }
    emit nodeUpdated(node, dirSize, filesCount);

    if (node->hasParent()) {
        analyzeNode(node->getParent());
    }
}

void DirectoryAnalyzer::beginScanning(const QString &rootDirectory) {
//    stopScanning();
//    workingThread = new DirectoryWalkerThread(rootDirectory);
//    connect(workingThread, &DirectoryWalkerThread::updateStatus, this, &DirectoryWalker::updateTreeModel);
//    workingThread->start();
    stopScanning();

}

void DirectoryAnalyzer::stopScanning() {
//    if (workingThread) {
//        if (workingThread->isRunning()) {
//            workingThread->quit();
//        }
//        delete workingThread;
//    }
}

bool DirectoryAnalyzer::isRunning() const {
//    return workingThread && workingThread->isRunning();
}

DirectoryAnalyzer::~DirectoryAnalyzer() {
    stopScanning();
}

