#ifndef DIRECTORYTREEMODEL_H
#define DIRECTORYTREEMODEL_H

#include <QAbstractItemModel>
#include <memory>
#include <tuple>

class DirectoryTreeItem;
class DirectoryAnalyzer;
class DirectoryTreeAnalyzer;

class DirectoryTreeModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    DirectoryTreeModel(const QString &rootDir, QObject *parent = Q_NULLPTR);
    ~DirectoryTreeModel();

//    void setRootDirectory(const QString &rootDir);

    int columnCount(const QModelIndex &parentIndex = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column, const QModelIndex &parentIndex = QModelIndex()) const override;
    int rowCount(const QModelIndex &parentIndex = QModelIndex()) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QModelIndex parent(const QModelIndex &index) const override;
//    bool canFetchMore(const QModelIndex &index) const override;
//    void fetchMore(const QModelIndex &index) override;
    bool hasChildren(const QModelIndex &index) const override;
    QVector<std::tuple<QString, qint64>> getFileTypesInfo(const QModelIndex &index);

    void buildIndex(const QString &rootDirectory);

private:
    DirectoryTreeItem *root = Q_NULLPTR;
//    DirectoryAnalyzer *analyzer;
    DirectoryTreeAnalyzer *analyzer;

    void notifyNodeUpdated(DirectoryTreeItem *node);
private slots:
    void onNodeConstructed(DirectoryTreeItem *node);
    void onNodeUpdated(DirectoryTreeItem *node);
    void onStatsUpdated(DirectoryTreeItem *node, qint64 size, int filesCount);

signals:
    void progressStarted();
    void progressFinished();
    void statusChanged(QString status);
};

#endif // DIRECTORYTREEMODEL_H
