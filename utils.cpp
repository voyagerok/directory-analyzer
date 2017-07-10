#include "utils.h"

#include <QStringList>

namespace Utils {

QString size_human(qint64 rawSize) {
    double num = rawSize;
    QStringList list;
    list << "KB" << "MB" << "GB" << "TB";

    QStringListIterator i(list);
    QString unit("bytes");

    while(num >= 1000.0 && i.hasNext())
     {
        unit = i.next();
        num /= 1000.0;
    }
    return QString().setNum(num,'f',2)+" "+unit;
}

QString extensionFromFileInfo(const QFileInfo &fileInfo) {
    QString ext = fileInfo.suffix();
    return ext.prepend("*.");
}

}

