#include "directory-tree-item.h"
#include "utils.h"

#include <QRunnable>
#include <QMutex>
#include <QThreadPool>
#include <stack>
#include <map>

using DataExtractionStatus = DirectoryTreeItem::DataExtractionStatus;

class DirectoryTreeItemPrivate {
    static constexpr QFlags<QDir::Filter> filterFlags =
            QDir::Dirs
            | QDir::NoDotAndDotDot
            | QDir::NoSymLinks
            | QDir::Hidden;

public:
    DirectoryTreeItemPrivate(const QString &absPath):
        dir {QDir{absPath}},
        data {std::make_tuple(dir.isRoot() ? dir.absolutePath() : dir.dirName(), dir.entryList(filterFlags).size(), -1, -1)}
    {
    }
    DirectoryTreeItemPrivate(const QDir &dir):
        dir {dir},
        data {std::make_tuple(dir.isRoot() ? dir.absolutePath() : dir.dirName(), dir.entryList(filterFlags).size(), -1, -1)}
    {
    }

    DirectoryTreeItemPrivate(const DirectoryTreeItemPrivate&) = delete;
    DirectoryTreeItemPrivate &operator=(const DirectoryTreeItemPrivate&) = delete;

    enum DataMembers { DIR_NAME = 0, DIRS_COUNT, FILES_COUNT, TOTAL_SIZE, THRESHOLD};

    QDir getDir() const { return dir; }
    QString getAbsolutePath() const { return dir.absolutePath(); }
    QString getDirectoryName() const { return dir.dirName(); }
    int &getTotalFilesCount() { return std::get<FILES_COUNT>(data); }
    qint64 &getTotalSize() { return std::get<TOTAL_SIZE>(data); }
    int getDataSize() const { return std::tuple_size<decltype (data)>::value; }
    int getSubsirsCount() const { return std::get<DIRS_COUNT>(data); }
    FileTypesInfoStorage &getFileTypesInfo() { return fileTypesInfo; }

    QVariant getDataAtPosition(int position, DataExtractionStatus &status);

    bool isValidDataIndex(int index) const { return index >= 0 && index < THRESHOLD; }
    bool sizeIsValid() const { return std::get<TOTAL_SIZE>(data) >= 0; }
    bool filesCountIsValid() const { return std::get<FILES_COUNT>(data) >= 0; }

//    using FileTypesInfoStorage = std::map<QString, std::pair<qint64, int>>;

    void updateFilesCount(int filesCount) { std::get<FILES_COUNT>(data) = filesCount; }
    void updateTotalSize(qint64 totalSize) { std::get<TOTAL_SIZE>(data) = totalSize; }
    void updateFileTypesInfo(const FileTypesInfoStorage &fileTypesInfo) { this->fileTypesInfo = fileTypesInfo; }
    void updateFileTypesInfo(FileTypesInfoStorage &&fileTypesInfo) { std::swap(this->fileTypesInfo, fileTypesInfo); }

    void resetSizeInfo() { updateFilesCount(-1); updateTotalSize(-1); fileTypesInfo.clear(); }

private:
    QDir dir;
    std::tuple<QString, int, int, qint64> data;
    FileTypesInfoStorage fileTypesInfo;
};

constexpr QFlags<QDir::Filter> DirectoryTreeItemPrivate::filterFlags;

QVariant DirectoryTreeItemPrivate::getDataAtPosition(int position, DataExtractionStatus &status) {
    status = DataExtractionStatus::VALID;
    switch (position) {
    case DIR_NAME:
        return std::get<DIR_NAME>(data);
    case DIRS_COUNT:
        return std::get<DIRS_COUNT>(data);
    case FILES_COUNT:
        if (filesCountIsValid()) {
            return QVariant(std::get<FILES_COUNT>(data));
        } else {
            status = DataExtractionStatus::INVALID;
            return QVariant("Undefined");
        }
    case TOTAL_SIZE:
        if (sizeIsValid()) {
            return QVariant(Utils::size_human(std::get<TOTAL_SIZE>(data)));
        } else {
            status = DataExtractionStatus::INVALID;
            return QVariant("Undefined");
        }
    default:
        status = DataExtractionStatus::INVALID;
        return QVariant();
    }
}

DirectoryTreeItem::DirectoryTreeItem(const QString &absPath, int position, DirectoryTreeItem *parent):
    parent {parent}, position {position}, privImpl {new DirectoryTreeItemPrivate(absPath)} {}

DirectoryTreeItem::DirectoryTreeItem(const QDir &dir, int position, DirectoryTreeItem *parent):
    parent {parent}, position {position}, privImpl {new DirectoryTreeItemPrivate(dir)} {}

DirectoryTreeItem::~DirectoryTreeItem() {
    delete privImpl;
    qDeleteAll(children);
}

QString DirectoryTreeItem::getAbsolutePath() const {
    return privImpl->getAbsolutePath();
}

QString DirectoryTreeItem::getDirectoryName() const {
    return privImpl->getDirectoryName();
}

QDir DirectoryTreeItem::getDirectory() const {
    return privImpl->getDir();
}

int DirectoryTreeItem::getTotalFilesCount() const {
    return privImpl->getTotalFilesCount();
}

qint64 DirectoryTreeItem::getTotalSize() const {
    return privImpl->getTotalSize();
}

int DirectoryTreeItem::getDataObjectsCount() const {
    return privImpl->getDataSize();
}

QVariant DirectoryTreeItem::getDataObject(int index, DataExtractionStatus &status) const {
    return privImpl->getDataAtPosition(index, status);
}

int DirectoryTreeItem::getSubdirsCount() const {
    return privImpl->getSubsirsCount();
}

void DirectoryTreeItem::updateFilesCount(int filesCount) {
    privImpl->updateFilesCount(filesCount);
}

void DirectoryTreeItem::updateTotalSize(qint64 totalSize) {
    privImpl->updateTotalSize(totalSize);
}

void DirectoryTreeItem::incrementFilesCount(int incValue) {
    privImpl->getTotalFilesCount() += incValue;
}

void DirectoryTreeItem::incrementTotalSize(qint64 incValue) {
    privImpl->getTotalSize() += incValue;
}

void DirectoryTreeItem::updateFileTypesInfo(const FileTypesInfoStorage &fileTypesInfo) {
    privImpl->updateFileTypesInfo(fileTypesInfo);
}

void DirectoryTreeItem::updateFileTypesInfo(FileTypesInfoStorage &&fileTypesInfo) {
    privImpl->updateFileTypesInfo(std::move(fileTypesInfo));
}

FileTypesInfoStorage &DirectoryTreeItem::getFileTypesInfo() {
    return privImpl->getFileTypesInfo();
}

void DirectoryTreeItem::reset() {
    qDeleteAll(children);
    children.clear();
    privImpl->resetSizeInfo();
}

void DirectoryTreeItem::populate() {
    QDir dir = getDirectory();
    QFileInfoList dirContent = dir.entryInfoList(QDir::Dirs
                                                 | QDir::NoDotAndDotDot
                                                 | QDir::Drives
                                                 | QDir::Readable
                                                 | QDir::Hidden
                                                 | QDir::NoSymLinks, QDir::SortFlag::Size);
    int position = 0;
    for (const auto &fileInfo : dirContent) {
        if (fileInfo.fileName() == "sys" || fileInfo.fileName() == "dev" || fileInfo.fileName() == "proc") {
            continue;
        }
        DirectoryTreeItem *childItem = new DirectoryTreeItem(fileInfo.absoluteFilePath(), position++, this);
        appendChild(childItem);
    }
}
