#include "mainwindow.h"
#include "ui_mainwindow.h"


#include <QTreeView>
#include <QListView>
#include <QDebug>
#include <QDesktopServices>
#include <QMenu>
#include <QMimeData>
#include <QMessageBox>
#include <QClipboard>
#include <QRegularExpression>

void MainWindow::setupHotkeys() {
    // Удаление (Del)
    QShortcut* deleteShortcut = new QShortcut(QKeySequence::Delete, this);
    connect(deleteShortcut, &QShortcut::activated, this, &MainWindow::onDelete);

    // Вырезать (Ctrl+X)
    QShortcut* cutShortcut = new QShortcut(QKeySequence::Cut, this);
    connect(cutShortcut, &QShortcut::activated, this, &MainWindow::onCut);

    // Копировать (Ctrl+C)
    QShortcut* copyShortcut = new QShortcut(QKeySequence::Copy, this);
    connect(copyShortcut, &QShortcut::activated, this, &MainWindow::onCopy);

    // Вставить (Ctrl+V)
    QShortcut* pasteShortcut = new QShortcut(QKeySequence::Paste, this);
    connect(pasteShortcut, &QShortcut::activated, this, &MainWindow::onPaste);

    // Переименовать (F2)
    QShortcut* renameShortcut = new QShortcut(Qt::Key_F2, this);
    connect(renameShortcut, &QShortcut::activated, this, &MainWindow::onRename);
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // Инициализация моделей
    // Настройка модели для TreeView
    dirModel = new QFileSystemModel(this);
    dirModel->setFilter(QDir::AllDirs | QDir::NoDotAndDotDot);
    dirModel->setRootPath("");
    dirModel->setNameFilterDisables(false);
    dirModel->setReadOnly(true);

    // Настройка TreeView
    ui->treeView->setModel(dirModel);
    ui->treeView->hideColumn(1);
    ui->treeView->hideColumn(2);
    ui->treeView->hideColumn(3);
    ui->treeView->setHeaderHidden(true);

    // Настройка ListView
    fileModel = new QFileSystemModel(this);
    fileModel->setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);
    fileModel->setReadOnly(false);

    // searchModel = new QFileSystemModel(this);
    // searchModel->setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);
    // searchModel->setRootPath("");

    proxyModel = new FileFilterProxyModel(this);
    proxyModel->setSourceModel(fileModel);
    //proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    proxyModel->setFilterKeyColumn(0);
    proxyModel->setCurrentPath(currentPath);

    ui->listView->setModel(proxyModel);

    currentPath = QDir::homePath();
    updateFileSystemView(currentPath);

    // Дополнительные настройки
    ui->treeView->setIconSize(QSize(32, 32));
    ui->listView->setIconSize(QSize(48, 48));
    ui->treeView->setSortingEnabled(true);
    ui->listView->setEditTriggers(QAbstractItemView::EditKeyPressed);

    // NEW: Настройка контекстного меню
    ui->listView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->listView, &QListView::customContextMenuRequested,
            this, &MainWindow::showContextMenu);

    // NEW: Подключение кнопок
    connect(ui->cutButton, &QPushButton::clicked, this, &MainWindow::onCut);
    connect(ui->copyButton, &QPushButton::clicked, this, &MainWindow::onCopy);
    connect(ui->pasteButton, &QPushButton::clicked, this, &MainWindow::onPaste);
    connect(ui->deleteButton, &QPushButton::clicked, this, &MainWindow::onDelete);
    connect(ui->renameButton, &QPushButton::clicked, this, &MainWindow::onRename);

    // Улучшенные папки
    setupCreateMenu();
    connect(ui->createButton, &QPushButton::clicked, this, &MainWindow::showCreateMenu);

    // Инициализация состояний кнопок
    ui->backButton->setEnabled(false);
    ui->forwardButton->setEnabled(false);
    // Отключаем поиск
    //ui->searchEdit->setReadOnly(true);

    //СОРТИРОВКА!
    currentSortOrder = Qt::AscendingOrder;
    setupSortMenu();


    //UPD: не схлопывать сплиттер
    ui->splitter->setCollapsible(0, false);
    ui->splitter->setCollapsible(1, false);

    setupHotkeys();
}

MainWindow::~MainWindow()
{
    delete ui;
}

