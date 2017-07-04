#ifndef DIRECTORYTREEITEM_H
#define DIRECTORYTREEITEM_H

#include <QVector>
#include <QVariant>
#include <QList>

class DirectoryTreeItem
{
public:
    explicit DirectoryTreeItem(const QVector<QVariant> &data, int itemPosition = -1, DirectoryTreeItem *parent = Q_NULLPTR):
        data { data }, parent { parent }, itemPosition { itemPosition } {}
    ~DirectoryTreeItem() { qDeleteAll(children.begin(), children.end()); }

    DirectoryTreeItem *getChildAtIndex(int childIndex) { return children.at(childIndex); }
    int childSize() const { return children.size(); }
    int dataSize() const { return data.size(); }
    QVariant getData(int column) const { return data.at(column); }
    DirectoryTreeItem *getParent() { return parent; }
    int getItemPosition() const { return itemPosition; }
    bool setDataAtIndex(int index, const QVariant &value);

private:
    QVector<DirectoryTreeItem *> children;
    QVector<QVariant> data;
    DirectoryTreeItem *parent;
    int itemPosition;
};

#endif // DIRECTORYTREEITEM_H
