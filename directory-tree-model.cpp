#include "directory-tree-model.h"
#include "directory-tree-item.h"

#define COLUMNS_COUNT 4
static const char *columnLabels[COLUMNS_COUNT] = {"Path", "Files Count", "Size", "Directories"};

DirectoryTreeModel::~DirectoryTreeModel() {
    if (root) {
        delete root;
    }
}

int DirectoryTreeModel::columnCount(const QModelIndex&) const {
    if (root == nullptr) {
        return 0;
    }
    return root->dataSize();
}

QVariant DirectoryTreeModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || role != Qt::DisplayRole) {
        return QVariant();
    }
    DirectoryTreeItem *item = static_cast<DirectoryTreeItem *>(index.internalPointer());
    return item->getData(index.column());
}

int DirectoryTreeModel::rowCount(const QModelIndex &parentIndex) const {
    if (root == nullptr) {
        return 0;
    }
    if (!parentIndex.isValid()) {
        return root->childSize();
    } else {
        DirectoryTreeItem *item = static_cast<DirectoryTreeItem *>(parentIndex.internalPointer());
        return item->childSize();
    }
}

QModelIndex DirectoryTreeModel::index(int row, int column, const QModelIndex &parentIndex) const {
    if (root == nullptr) {
        return QModelIndex();
    }
    DirectoryTreeItem *item;
    if (!parentIndex.isValid()) {
        item = root;
    } else {
        item = static_cast<DirectoryTreeItem *>(parentIndex.internalPointer());
    }
    return createIndex(row, column, item);
}

QVariant DirectoryTreeModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation != Qt::Orientation::Horizontal ||
            role != Qt::DisplayRole ||
            section >= COLUMNS_COUNT)
    {
        return QVariant();
    }

    return QVariant(columnLabels[section]);
}

Qt::ItemFlags DirectoryTreeModel::flags(const QModelIndex&) const {
    return Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable;
}
