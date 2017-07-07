#include "directory-tree-item.h"

//bool DirectoryTreeItem::setDataAtIndex(int index, const QVariant &value) {
//    if (index < 0 || index >= data.size()) {
//        return false;
//    }
//    data[index] = value;
//    return true;
//}

//QDir DirectoryTreeItem::getDirectory() const {

//}

class DirectoryTreeItemPrivate {
public:
    DirectoryTreeItemPrivate(const QString &absPath):
        data {std::make_tuple(QDir{absPath}, -1, -1)} {}
    DirectoryTreeItemPrivate(const QDir &dir):
        data {std::make_tuple(dir, -1, -1)} {}

    DirectoryTreeItemPrivate(const DirectoryTreeItemPrivate&) = delete;
    DirectoryTreeItemPrivate &operator=(const DirectoryTreeItemPrivate&) = delete;

    enum DataMembers { DIR_OBJECT = 0, FILES_COUNT, TOTAL_SIZE, THRESHOLD};

    QDir getDir() const { return std::get<DIR_OBJECT>(data); }
    QString getAbsolutePath() const { return std::get<DIR_OBJECT>(data).absolutePath(); }
    QString getDirectoryName() const { return std::get<DIR_OBJECT>(data).dirName(); }
    int getTotalFilesCount() const { return std::get<FILES_COUNT>(data); }
    qint64 getTotalSize() const { return std::get<TOTAL_SIZE>(data); }
    int getDataSize() const { return std::tuple_size<decltype (data)>::value; }

    QVariant getDataAtPosition(int position) const;

    bool isValidDataIndex(int index) const { return index >= 0 && index < THRESHOLD; }
    bool sizeIsValid() const { return std::get<TOTAL_SIZE>(data) >= 0; }
    bool filesCountIsValid() const { return std::get<FILES_COUNT>(data) >= 0; }

    void updateFilesCount(int filesCount) { std::get<FILES_COUNT>(data) = filesCount; }
    void updateTotalSize(qint64 totalSize) { std::get<TOTAL_SIZE>(data) = totalSize; }

private:
    std::tuple<QDir, int, qint64> data;
};

QVariant DirectoryTreeItemPrivate::getDataAtPosition(int position) const {
    switch (position) {
    case DIR_OBJECT:
        return QVariant(std::get<DIR_OBJECT>(data).dirName());
    case FILES_COUNT:
        return QVariant(std::get<FILES_COUNT>(data));
    case TOTAL_SIZE:
        return QVariant(std::get<TOTAL_SIZE>(data));
    default:
        return QVariant();
    }
}

DirectoryTreeItem::DirectoryTreeItem(const QString &absPath, DirectoryTreeItem *parent):
    parent {parent}, privImpl {new DirectoryTreeItemPrivate(absPath)} {}

DirectoryTreeItem::DirectoryTreeItem(const QDir &dir, DirectoryTreeItem *parent):
    parent {parent}, privImpl {new DirectoryTreeItemPrivate(dir)} {}

inline
DirectoryTreeItem::~DirectoryTreeItem() {
    delete privImpl;
    qDeleteAll(children);
}

inline
QString DirectoryTreeItem::getAbsolutePath() const {
    return privImpl->getAbsolutePath();
}

inline
QString DirectoryTreeItem::getDirectoryName() const {
    return privImpl->getDirectoryName();
}

inline
QDir DirectoryTreeItem::getDirectory() const {
    return privImpl->getDir();
}

inline
int DirectoryTreeItem::getTotalFilesCount() const {
    return privImpl->getTotalFilesCount();
}

inline
qint64 DirectoryTreeItem::getTotalSize() const {
    return privImpl->getTotalSize();
}

inline
int DirectoryTreeItem::getDataObjectsCount() const {
    return privImpl->getDataSize() + 1; // plus subdirs
}

inline
QVariant DirectoryTreeItem::getDataObject(int index) const {
    if (index == privImpl->getDataSize())
        return QVariant(getSubdirsCount());
    else
        return privImpl->getDataAtPosition(index);
}

inline
void DirectoryTreeItem::updateFilesCount(int filesCount) {
    privImpl->updateFilesCount(filesCount);
}

inline
void DirectoryTreeItem::updateTotalSize(qint64 totalSize) {
    privImpl->updateTotalSize(totalSize);
}

