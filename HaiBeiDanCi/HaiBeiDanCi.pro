#-------------------------------------------------
#
# Project created by QtCreator 2018-07-30T14:43:53
#
#-------------------------------------------------

QT       += core gui webenginewidgets multimedia sql concurrent

#greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = HaiBeiDanCi
TEMPLATE = app

# version, APP_VERSION is string "0.9.1"
VERSION = 0.9.1
DEFINES += APP_VERSION=\\\"$$VERSION\\\"

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
    svragt.cpp \
    serverdatadownloader.cpp \
    helpfunc.cpp \
    mediafilemanager.cpp \
    applicationuser.cpp \
    registerdialog.cpp \
    serveruseragent.cpp \
    logindialog.cpp

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
    ../golddict/sptr.hh \
    temporaryfilemanager.h \
    browserwindow.h \
    mysettings.h \
    preferencesdialog.h \
    downloadmanager.h \
    serverdatadialog.h \
    serverclientprotocol.h \
    aboutdialog.h \
    svragt.h \
    serverdatadownloader.h \
    helpfunc.h \
    mediafilemanager.h \
    applicationuser.h \
    registerdialog.h \
    serveruseragent.h \
    logindialog.h

FORMS += \
    mainwindow.ui \
    studywindow.ui \
    browserwindow.ui \
    preferencesdialog.ui \
    serverdatadialog.ui \
    aboutdialog.ui \
    registerdialog.ui \
    logindialog.ui

SUBDIRS += \
    HaiBeiDanCi.pro

RESOURCES += \
    haibeidanci.qrc

TRANSLATIONS = myapp_zh_CN.ts

DISTFILES += \
    info.json

RC_ICONS = HaiBeiDanCi.ico
