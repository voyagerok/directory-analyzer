#include "directory-analyzer.h"
#include "directory-tree-item.h"
#include "utils.h"

#include <QThreadPool>
#include <stack>
#include <queue>
#include <QTextStream>>

//void DirectoryWalker::run() {
//    calcStats();
//}

//void DirectoryWalker::calcStats() {
//    qint64 size = 0;
//    int filesCount = 0;
//    std::stack<QDir> dirs;
//    dirs.push(node->getDirectory());

//    while (!dirs.empty() && !interruptFlag) {
//        QDir dir = dirs.top();
//        dirs.pop();
//        QFileInfoList content = dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot, QDir::SortFlag::Size);
//        for (const auto &fileInfo : content) {
//            if (fileInfo.isDir()) {
//                dirs.push(QDir{fileInfo.absoluteFilePath()});
//            } else {
//                size += fileInfo.size();
//                ++filesCount;
//            }
//        }
//    }
//    emit directoryUpdated(node, size, filesCount);
//}

//void DirectoryAnalyzer::updateDirectoryNode(DirectoryTreeItem *node) {
//    if (activeNodes.find(node) == activeNodes.end()) {
//        DirectoryWalker *worker = new DirectoryWalker(node, interruptFlag);
//        connect(worker, &DirectoryWalker::directoryUpdated, this, &DirectoryAnalyzer::onNodeUpdated);
////        QThreadPool::globalInstance()->start(worker, QThread::Priority::LowPriority);
//        pool->start(worker);
//        activeNodes.insert(node);
//    }
//}

//void DirectoryAnalyzer::onNodeUpdated(DirectoryTreeItem *node, qint64 newSize, int newFilesCount) {
//    node->updateFilesCount(newFilesCount);
//    node->updateTotalSize(newSize);
//    activeNodes.erase(node);
//    emit nodeUpdated(node);
//}

//void DirectoryAnalyzer::requestInterruption() {
//    mutex.lock();
//    interruptFlag = true;
//    mutex.unlock();
////    QThreadPool::globalInstance()->waitForDone();
//    pool->waitForDone();
//    interruptFlag = false;
//}

void DirectoryTreeBuilderThread::run() {
    QString status;
    QTextStream(&status) << "Building index for root directory "
                         << node->getDirectory().absolutePath()
                         << "...";
    emit statusChanged(status);
    buildDirectoryTree(node);
}

void DirectoryTreeBuilderThread::buildDirectoryTree(DirectoryTreeItem *root) {
    std::queue<DirectoryTreeItem *> nodes;
    nodes.push(root);

    while(!nodes.empty() && !isInterruptionRequested()) {
        DirectoryTreeItem *item = nodes.front();
        nodes.pop();
        item->populate();
        emit buildingCompleted(item);
        for (int i = 0; i < item->getChildrenCount(); ++i) {
            DirectoryTreeItem *child = item->getChildAtIndex(i);
            nodes.push(child);
        }
    }

    emit buildFinished(root);
}

void DirectoryTreeSizeCalculator::run() {
    // not very nice, but does the job
    try {
        QString status;
        QTextStream(&status) << "Calculating size for root directory "
                             << root->getDirectory().absolutePath()
                             << "...";
        emit statusChanged(status);
        auto result = calcSize(root);
        emit doneCalculation(root, result);
        emit done();
    } catch (std::exception &) {
        return;
    }
}

static void mergeMaps(const FileTypesInfoStorage &source, FileTypesInfoStorage &dest) {
    for (auto &elem : source) {
        auto elemIter = dest.find(elem.first);
        if (elemIter == dest.end()) {
            dest.insert(elem);
        } else {
            elemIter->second.filesCount += elem.second.filesCount;
            elemIter->second.totalSize += elem.second.totalSize;
        }
    }
}