// Контекстное меню
void MainWindow::showContextMenu(const QPoint &pos)
{
    QModelIndex index = ui->listView->indexAt(pos);
    QMenu menu;

    menu.setStyleSheet(loadStyleSheet("://context_menu.qss"));

    if (index.isValid()) {
        menu.addAction("Вырезать", this, &MainWindow::onCut);
        menu.addAction("Копировать", this, &MainWindow::onCopy);
        menu.addAction("Вставить", this, &MainWindow::onPaste);
        menu.addAction("Удалить", this, &MainWindow::onDelete);
        menu.addAction("Переименовать", this, &MainWindow::onRename);
    } else {
        menu.addAction("Создать папку", this, &MainWindow::onCreateFolder);
        menu.addAction("Вставить", this, &MainWindow::onPaste);
    }

    menu.exec(ui->listView->viewport()->mapToGlobal(pos));
}

QString MainWindow::loadStyleSheet(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) return "";
    return QLatin1String(file.readAll());
}
// Вырезать
void MainWindow::onCut()
{
    QModelIndex index = proxyModel->mapToSource(ui->listView->currentIndex());
    if (!index.isValid()) return;

    QMimeData *mimeData = new QMimeData();
    QList<QUrl> urls;
    urls << QUrl::fromLocalFile(fileModel->filePath(index));
    mimeData->setUrls(urls);
    mimeData->setProperty("cutAction", true);
    QApplication::clipboard()->setMimeData(mimeData);
}
// Копировать
void MainWindow::onCopy()
{
    QModelIndex index = proxyModel->mapToSource(ui->listView->currentIndex());
    if (!index.isValid()) return;

    QMimeData *mimeData = new QMimeData();
    QList<QUrl> urls;
    urls << QUrl::fromLocalFile(fileModel->filePath(index));
    mimeData->setUrls(urls);
    QApplication::clipboard()->setMimeData(mimeData);
}
// Вставить
void MainWindow::onPaste()
{
    const QMimeData *mimeData = QApplication::clipboard()->mimeData();
    if (!mimeData || !mimeData->hasUrls()) return;

    QString destDir = currentPath;
    bool cutAction = mimeData->property("cutAction").toBool();

    for (const QUrl &url : mimeData->urls()) {
        QString srcPath = url.toLocalFile();
        QFileInfo srcInfo(srcPath);
        QString destPath = QDir(destDir).filePath(srcInfo.fileName());

        if (QFile::exists(destPath)) {
            QMessageBox msgBox;
            msgBox.setWindowTitle(tr("Перезапись"));
            msgBox.setText(tr("Файл уже существует. Перезаписать?"));
            msgBox.setIcon(QMessageBox::Question);

            QPushButton *overwriteButton = msgBox.addButton(tr("Да, перезаписать"), QMessageBox::YesRole);
            QPushButton *skipButton = msgBox.addButton(tr("Нет, пропустить"), QMessageBox::NoRole);
            // Стиль?
            msgBox.setStyleSheet(loadStyleSheet("://paste_menu.qss"));


            msgBox.exec();

            if (msgBox.clickedButton() == skipButton) {
                continue;
            }
        }

        if (cutAction) {
            QFile::rename(srcPath, destPath);
        } else {
            if (srcInfo.isDir()) {
                copyDirectory(srcPath, destPath);
            } else {
                QFile::copy(srcPath, destPath);
            }
        }
    }
    refreshView();
}
// Удалить
void MainWindow::onDelete()
{
    QModelIndex index = proxyModel->mapToSource(ui->listView->currentIndex());
    if (!index.isValid()) return;

    QFileInfo info = fileModel->fileInfo(index);

    // Создаем кастомный MessageBox
    QMessageBox msgBox;
    msgBox.setWindowTitle("Подтверждение удаления");
    msgBox.setText(QString("Вы действительно хотите удалить <b>'%1'</b> безвозвратно?").arg(info.fileName()));
    msgBox.setIcon(QMessageBox::Question);

    // Добавляем кнопки
    QPushButton *yesButton = msgBox.addButton("Да, удалить", QMessageBox::YesRole);
    QPushButton *noButton = msgBox.addButton("Нет", QMessageBox::NoRole);

    // Применяем стиль
    msgBox.setStyleSheet(loadStyleSheet("://paste_menu.qss"));

    // Дополнительная стилизация кнопок
    yesButton->setStyleSheet("min-width: 80px; padding: 5px;");
    noButton->setStyleSheet("min-width: 80px; padding: 5px;");

    msgBox.exec();

    if (msgBox.clickedButton() != yesButton) return;

    // Удаление файла/папки
    bool success = false;
    if(info.isDir()) {
        success = QDir(info.filePath()).removeRecursively();
    } else {
        success = QFile::remove(info.filePath());
    }

    if(!success) {
        QMessageBox::critical(this, tr("Ошибка"), tr("Не удалось удалить объект"));
    }
    refreshView();
}
void MainWindow::refreshView()
{
    fileModel->setRootPath(fileModel->rootPath());
    dirModel->setRootPath(dirModel->rootPath());
    updateFileSystemView(currentPath);
}
// Переименовать
void MainWindow::onRename()
{
    QModelIndex proxyIndex = ui->listView->currentIndex();
    if (!proxyIndex.isValid()) return;

    QModelIndex sourceIndex = proxyModel->mapToSource(proxyIndex);
    QFileInfo oldInfo = fileModel->fileInfo(sourceIndex);

    fileModel->fetchMore(sourceIndex.parent());
    // Запускаем стандартное редактирование имени
    ui->listView->edit(proxyIndex);

    // Получаем указатель на редактор
    QLineEdit* editor = qobject_cast<QLineEdit*>(ui->listView->indexWidget(proxyIndex));
    if (!editor) return;

    // Обработка завершения редактирования
    connect(editor, &QLineEdit::editingFinished, this, [=]() {
        QString newName = editor->text().trimmed();
        QString newPath = QDir(oldInfo.path()).filePath(newName);

        // Проверяем существование через модель
        bool exists = false;
        QModelIndex parentIndex = fileModel->index(oldInfo.path());
        for (int i = 0; i < fileModel->rowCount(parentIndex); ++i) {
            QModelIndex child = fileModel->index(i, 0, parentIndex);
            if (fileModel->fileName(child) == newName && child != sourceIndex) {
                exists = true;
                break;
            }
        }

        if (exists) {
            QMessageBox msgBox;
            msgBox.setWindowTitle(tr("Конфликт имён"));
            msgBox.setText(
                tr("Файл с именем '%1' уже существует.\n"
                   "Хотите переименовать в '%2'?")
                    .arg(newName, generateUniqueName(oldInfo.path(), newName))
                );
            msgBox.setInformativeText(tr("Выберите действие:"));
            msgBox.setIcon(QMessageBox::Question);

            QPushButton *renameButton = msgBox.addButton(tr("Переименовать"), QMessageBox::AcceptRole);
            QPushButton *cancelButton = msgBox.addButton(QMessageBox::Cancel);


            msgBox.setStyleSheet(loadStyleSheet("://paste_menu.qss"));
            msgBox.exec();

            if (msgBox.clickedButton() == renameButton) {
                // Автоматическое переименование
                QString uniqueName = generateUniqueName(oldInfo.path(), newName);
                editor->setText(uniqueName);
                fileModel->setData(sourceIndex, uniqueName);
            }

            else {
                // Отмена - возвращаем исходное имя
                editor->setText(oldInfo.fileName());
            }
        } else {
            // Если конфликта нет - применяем новое имя
            if (!fileModel->setData(sourceIndex, newName)) {
                QMessageBox::critical(this, tr("Ошибка"), tr("Не удалось переименовать файл"));
            }
        }

        refreshView();
    });
}

