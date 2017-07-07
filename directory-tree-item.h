#ifndef DIRECTORYTREEITEM_H
#define DIRECTORYTREEITEM_H

#include <QVector>
#include <QVariant>
#include <QList>
#include <QDir>
#include <tuple>

class DirectoryTreeItemPrivate;

class DirectoryTreeItem
{
public:
    DirectoryTreeItem(const QString &absPath, DirectoryTreeItem *parent = Q_NULLPTR);
    DirectoryTreeItem(const QDir &dirObject, DirectoryTreeItem *parent = Q_NULLPTR);
    ~DirectoryTreeItem();

    DirectoryTreeItem(const DirectoryTreeItem &) = delete;
    DirectoryTreeItem &operator=(const DirectoryTreeItem &) = delete;

    DirectoryTreeItem *getChildAtIndex(int childIndex) { return children.at(childIndex); }
    int getSubdirsCount() const { return children.size(); }
    int getDataObjectsCount() const;
    QVariant getDataObject(int index) const;
    DirectoryTreeItem *getParent() { return parent; }

    QDir getDirectory() const;
    QString getAbsolutePath() const;
    QString getDirectoryName() const;
    int getTotalFilesCount() const;
    qint64 getTotalSize() const;
    bool isLeaf() const { return children.empty(); }
    bool hasParent() const { return parent != Q_NULLPTR; }

    void updateTotalSize(qint64 totalSize);
    void updateFilesCount(int filesCount);

    void appendChild(DirectoryTreeItem *child) { children.push_back(child); }

private:
    QVector<DirectoryTreeItem *> children;
    DirectoryTreeItem *parent;

    DirectoryTreeItemPrivate *privImpl;
};

#endif // DIRECTORYTREEITEM_H
