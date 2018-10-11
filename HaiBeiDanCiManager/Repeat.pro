#-------------------------------------------------
#
# Project created by QtCreator 2018-07-03T21:46:32
#
#-------------------------------------------------

QT       += core gui xml network webenginewidgets multimedia sql concurrent

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
INCLUDEPATH += $${PWD}/../golddict/maclibs/include
LIBS += -L$${PWD}/../golddict/maclibs/lib -liconv -llzo2 -lbz2 -lz

# copy libs
#QMAKE_POST_LINK = -mkdir -p Repeat.app/Contents/Frameworks & \
#                      cp -nR $${PWD}/../golddict/maclibs/lib/ Repeat.app/Contents/Frameworks/

SOURCES += \
    ../golddict/atomic_rename.cc \
    ../golddict/audiolink.cc \
    ../golddict/btreeidx.cc \
    ../golddict/chunkedstorage.cc \
    ../golddict/config.cc \
    ../golddict/decompress.cc \
    ../golddict/delegate.cc \
    ../golddict/dictionary.cc \
    ../golddict/file.cc \
    ../golddict/filetype.cc \
    ../golddict/folding.cc \
    ../golddict/fsencoding.cc \
    ../golddict/ftshelpers.cc \
    ../golddict/fulltextsearch.cc \
    ../golddict/gddebug.cc \
    ../golddict/htmlescape.cc \
    ../golddict/instances.cc \
    ../golddict/langcoder.cc \
    ../golddict/language.cc \
    ../golddict/mdictparser.cc \
    ../golddict/mdx.cc \
    ../golddict/mutex.cc \
    ../golddict/ripemd.cc \
    ../golddict/splitfile.cc \
    ../golddict/tiff.cc \
    ../golddict/ufile.cc \
    ../golddict/utf8.cc \
    ../golddict/wildcard.cc \
    ../golddict/wstring.cc \
    ../golddict/wstring_qt.cc \
    ../golddict/zipfile.cc \
    ../golddict/article_maker.cc \
    ../golddict/wordfinder.cc \
    ../golddict/article_netmgr.cc \
    ../HaiBeiDanCi/temporaryfilemanager.cpp \
    ../HaiBeiDanCi/mediaplayer.cpp \
    ../HaiBeiDanCi/wordview.cpp \
    ../HaiBeiDanCi/worddb.cpp \
    ../HaiBeiDanCi/word.cpp \
    ../HaiBeiDanCi/memoryitem.cpp \
    ../HaiBeiDanCi/wordcard.cpp \
    ../HaiBeiDanCi/wordbook.cpp \
    ../HaiBeiDanCi/mysettings.cpp \
    ../HaiBeiDanCi/downloadmanager.cpp \
    ../HaiBeiDanCi/serverclientprotocol.cpp \
    ../HaiBeiDanCi/svragt.cpp \
    ../HaiBeiDanCi/helpfunc.cpp \
    ../HaiBeiDanCi/mediafilemanager.cpp \
    ../HaiBeiDanCi/applicationuser.cpp \
    ../HaiBeiDanCi/clienttoken.cpp \
    ../HaiBeiDanCi/token.cpp \
    ../HaiBeiDanCi/logindialog.cpp \
    ../HaiBeiDanCi/serveruseragent.cpp \
    ../HaiBeiDanCi/applicationversion.cpp \
    main.cpp \
    mainwindow.cpp \
    mdxdict.cpp \
    dictschemehandler.cpp \
    gdhelper.cpp \
    newbook.cpp \
    servermanager.cpp \
    manageragent.cpp \
    preferencesdialog.cpp \
    releaseappdialog.cpp \
    releaseupgraderdialog.cpp \
    ../HaiBeiDanCi/servercommunicator.cpp \
    ../HaiBeiDanCi/bookdownloader.cpp

HEADERS += \
    ../golddict/atomic_rename.hh \
    ../golddict/audiolink.hh \
    ../golddict/btreeidx.hh \
    ../golddict/chunkedstorage.hh \
    ../golddict/config.hh \
    ../golddict/decompress.hh \
    ../golddict/delegate.hh \
    ../golddict/dictionary.hh \
    ../golddict/file.hh \
    ../golddict/filetype.hh \
    ../golddict/folding.hh \
    ../golddict/fsencoding.hh \
    ../golddict/ftshelpers.hh \
    ../golddict/fulltextsearch.hh \
    ../golddict/gddebug.hh \
    ../golddict/htmlescape.hh \
    ../golddict/instances.hh \
    ../golddict/langcoder.hh \
    ../golddict/language.hh \
    ../golddict/mdictparser.hh \
    ../golddict/mdx.hh \
    ../golddict/mutex.hh \
    ../golddict/ripemd.hh \
    ../golddict/splitfile.hh \
    ../golddict/tiff.hh \
    ../golddict/ufile.hh \
    ../golddict/utf8.hh \
    ../golddict/wildcard.hh \
    ../golddict/wstring.hh \
    ../golddict/wstring_qt.hh \
    ../golddict/zipfile.hh \
    ../golddict/article_maker.hh \
    ../golddict/wordfinder.hh \
    ../golddict/article_netmgr.hh \
    ../golddict/sptr.hh \
    ../HaiBeiDanCi/temporaryfilemanager.h \
    ../HaiBeiDanCi/mediaplayer.h \
    ../HaiBeiDanCi/wordview.h \
    ../HaiBeiDanCi/worddb.h \
    ../HaiBeiDanCi/word.h \
    ../HaiBeiDanCi/memoryitem.h \
    ../HaiBeiDanCi/wordcard.h \
    ../HaiBeiDanCi/wordbook.h \
    ../HaiBeiDanCi/mysettings.h \
    ../HaiBeiDanCi/downloadmanager.h \
    ../HaiBeiDanCi/serverclientprotocol.h \
    ../HaiBeiDanCi/svragt.h \
    ../HaiBeiDanCi/helpfunc.h \
    ../HaiBeiDanCi/mediafilemanager.h \
    ../HaiBeiDanCi/applicationuser.h \
    ../HaiBeiDanCi/clienttoken.h \
    ../HaiBeiDanCi/token.h \
    ../HaiBeiDanCi/logindialog.h \
    ../HaiBeiDanCi/serveruseragent.h \
    ../HaiBeiDanCi/applicationversion.h \
    mainwindow.h \
    mdxdict.h \
    dictschemehandler.h \
    gdhelper.h \
    newbook.h \
    servermanager.h \
    manageragent.h \
    preferencesdialog.h \
    releaseappdialog.h \
    releaseupgraderdialog.h \
    ../HaiBeiDanCi/servercommunicator.h \
    ../HaiBeiDanCi/bookdownloader.h

FORMS += \
    ../HaiBeiDanCi/logindialog.ui \
    mainwindow.ui \
    newbook.ui \
    preferencesdialog.ui \
    releaseappdialog.ui \
    releaseupgraderdialog.ui

RESOURCES += \
    res.qrc

TRANSLATIONS = myapp_zh_CN.ts
