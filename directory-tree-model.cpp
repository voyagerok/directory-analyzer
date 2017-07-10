#include "directory-tree-model.h"
#include "directory-tree-item.h"
#include "directory-analyzer.h"

#include <cmath>
#include <QTextStream>

#define COLUMNS_COUNT 4
static const char *columnLabels[COLUMNS_COUNT] = {"Path", "Subdirectories", "Files Count", "Size"};

DirectoryTreeModel::DirectoryTreeModel(const QString &rootDir, QObject *parent):
    QAbstractItemModel(parent)
{
//    connect(dirAnalyzer, &DirectoryAnalyzer::nodeConstructed, this, &DirectoryTreeModel::onNodeConstructed);
//    connect(dirAnalyzer, &DirectoryAnalyzer::nodeUpdated, this, &DirectoryTreeModel::onNodeUpdated);
//    dirAnalyzer->beginScanning(rootDir);
//    root->populate();
    analyzer = new DirectoryTreeAnalyzer();
    connect(analyzer, &DirectoryTreeAnalyzer::buildCompleted, this, &DirectoryTreeModel::onNodeConstructed);
    connect(analyzer, &DirectoryTreeAnalyzer::sizeUpdated, this, &DirectoryTreeModel::onNodeUpdated);
    connect(analyzer, &DirectoryTreeAnalyzer::done, this, &DirectoryTreeModel::progressFinished);
    connect(analyzer, &DirectoryTreeAnalyzer::statusChanged, this, &DirectoryTreeModel::statusChanged);
    connect(analyzer, &DirectoryTreeAnalyzer::progressStarted, this, &DirectoryTreeModel::progressStarted);

//    emit progressStarted();
//    analyzer->start(root);

//    analyzer = new DirectoryAnalyzer();
//    connect(analyzer, &DirectoryAnalyzer::nodeUpdated, this, &DirectoryTreeModel::onNodeUpdated);
}

DirectoryTreeModel::~DirectoryTreeModel() {
//    dirAnalyzer->stopScanning();
//    dirAnalyzer->stopWorkers();
    delete analyzer;
//    delete analyzer;
//    if () {
//        delete root;
//    }
    qDeleteAll(rootItems);
}


//void DirectoryTreeModel::buildIndex(const QString &rootDir) {
//    root = new DirectoryTreeItem(rootDir, 0);
//    analyzer->start(root);
//}

void DirectoryTreeModel::buildIndex(const QModelIndex &index) {
    if (index.isValid()) {
        emit beginResetModel();
        analyzer->stop();
        DirectoryTreeItem *node = static_cast<DirectoryTreeItem *>(index.internalPointer());
        node->reset();
        analyzer->start(node);
        emit endResetModel();
    }
}

void DirectoryTreeModel::stopProgress() {
    if (analyzer->isRunning()) {
         analyzer->stop();
    }
}

void DirectoryTreeModel::setRootPath(const QString &rootPath) {
    emit beginResetModel();
    stopProgress();
//    if (root != Q_NULLPTR) {
//        delete root;
//    }
    qDeleteAll(rootItems);
    rootItems.clear();
//    root = new DirectoryTreeItem(rootPath, 0);
    rootItems.push_back(new DirectoryTreeItem(rootPath, 0));
    emit endResetModel();
//    notifyNodeUpdated(root);
}

void DirectoryTreeModel::setDefaultPath() {
    emit beginResetModel();
    stopProgress();
    qDeleteAll(rootItems);
    rootItems.clear();
#ifdef Q_OS_WIN
    QFileInfoList drivesInfo = QDir::drives();
    for (int i = 0; i < drivesInfo.size(); ++i) {
        DirectoryTreeItem *item = new DirectoryTreeItem(drivesInfo[i].absoluteFilePath(), i);
        rootItems.push_back(item);
    }
#else
    rootItems.push_back(new DirectoryTreeItem(QDir::root(), 0));
    emit endResetModel();
#endif
}

//#ifdef Q_OS_WIN
//    QFileInfoList drives = QDir::drv
//}


int DirectoryTreeModel::columnCount(const QModelIndex&) const {
    if (rootItems.empty()) {
        return 0;
    }
    return rootItems[0]->getDataObjectsCount();
}

QVariant DirectoryTreeModel::data(const QModelIndex &index, int role) const {
    using ExtractionStatus = DirectoryTreeItem::DataExtractionStatus;
    if (!index.isValid() || role != Qt::DisplayRole) {
        return QVariant();
    }
    DirectoryTreeItem *item = static_cast<DirectoryTreeItem *>(index.internalPointer());
    ExtractionStatus status;
    auto result = item->getDataObject(index.column(), status);
//    if (status == ExtractionStatus::INVALID) {
//        analyzer->updateDirectoryNode(item);
//    }
    return result;
}

