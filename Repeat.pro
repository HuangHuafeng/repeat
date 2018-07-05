#-------------------------------------------------
#
# Project created by QtCreator 2018-07-03T21:46:32
#
#-------------------------------------------------

QT       += core gui xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Repeat
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
#DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


# add maclibs
INCLUDEPATH += $${PWD}/golddict/maclibs/include
LIBS += -L$${PWD}/golddict/maclibs/lib -liconv -llzo2 -lbz2 -lz

# copy libs
#QMAKE_POST_LINK = -mkdir -p Repeat.app/Contents/Frameworks & \
#                      cp -nR $${PWD}/golddict/maclibs/lib/ Repeat.app/Contents/Frameworks/

SOURCES += \
        main.cpp \
        mainwindow.cpp \
    golddict/atomic_rename.cc \
    golddict/audiolink.cc \
    golddict/btreeidx.cc \
    golddict/chunkedstorage.cc \
    golddict/config.cc \
    golddict/decompress.cc \
    golddict/delegate.cc \
    golddict/dictionary.cc \
    golddict/file.cc \
    golddict/filetype.cc \
    golddict/folding.cc \
    golddict/fsencoding.cc \
    golddict/ftshelpers.cc \
    golddict/fulltextsearch.cc \
    golddict/gddebug.cc \
    golddict/htmlescape.cc \
    golddict/instances.cc \
    golddict/langcoder.cc \
    golddict/language.cc \
    golddict/mdictparser.cc \
    golddict/mdx.cc \
    golddict/mutex.cc \
    golddict/ripemd.cc \
    golddict/splitfile.cc \
    golddict/tiff.cc \
    golddict/ufile.cc \
    golddict/utf8.cc \
    golddict/wildcard.cc \
    golddict/wstring.cc \
    golddict/wstring_qt.cc \
    golddict/zipfile.cc \
    loadmdx.cpp \
    golddict/article_maker.cc \
    golddict/wordfinder.cc

HEADERS += \
        mainwindow.h \
    golddict/atomic_rename.hh \
    golddict/audiolink.hh \
    golddict/btreeidx.hh \
    golddict/chunkedstorage.hh \
    golddict/config.hh \
    golddict/decompress.hh \
    golddict/delegate.hh \
    golddict/dictionary.hh \
    golddict/file.hh \
    golddict/filetype.hh \
    golddict/folding.hh \
    golddict/fsencoding.hh \
    golddict/ftshelpers.hh \
    golddict/fulltextsearch.hh \
    golddict/gddebug.hh \
    golddict/htmlescape.hh \
    golddict/instances.hh \
    golddict/langcoder.hh \
    golddict/language.hh \
    golddict/mdictparser.hh \
    golddict/mdx.hh \
    golddict/mutex.hh \
    golddict/ripemd.hh \
    golddict/splitfile.hh \
    golddict/tiff.hh \
    golddict/ufile.hh \
    golddict/utf8.hh \
    golddict/wildcard.hh \
    golddict/wstring.hh \
    golddict/wstring_qt.hh \
    golddict/zipfile.hh \
    golddict/article_maker.hh \
    golddict/wordfinder.hh

FORMS += \
        mainwindow.ui
