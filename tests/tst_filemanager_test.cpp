#include <QtTest>
#include <QCoreApplication>
#include <QTemporaryDir>
#include <QFileSystemModel>
#include <QListView>
#include <QTreeView>
#include <QClipboard>
#include <QMessageBox>
#include <QPushButton>
#include <QLineEdit>
#include <QSignalSpy>
#include <QDir>

#include "mainwindow.h"
#include "ui_mainwindow.h"

class TestMainWindow : public QObject
{
    Q_OBJECT

public:
    TestMainWindow();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void testNavigation();
    void testCreateFolder();
    void testDeleteFile();
    void testCopyPasteFile();
    void testRenameFile();
    void testSorting();

private:
    void handleDialog(QMessageBox::StandardButton button, int delay = 0);
    void waitForModelUpdate();
    void verifyFileSystemState(const QString& path, bool shouldExist);
    void cleanTestDirectory();

    MainWindow* mw = nullptr;
    QTemporaryDir* tempDir = nullptr;
    QString testFilePath;
};

TestMainWindow::TestMainWindow() {}

void TestMainWindow::initTestCase()
{
    tempDir = new QTemporaryDir;
    QVERIFY2(tempDir->isValid(), "Не удалось создать временную директорию.");
}

void TestMainWindow::cleanupTestCase()
{
    delete tempDir;
}

void TestMainWindow::init()
{
    cleanTestDirectory();

    testFilePath = tempDir->filePath("testfile.txt");
    QFile file(testFilePath);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write("test content");
    file.close();

    mw = new MainWindow;
    mw->show();
    mw->navigateTo(tempDir->path());
    QApplication::clipboard()->clear();
}

void TestMainWindow::cleanup()
{
    delete mw;
    mw = nullptr;
    cleanTestDirectory();
}

void TestMainWindow::cleanTestDirectory()
{
    QDir dir(tempDir->path());
    dir.setNameFilters(QStringList() << "*");
    dir.setFilter(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);

    for (const QString& entry : dir.entryList()) {
        QString path = dir.filePath(entry);
        QFileInfo fi(path);

        if (fi.isDir())
            QDir(path).removeRecursively();
        else
            QFile::remove(path);
    }
}

void TestMainWindow::verifyFileSystemState(const QString& path, bool shouldExist)
{
    for (int i = 0; i < 5; ++i) {
        bool exists = QFile::exists(path);
        bool inModel = mw->fileModel->index(path).isValid();

        if (exists == shouldExist && inModel == shouldExist) return;
        QTest::qWait(200);
    }

    QFAIL(qPrintable(QString("File state mismatch for: %1\nExists: %2\nIn model: %3")
                         .arg(path)
                         .arg(QFile::exists(path))
                         .arg(mw->fileModel->index(path).isValid())));
}

void TestMainWindow::testNavigation()
{
    QString newDir = tempDir->filePath("test_dir");
    QDir().mkdir(newDir);

    mw->navigateTo(newDir);
    QCOMPARE(mw->currentPath, newDir);
    QCOMPARE(mw->backStack.size(), 2);

    mw->on_backButton_clicked();
    QCOMPARE(mw->currentPath, tempDir->path());
    QCOMPARE(mw->forwardStack.size(), 1);
}

void TestMainWindow::testCreateFolder()
{
    int initialCount = mw->fileModel->rowCount(mw->fileModel->index(tempDir->path()));
    mw->onCreateFolder();
    waitForModelUpdate();

    int newCount = mw->fileModel->rowCount(mw->fileModel->index(tempDir->path()));
    QVERIFY(newCount > initialCount);
}

void TestMainWindow::handleDialog(QMessageBox::StandardButton button, int delay)
{
    QTimer::singleShot(delay, [=]() {
        if (auto msgBox = qobject_cast<QMessageBox*>(QApplication::activeModalWidget())) {
            QAbstractButton* btn = nullptr;
            foreach (QAbstractButton* b, msgBox->buttons()) {
                QString text = b->text().remove("&");
                if (text.contains("Да", Qt::CaseInsensitive)) {
                    btn = b;
                    break;
                }
            }

            QStringList buttonTexts;
            foreach (QAbstractButton* b, msgBox->buttons()) {
                buttonTexts << b->text().replace("&", "");
            }

            QVERIFY2(btn, qPrintable(QString("Кнопка не найдена. Доступные кнопки: %1")
                                         .arg(buttonTexts.join(", "))));
            QTest::mouseClick(btn, Qt::LeftButton);
        }
    });
}

void TestMainWindow::waitForModelUpdate()
{
    QTest::qWait(500);
    mw->proxyModel->invalidate();
    QTest::qWait(200);
}

void TestMainWindow::testDeleteFile()
{
    QString testFile = tempDir->filePath("to_delete.txt");
    QVERIFY(QFile::copy(testFilePath, testFile));

    mw->updateFileSystemView(tempDir->path());
    waitForModelUpdate();

    QModelIndex sourceIndex = mw->fileModel->index(testFile);
    QModelIndex proxyIndex = mw->proxyModel->mapFromSource(sourceIndex);
    mw->ui->listView->setCurrentIndex(proxyIndex);

    handleDialog(QMessageBox::Yes, 500);
    mw->onDelete();

    mw->refreshView();
    waitForModelUpdate();

    verifyFileSystemState(testFile, false);
}

void TestMainWindow::testCopyPasteFile()
{

}

void TestMainWindow::testRenameFile()
{
    QString newName = "renamed.txt";
    QString newPath = tempDir->filePath(newName);


    QFile::remove(newPath);


    QModelIndex sourceIndex = mw->fileModel->index(testFilePath);
    QModelIndex proxyIndex = mw->proxyModel->mapFromSource(sourceIndex);
    mw->ui->listView->setCurrentIndex(proxyIndex);
    mw->ui->listView->edit(proxyIndex);


    QLineEdit* editor = mw->ui->listView->findChild<QLineEdit*>();
    QVERIFY2(editor, "Редактор имени файла (QLineEdit) не найден");
    editor->setText(newName);
    QTest::keyClick(editor, Qt::Key_Enter);

    waitForModelUpdate();
    verifyFileSystemState(newPath, true);
}

void TestMainWindow::testSorting()
{

}

QTEST_MAIN(TestMainWindow)
#include "tst_filemanager_test.moc"
