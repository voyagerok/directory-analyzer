#ifndef DIRECTORYTREEMODEL_H
#define DIRECTORYTREEMODEL_H

#include <QAbstractItemModel>
#include <memory>

class DirectoryTreeItem;

class DirectoryTreeModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    DirectoryTreeModel() = default;
    ~DirectoryTreeModel();

    int columnCount(const QModelIndex &parentIndex = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column, const QModelIndex &parentIndex = QModelIndex()) const override;
    int rowCount(const QModelIndex &parentIndex = QModelIndex()) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

private:
    DirectoryTreeItem *root = Q_NULLPTR;
};

#endif // DIRECTORYTREEMODEL_H
