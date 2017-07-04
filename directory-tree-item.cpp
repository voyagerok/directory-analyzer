#include "directory-tree-item.h"

bool DirectoryTreeItem::setDataAtIndex(int index, const QVariant &value) {
    if (index < 0 || index >= data.size()) {
        return false;
    }
    data[index] = value;
    return true;
}

