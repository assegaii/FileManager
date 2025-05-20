QT       += core gui widgets testlib

CONFIG += c++17 testlib

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    filefilterproxymodel.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    filefilterproxymodel.h \
    mainwindow.h

FORMS += \
    mainwindow.ui


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resourses.qrc

DISTFILES += \
    context_menu.qss \
    paste_menu.qss
