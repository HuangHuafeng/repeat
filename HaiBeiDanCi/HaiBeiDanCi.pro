#-------------------------------------------------
#
# Project created by QtCreator 2018-07-30T14:43:53
#
#-------------------------------------------------

QT       += core gui webenginewidgets multimedia sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = HaiBeiDanCi
TEMPLATE = app

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
    ../golddict/gddebug.cc

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
    ../golddict/gddebug.hh \
    ../golddict/sptr.hh

FORMS += \
        mainwindow.ui \
    studywindow.ui

SUBDIRS += \
    HaiBeiDanCi.pro

DISTFILES += \
    HaiBeiDanCi.pro.user \
    wordview.html \
    wordlist.txt

RESOURCES += \
    haibeidanci.qrc
