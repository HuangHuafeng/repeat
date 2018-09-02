QT -= gui
QT += network sql

CONFIG += c++11 console
CONFIG -= app_bundle

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# tells that this is the server application
DEFINES += HAIBEIDANCI_SERVER

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        main.cpp \
    clientwaiter.cpp \
    hbdcserver.cpp \
    ../HaiBeiDanCi/word.cpp \
    ../HaiBeiDanCi/wordbook.cpp \
    ../HaiBeiDanCi/worddb.cpp \
    ../HaiBeiDanCi/mysettings.cpp \
    ../HaiBeiDanCi/downloadmanager.cpp \
    ../HaiBeiDanCi/wordcard.cpp \
    ../HaiBeiDanCi/memoryitem.cpp \
    ../HaiBeiDanCi/serverclientprotocol.cpp \
    clienthandler.cpp \
    hbdcapphandler.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    clientwaiter.h \
    hbdcserver.h \
    ../HaiBeiDanCi/word.h \
    ../HaiBeiDanCi/wordbook.h \
    ../HaiBeiDanCi/worddb.h \
    ../HaiBeiDanCi/mysettings.h \
    ../HaiBeiDanCi/downloadmanager.h \
    ../HaiBeiDanCi/wordcard.h \
    ../HaiBeiDanCi/memoryitem.h \
    ../golddict/sptr.hh \
    clienthandler.h \
    hbdcapphandler.h

DISTFILES += \
    idea.txt
