#ifndef MAINWINDOW_H
#define MAINWINDOW_H

//ФИЛЬТРЫ
#include "filefilterproxymodel.h"
#include "qlistview.h"
#include "qtreeview.h"

#include <QMainWindow>
#include <QFileSystemModel>
#include <QSortFilterProxyModel>
#include <QRegularExpression>
#include <QStack>
#include <QActionGroup>

#include <QShortcut>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_treeView_clicked(const QModelIndex &index);

    void on_listView_doubleClicked(const QModelIndex &index);
    //Мое
    void showContextMenu(const QPoint &pos);
    void onCut();
    void onCopy();
    void onPaste();
    void onDelete();
    void onRename();

    void onSortTriggered(QAction* action);

    //Сортировка
    void setupSortMenu();

    // файлы
    void onCreateActionTriggered(QAction *action);
    void showCreateMenu();
    void onCreateFolder();
    void setupCreateMenu();
    void createNewFile(const QString &prefix, const QString &extension);

    void on_backButton_clicked();

    void on_forwardButton_clicked();

    void on_updateButton_clicked();

    void on_searchEdit_textChanged(const QString &text);

    void on_sortButton_clicked();

    void on_lineEdit_textChanged(const QString &arg1);

private:
    Ui::MainWindow *ui;


        QFileSystemModel* dirModel;
        QFileSystemModel* fileModel;
        FileFilterProxyModel* proxyModel;
        QStack<QString> backStack;
        QStack<QString> forwardStack;
        QString currentPath;
        QTreeView* treeView;
        QListView* listView;

    QActionGroup *sortActionGroup;
    Qt::SortOrder currentSortOrder;

    // Новые файлы
    QActionGroup *createActionGroup;
    QModelIndex getCurrentIndex() const;
    QString currentDirectory() const;
    QString loadStyleSheet(const QString &path);

    QString generateUniqueName(const QString &path, const QString &name);

    void updateNavigation(const QString &newPath);
    void updateFileSystemView(const QString &path);
    void navigateTo(const QString &path);
    void updateNavButtons();
    void refreshView();
    void setupNewTab(int index, const QString& path);
    bool copyDirectory(const QString &src, const QString &dest);

    void setupHotkeys();

friend class TestMainWindow;

signals:
    void sortOrderChanged();
};
#endif // MAINWINDOW_H