int DirectoryTreeModel::rowCount(const QModelIndex &parentIndex) const {
//    if (root == nullptr) {
//        return 0;
//    }
    if (rootItems.empty()) {
        return 0;
    }
    DirectoryTreeItem *node;
    if (!parentIndex.isValid()) {
//        node = root;
//        return 1;
        return rootItems.size();
    } else {
        node = static_cast<DirectoryTreeItem *>(parentIndex.internalPointer());
        return node->getChildrenCount();
    }
}

QModelIndex DirectoryTreeModel::index(int row, int column, const QModelIndex &parentIndex) const {
//    if (root == nullptr) {
//        return QModelIndex();
//    }
    if (rootItems.empty()) {
        return QModelIndex();
    }
    DirectoryTreeItem *item;
    if (!parentIndex.isValid()) {
        item = rootItems[row];
    } else {
//        item = static_cast<DirectoryTreeItem *>(parentIndex.internalPointer());
        DirectoryTreeItem *parentItem = static_cast<DirectoryTreeItem *>(parentIndex.internalPointer());
        item = parentItem->getChildAtIndex(row);
    }
    return createIndex(row, column, item);
}

QVariant DirectoryTreeModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation != Qt::Orientation::Horizontal
            || role != Qt::DisplayRole
            || section >= COLUMNS_COUNT)
    {
        return QVariant();
    }

    return QVariant(columnLabels[section]);
}

QModelIndex DirectoryTreeModel::parent(const QModelIndex &index) const {
    if (!index.isValid()) {
        return QModelIndex();
    }
    DirectoryTreeItem *node = static_cast<DirectoryTreeItem *>(index.internalPointer());
    if (node->hasParent()) {
        return createIndex(node->getParent()->getPosition(), index.column(), node->getParent());
    } else {
        return QModelIndex();
    }
}

Qt::ItemFlags DirectoryTreeModel::flags(const QModelIndex&) const {
    return Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsUserCheckable | Qt::ItemFlag::ItemIsSelectable;
}

bool DirectoryTreeModel::hasChildren(const QModelIndex &index) const {
    if (!index.isValid()) {
        return true;
    }
    DirectoryTreeItem *node = static_cast<DirectoryTreeItem *>(index.internalPointer());
//    return node->getSubdirsCount() > 0;
    return node->getChildrenCount() > 0;
}

QVector<std::tuple<QString, qint64>> DirectoryTreeModel::getFileTypesInfo(const QModelIndex &index) {
    using ResultType = QVector<std::tuple<QString, qint64>>;
    ResultType result;
    if (!index.isValid()) {
        return result;
    }
    DirectoryTreeItem *item = static_cast<DirectoryTreeItem *>(index.internalPointer());
    FileTypesInfoStorage &storage = item->getFileTypesInfo();
    for (const auto &infoObject : storage) {
        qint64 totalSize = infoObject.second.totalSize;
        int filesCount = infoObject.second.filesCount;
        qint64 avgSize = static_cast<qint64>(std::ceil(static_cast<double>(totalSize) / filesCount));
        result.push_back(std::make_tuple(infoObject.first, avgSize));
    }

    return result;
}

//bool DirectoryTreeModel::canFetchMore(const QModelIndex &index) const {
//    if (!index.isValid()) {
//        return false;
//    }
//    DirectoryTreeItem *item = static_cast<DirectoryTreeItem *>(index.internalPointer());
//    QDir dir = item->getDirectory();
//    auto result = item->getSubdirsCount() > item->getChildrenCount();
//    return result;
//}

//void DirectoryTreeModel::fetchMore(const QModelIndex &index) {
//    if (!index.isValid()) {
//        return;
//    }
//    DirectoryTreeItem *item = static_cast<DirectoryTreeItem *>(index.internalPointer());
//    item->populate();
////    dirAnalyzer->populate(item);
////    emit layoutChanged();
//}

void DirectoryTreeModel::notifyNodeUpdated(DirectoryTreeItem *node) {
    QModelIndex topLeft = createIndex(node->getPosition(), 0, node);
    QModelIndex bottomRight = createIndex(node->getPosition(), node->getDataObjectsCount() - 1, node);
    emit dataChanged(topLeft, bottomRight);
}

void DirectoryTreeModel::onNodeConstructed(DirectoryTreeItem *node) {
    notifyNodeUpdated(node);
}

void DirectoryTreeModel::onNodeUpdated(DirectoryTreeItem *node) {
    notifyNodeUpdated(node);
}

void DirectoryTreeModel::onStatsUpdated(DirectoryTreeItem *node, qint64 size, int filesCount) {
    node->updateFilesCount(filesCount);
    node->updateTotalSize(size);
    notifyNodeUpdated(node);
}
