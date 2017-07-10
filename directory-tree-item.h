#ifndef DIRECTORYTREEITEM_H
#define DIRECTORYTREEITEM_H

#include <QVector>
#include <QVariant>
#include <QList>
#include <QDir>
#include <tuple>
#include <QThread>

class DirectoryTreeItemPrivate;

#define FILE_TYPE_KEY_ALL "*.*"

struct DirectorySizeInfo {
    qint64 totalSize;
    int filesCount;
};

using FileTypesInfoStorage = std::map<QString, DirectorySizeInfo>;

class DirectoryTreeItem {
public:
    enum class DataExtractionStatus {
        VALID, INVALID
    };

    DirectoryTreeItem(const QString &absPath, int position, DirectoryTreeItem *parent = Q_NULLPTR);
    DirectoryTreeItem(const QDir &dirObject, int position, DirectoryTreeItem *parent = Q_NULLPTR);
    ~DirectoryTreeItem();

    DirectoryTreeItem(const DirectoryTreeItem &) = delete;
    DirectoryTreeItem &operator=(const DirectoryTreeItem &) = delete;

    void populate();

    DirectoryTreeItem *getChildAtIndex(int childIndex) { return children.at(childIndex); }
    int getChildrenCount() const { return children.size(); }
    int getSubdirsCount() const;
    int getDataObjectsCount() const;
    QVariant getDataObject(int index, DataExtractionStatus &status) const;
    DirectoryTreeItem *getParent() { return parent; }
    int getPosition() const { return position; }

    QDir getDirectory() const;
    QString getAbsolutePath() const;
    QString getDirectoryName() const;
    int getTotalFilesCount() const;
    qint64 getTotalSize() const;
    bool isLeaf() const { return children.empty(); }
    bool hasParent() const { return parent != Q_NULLPTR; }

    FileTypesInfoStorage &getFileTypesInfo();

    void updateTotalSize(qint64 totalSize);
    void updateFilesCount(int filesCount);
    void incrementTotalSize(qint64 incValue);
    void incrementFilesCount(int incValue);
    void updateFileTypesInfo(const FileTypesInfoStorage &fileTypesInfo);
    void updateFileTypesInfo(FileTypesInfoStorage &&fileTypesInfo);

    void appendChild(DirectoryTreeItem *child) { children.push_back(child); }

private:
    QVector<DirectoryTreeItem *> children;
    DirectoryTreeItem *parent;
    int position;

    DirectoryTreeItemPrivate *privImpl;
};

#endif // DIRECTORYTREEITEM_H