// Генерация уникального имени
QString MainWindow::generateUniqueName(const QString &path, const QString &name)
{
    QFileInfo fi(name);
    QString base = fi.baseName();
    QString ext = fi.suffix().isEmpty() ? "" : "." + fi.suffix();
    int counter = 1;
    QString newName;

    // Получаем список существующих имен через модель
    QModelIndex parentIndex = fileModel->index(path);
    int rowCount = fileModel->rowCount(parentIndex);
    QSet<QString> existingNames;

    for (int i = 0; i < rowCount; ++i) {
        QModelIndex childIndex = fileModel->index(i, 0, parentIndex);
        existingNames.insert(fileModel->fileName(childIndex));
    }

    do {
        newName = QString("%1 (%2)%3").arg(base).arg(counter++).arg(ext);
    } while (existingNames.contains(newName));

    return newName;
}
// Создать папку
void MainWindow::onCreateFolder()
{
    QString baseName = tr("Новая папка");
    QString newName = baseName;
    int counter = 1;

    while(QFileInfo::exists(QDir(currentPath).filePath(newName))) {
        newName = QString("%1 (%2)").arg(baseName).arg(counter++);
    }

    QModelIndex parentIndex = fileModel->index(currentPath);
    QModelIndex newIndex = fileModel->mkdir(parentIndex, newName);

    if(newIndex.isValid()) {
        ui->listView->edit(proxyModel->mapFromSource(newIndex));
    }
}

