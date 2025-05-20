#ifndef FILEFILTERPROXYMODEL_H
#define FILEFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>
#include <QFileSystemModel>

class FileFilterProxyModel : public QSortFilterProxyModel {
    Q_OBJECT
public:
    explicit FileFilterProxyModel(QObject *parent = nullptr);

    void setCurrentPath(const QString &path);
    void setFilterString(const QString &filter);

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

private:
    QString m_filter;
    QString m_currentPath;
    QFileSystemModel *m_fsModel = nullptr;
};

#endif // FILEFILTERPROXYMODEL_H
