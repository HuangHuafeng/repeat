#-------------------------------------------------
#
# Project created by QtCreator 2018-09-27T13:58:03
#
#-------------------------------------------------

QT       += core gui widgets

TARGET = Upgrader
TEMPLATE = app

# version, APP_VERSION is string "major.minor.patch"
VERSION = 1.8.4
DEFINES += APP_VERSION=\\\"$$VERSION\\\"

# quazip
unix:!macx {
}

macx {
    INCLUDEPATH += /Users/huafeng/Qt/zlib-1.2.11
    LIBS += -L/Users/huafeng/Qt/zlib-1.2.11 -lz
    INCLUDEPATH += /Users/huafeng/Qt/quazip-0.7.6/quazip
    LIBS += -L/Users/huafeng/Qt/quazip-0.7.6/quazip/ -lquazip
}

win32 {
    CONFIG(release, debug|release): LIBS += -LC:/Qt/quazip-0.7.6/quazip/release/ -lquazip
    else:win32:CONFIG(debug, debug|release): LIBS += -LC:/Qt/quazip-0.7.6/quazip/debug/ -lquazipd

    INCLUDEPATH += C:/Qt/quazip-0.7.6/quazip
    DEPENDPATH += C:/Qt/quazip-0.7.6/quazip

    LIBS += -LC:/Qt/zlib-1.2.11/ -lzlib
    INCLUDEPATH += C:/Qt/zlib-1.2.11
    DEPENDPATH += C:/Qt/zlib-1.2.11
}

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
        main.cpp \
        upgrader.cpp \
    upgradedata.cpp \
    ../HaiBeiDanCi/applicationversion.cpp

HEADERS += \
        upgrader.h \
    upgradedata.h \
    ../HaiBeiDanCi/applicationversion.h

FORMS += \
        upgrader.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