// Существующие слоты
void MainWindow::on_treeView_clicked(const QModelIndex &index)
{
    QString newPath = dirModel->fileInfo(index).absoluteFilePath();
    navigateTo(newPath);
}

void MainWindow::on_listView_doubleClicked(const QModelIndex &proxyIndex)
{
    QModelIndex sourceIndex = proxyModel->mapToSource(proxyIndex);
    QFileInfo info = fileModel->fileInfo(sourceIndex);

    if(info.isDir()) {
        navigateTo(info.absoluteFilePath());
    } else {
        QDesktopServices::openUrl(QUrl::fromLocalFile(info.absoluteFilePath()));
    }
}

void MainWindow::navigateTo(const QString &path)
{
    if(path == currentPath) return;

    backStack.push(currentPath);
    forwardStack.clear();
    currentPath = path;

    updateFileSystemView(path);
    updateNavButtons();
}

void MainWindow::updateNavButtons()
{
    ui->backButton->setEnabled(!backStack.isEmpty());
    ui->forwardButton->setEnabled(!forwardStack.isEmpty());
}
void MainWindow::on_backButton_clicked()
{
    if(backStack.isEmpty()) return;

    forwardStack.push(currentPath);
    currentPath = backStack.pop();
    updateFileSystemView(currentPath);
    updateNavButtons();
}


void MainWindow::on_forwardButton_clicked()
{
    if(forwardStack.isEmpty()) return;

    backStack.push(currentPath);
    currentPath = forwardStack.pop();
    updateFileSystemView(currentPath);
    updateNavButtons();
}


void MainWindow::on_updateButton_clicked()
{
    updateFileSystemView(currentPath);
}


void MainWindow::updateFileSystemView(const QString &path)
{
    fileModel->setRootPath(path);
    dirModel->setRootPath(path);

    QModelIndex fileIndex = fileModel->index(path);
    ui->listView->setRootIndex(proxyModel->mapFromSource(fileIndex));

    QModelIndex treeIndex = dirModel->index(path);
    ui->treeView->expand(treeIndex);
    ui->treeView->setCurrentIndex(treeIndex);

    ui->pathEdit->setText(QDir::toNativeSeparators(path));

    //Сортировка
    int sortColumn = proxyModel->sortColumn();
    Qt::SortOrder sortOrder = proxyModel->sortOrder();

    proxyModel->invalidate();
    proxyModel->sort(sortColumn, sortOrder);

}
// Поиск
void MainWindow::on_searchEdit_textChanged(const QString &text)
{
    proxyModel->setFilterString(text);
}

bool MainWindow::copyDirectory(const QString &src, const QString &dest)
{
    QDir sourceDir(src);
    if(!sourceDir.exists()) return false;

    if(!QDir().mkpath(dest)) return false;

    for(const QString &entry : sourceDir.entryList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot)) {
        QString srcPath = sourceDir.filePath(entry);
        QString destPath = QDir(dest).filePath(entry);

        QFileInfo fi(srcPath);
        if(fi.isDir()) {
            if(!copyDirectory(srcPath, destPath)) return false;
        } else {
            if(!QFile::copy(srcPath, destPath)) return false;
        }
    }
    return true;
}


// НОВОЕ!!!! СОРТИРОВКА!!!!
void MainWindow::setupSortMenu()
{
    sortActionGroup = new QActionGroup(this);

    QAction *sortByName = new QAction("По имени", this);
    QAction *sortByDate = new QAction("По дате изменения", this);
    QAction *sortBySize = new QAction("По размеру", this);
    QAction *sortByType = new QAction("По типу", this);

    // Установка данных для идентификации
    sortByName->setData(0);
    sortByDate->setData(3);
    sortBySize->setData(1);
    sortByType->setData(2);

    // Добавляем действия в группу
    sortActionGroup->addAction(sortByName);
    sortActionGroup->addAction(sortByDate);
    sortActionGroup->addAction(sortBySize);
    sortActionGroup->addAction(sortByType);

    // Подключаем сигналы
    connect(sortActionGroup, &QActionGroup::triggered,
            this, &MainWindow::onSortTriggered);

    QAction *sortOrderAction = new QAction(
        currentSortOrder == Qt::AscendingOrder
            ? "▲ По возрастанию"
            : "▼ По убыванию",
        this
        );
    sortOrderAction->setData(-1);
    sortActionGroup->addAction(sortOrderAction);

    // Обновляем текст при изменении порядка
    connect(this, &MainWindow::sortOrderChanged, [=](){
        sortOrderAction->setText(
            currentSortOrder == Qt::AscendingOrder
                ? "▲ По возрастанию"
                : "▼ По убыванию"
            );
    });
}

