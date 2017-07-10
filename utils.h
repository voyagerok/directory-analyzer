#ifndef UTILS_H
#define UTILS_H

#include <QString>
#include <QFileInfo>

namespace Utils {

QString size_human(qint64 rawSize);
QString extensionFromFileInfo(const QFileInfo &fileInfo);

}

#endif // UTILS_H
