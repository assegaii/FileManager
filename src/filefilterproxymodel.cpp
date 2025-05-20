#include "filefilterproxymodel.h"
#include <QRegularExpression>

FileFilterProxyModel::FileFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent) {}

void FileFilterProxyModel::setCurrentPath(const QString &path) {
    m_currentPath = path;
    m_fsModel = qobject_cast<QFileSystemModel*>(sourceModel());
    invalidateFilter();
}

void FileFilterProxyModel::setFilterString(const QString &filter) {
    m_filter = filter;
    invalidateFilter();
}

bool FileFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const {
    if(!m_fsModel || m_currentPath.isEmpty()) return true;

    QModelIndex index = m_fsModel->index(sourceRow, 0, sourceParent);
    QFileInfo fileInfo = m_fsModel->fileInfo(index);

    // Проверяем принадлежность к текущей директории
    bool isInCurrentDir = fileInfo.dir().absolutePath() == m_currentPath;

    // Всегда показываем саму текущую директорию
    if(fileInfo.absoluteFilePath() == m_currentPath) return true;

    // Показываем только элементы из текущей директории
    if(!isInCurrentDir) return false;

    // Фильтрация по имени
    if(m_filter.isEmpty()) return true;

    // Поддержка wildcards (* и ?)
    QRegularExpression regex(
        QRegularExpression::wildcardToRegularExpression(m_filter),
        QRegularExpression::CaseInsensitiveOption
        );

    return regex.match(fileInfo.fileName()).hasMatch();
}