void MainWindow::on_sortButton_clicked()
{
    QMenu sortMenu;
    sortMenu.setStyleSheet(loadStyleSheet("://context_menu.qss"));
    sortMenu.addActions(sortActionGroup->actions());
    sortMenu.exec(ui->sortButton->mapToGlobal(QPoint(0, ui->sortButton->height())));
}

void MainWindow::onSortTriggered(QAction* action)
{
    int column = action->data().toInt();

    if(column == -1) {
        // Переключение порядка сортировки
        currentSortOrder = (currentSortOrder == Qt::AscendingOrder)
                               ? Qt::DescendingOrder
                               : Qt::AscendingOrder;

        // Применяем новый порядок к текущей сортировке
        int currentColumn = proxyModel->sortColumn();
        proxyModel->sort(currentColumn, currentSortOrder);
    } else {
        // Установка новой сортировки
        proxyModel->sort(column, currentSortOrder);
    }

    // Обновление вида
    ui->listView->update();
    if(column == -1) {
        // ... существующий код ...
        emit sortOrderChanged(); // Новый сигнал
    }

}

void MainWindow::on_lineEdit_textChanged(const QString &text)
{
    if (!ui->searchEdit->text().isEmpty()) {
        return;
    }

    // Разделить ввод на маски по запятым или пробелам
    QStringList filters;
    if (!text.isEmpty()) {
        filters = text.split(QRegularExpression("[, ]+"), Qt::SkipEmptyParts);
    }

    // Установить фильтры для модели файлов
    fileModel->setNameFilters(filters);
    fileModel->setNameFilterDisables(false);

    // Обновить представление
    updateFileSystemView(currentPath);
}

//новые файлы
void MainWindow::showCreateMenu()
{
    QMenu createMenu;
    createMenu.setStyleSheet(loadStyleSheet("://context_menu.qss"));
    createMenu.addActions(createActionGroup->actions());

    QPoint pos = ui->createButton->mapToGlobal(QPoint(0, ui->createButton->height()));
    createMenu.exec(pos);
}
void MainWindow::setupCreateMenu()
{
    createActionGroup = new QActionGroup(this);

    QAction *createFolder = new QAction("Папка", this);
    QAction *createDoc = new QAction("Документ Word (.docx)", this);
    QAction *createTxt = new QAction("Текстовый файл (.txt)", this);
    QAction *createPpt = new QAction("Презентация (.pptx)", this);
    QAction *createXls = new QAction("Таблица Excel (.xlsx)", this);

    // Привязываем данные для идентификации
    createFolder->setData("folder");
    createDoc->setData("docx");
    createTxt->setData("txt");
    createPpt->setData("pptx");
    createXls->setData("xlsx");

    createActionGroup->addAction(createFolder);
    createActionGroup->addAction(createDoc);
    createActionGroup->addAction(createTxt);
    createActionGroup->addAction(createPpt);
    createActionGroup->addAction(createXls);

    connect(createActionGroup, &QActionGroup::triggered,
            this, &MainWindow::onCreateActionTriggered);
}
void MainWindow::onCreateActionTriggered(QAction *action)
{
    QString type = action->data().toString();
    QString prefix, extension;

    if(type == "folder") {
        onCreateFolder();
        return;
    }
    else if(type == "docx") {
        prefix = "Новый документ";
        extension = ".docx";
    }
    else if(type == "txt") {
        prefix = "Новый текстовый документ";
        extension = ".txt";
    }
    else if(type == "pptx") {
        prefix = "Новая презентация";
        extension = ".pptx";
    }
    else if(type == "xlsx") {
        prefix = "Новая таблица";
        extension = ".xlsx";
    }

    createNewFile(prefix, extension);
}
void MainWindow::createNewFile(const QString &prefix, const QString &extension)
{
    QString baseName = prefix;
    QString newName = baseName;
    int counter = 1;

    // Ищем уникальное имя
    while(QFile::exists(QDir(currentPath).filePath(newName + extension))) {
        newName = QString("%1 (%2)").arg(baseName).arg(counter++);
    }

    QString fullPath = QDir(currentPath).filePath(newName + extension);
    QFile file(fullPath);

    if(file.open(QIODevice::WriteOnly)) {
        file.close();
        refreshView();


    } else {
        QMessageBox::critical(this, "Ошибка",
                              QString("Не удалось создать файл:\n%1").arg(file.errorString()));
    }
}


