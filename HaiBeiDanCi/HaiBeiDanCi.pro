#-------------------------------------------------
#
# Project created by QtCreator 2018-07-30T14:43:53
#
#-------------------------------------------------

QT       += core gui webenginewidgets multimedia sql concurrent

#greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = HaiBeiDanCi
TEMPLATE = app
CONFIG += no_batch

# version, APP_VERSION is string "major.minor.patch"
VERSION = 0.9.3
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


SOURCES += \
        main.cpp \
        mainwindow.cpp \
    mediaplayer.cpp \
    memoryitem.cpp \
    studylist.cpp \
    studywindow.cpp \
    word.cpp \
    wordbook.cpp \
    wordcard.cpp \
    worddb.cpp \
    wordview.cpp \
    dictschemehandler.cpp \
    temporaryfilemanager.cpp \
    browserwindow.cpp \
    mysettings.cpp \
    preferencesdialog.cpp \
    downloadmanager.cpp \
    serverdatadialog.cpp \
    serverclientprotocol.cpp \
    aboutdialog.cpp \
    helpfunc.cpp \
    mediafilemanager.cpp \
    applicationuser.cpp \
    registerdialog.cpp \
    serveruseragent.cpp \
    logindialog.cpp \
    token.cpp \
    clienttoken.cpp \
    applicationversion.cpp \
    autoupgrader.cpp \
    ../Upgrader/upgradedata.cpp \
    servercommunicator.cpp \
    filedownloader.cpp \
    bookdownloader.cpp \
    versionchecker.cpp

HEADERS += \
        mainwindow.h \
    mediaplayer.h \
    memoryitem.h \
    studylist.h \
    studywindow.h \
    word.h \
    wordbook.h \
    wordcard.h \
    worddb.h \
    wordview.h \
    dictschemehandler.h \
    temporaryfilemanager.h \
    browserwindow.h \
    mysettings.h \
    preferencesdialog.h \
    downloadmanager.h \
    serverdatadialog.h \
    serverclientprotocol.h \
    aboutdialog.h \
    helpfunc.h \
    mediafilemanager.h \
    applicationuser.h \
    registerdialog.h \
    serveruseragent.h \
    logindialog.h \
    token.h \
    clienttoken.h \
    applicationversion.h \
    autoupgrader.h \
    ../Upgrader/upgradedata.h \
    ../golddict/sptr.hh \
    servercommunicator.h \
    filedownloader.h \
    bookdownloader.h \
    versionchecker.h

FORMS += \
    mainwindow.ui \
    studywindow.ui \
    browserwindow.ui \
    preferencesdialog.ui \
    serverdatadialog.ui \
    aboutdialog.ui \
    registerdialog.ui \
    logindialog.ui

RESOURCES += \
    haibeidanci.qrc

TRANSLATIONS = myapp_zh_CN.ts

DISTFILES += \
    info.json

RC_ICONS = HaiBeiDanCi.ico
