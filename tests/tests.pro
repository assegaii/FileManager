QT += core gui widgets testlib

CONFIG += c++17 testlib


# Добавляем все исходники из src
SOURCES += \
    ../src/mainwindow.cpp \
    ../src/filefilterproxymodel.cpp \
    tst_filemanager_test.cpp

# Добавляем все заголовочные файлы
HEADERS += \
    ../src/filefilterproxymodel.h \
    ../src/mainwindow.h

# Добавляем все формы .ui, если они используются в тестах
FORMS += \
    ../src/mainwindow.ui


RESOURCES += \
    ../src/resourses.qrc

DISTFILES += \
    ../src/context_menu.qss \
    ../src/paste_menu.qss

INCLUDEPATH += $$PWD/../src

qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