FileTypesInfoStorage DirectoryTreeSizeCalculator::calcSize(DirectoryTreeItem *node) {
    if (interruptFlag) {
        throw std::exception();
   }

    FileTypesInfoStorage storage;
    storage[FILE_TYPE_KEY_ALL] = DirectorySizeInfo{0, 0};

    QDir dir = node->getDirectory();
    QFileInfoList content = dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);
    for (auto &entry : content) {
        if (entry.isFile()) {
            storage[FILE_TYPE_KEY_ALL].filesCount ++;
            storage[FILE_TYPE_KEY_ALL].totalSize += entry.size();
            QString ext = Utils::extensionFromFileInfo(entry);
            auto extensionIter = storage.find(ext);
            if (extensionIter == storage.end()) {
                storage.insert(std::make_pair(ext, DirectorySizeInfo{entry.size(), 1}));
            } else {
                extensionIter->second.filesCount ++;
                extensionIter->second.totalSize += entry.size();
            }
        }
    }
    for (int i = 0; i < node->getChildrenCount(); ++i) {
        auto result = calcSize(node->getChildAtIndex(i));
        mergeMaps(result, storage);
    }
    emit doneCalculation(node, storage);
    return storage;
}

void DirectoryTreeAnalyzer::start(DirectoryTreeItem *node) {
    stopBuilderThread();

//    QString status;
//    QTextStream(&status) << "Building index for root directory "
//                         << node->getDirectory().absolutePath();
//    emit statusChanged(status);

    emit progressStarted();
    builderThread = new DirectoryTreeBuilderThread(node);
    connect(builderThread, &DirectoryTreeBuilderThread::buildingCompleted, this, &DirectoryTreeAnalyzer::buildCompleted);
    connect(builderThread, &DirectoryTreeBuilderThread::buildFinished, this, &DirectoryTreeAnalyzer::startSizeCalculating);
    connect(builderThread, &DirectoryTreeBuilderThread::statusChanged, this, &DirectoryTreeAnalyzer::statusChanged);
    builderThread->start(QThread::IdlePriority);
}

void DirectoryTreeAnalyzer::stopBuilderThread() {
    if (builderThread) {
        if (builderThread->isRunning()) {
            disconnect(builderThread, 0, 0, 0);
            builderThread->requestInterruption();
            builderThread->wait();
        }
        delete builderThread;
        builderThread = Q_NULLPTR;
    }
}

void DirectoryTreeAnalyzer::stopCalculationThreads() {
    mutex.lock();
    poolInterruptionFlag = true;
    mutex.unlock();
//    QThreadPool::globalInstance()->waitForDone();
    pool->waitForDone();
    poolInterruptionFlag = false;
}

void DirectoryTreeAnalyzer::startSizeCalculating(DirectoryTreeItem *root) {
    stopCalculationThreads();

//    QString status;
//    QTextStream(&status) << "Calculating size for root directory "
//                         << root->getDirectory().absolutePath();
//    emit statusChanged(status);

    DirectoryTreeSizeCalculator *worker = new DirectoryTreeSizeCalculator(root, poolInterruptionFlag);
    connect(worker, &DirectoryTreeSizeCalculator::doneCalculation, this, &DirectoryTreeAnalyzer::onSizeUpdated);
    connect(worker, &DirectoryTreeSizeCalculator::done, this, &DirectoryTreeAnalyzer::done);
    connect(worker, &DirectoryTreeSizeCalculator::statusChanged, this, &DirectoryTreeAnalyzer::statusChanged);
//    QThreadPool::globalInstance()->start(worker);
    pool->start(worker, QThread::IdlePriority);
}

void DirectoryTreeAnalyzer::onSizeUpdated(DirectoryTreeItem *node, FileTypesInfoStorage storage) {
    DirectorySizeInfo allTypesInfo = storage[FILE_TYPE_KEY_ALL];
    node->updateFilesCount(allTypesInfo.filesCount);
    node->updateTotalSize(allTypesInfo.totalSize);
    node->updateFileTypesInfo(std::move(storage));
    emit sizeUpdated(node);
}

bool DirectoryTreeAnalyzer::isRunning() const {
    return (builderThread && builderThread->isRunning()) || pool->activeThreadCount() > 0;
}

void DirectoryTreeAnalyzer::stop() {
    stopBuilderThread();
    stopCalculationThreads();
    emit done();
}
