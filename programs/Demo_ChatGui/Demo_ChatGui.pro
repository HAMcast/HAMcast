#-------------------------------------------------
#
# Project created by QtCreator 2011-07-04T14:17:09
#
#-------------------------------------------------

QT       += core gui

TARGET = Demo_ChatGui
TEMPLATE = app
INCLUDEPATH += .
INCLUDEPATH += codec
INCLUDEPATH += rtp
SUBDIRS += rtp
SUBDIRS += codec

message($$PWD)
SOURCES += main.cpp\
        mainwindow.cpp \
    chat.cpp \
    chatthreads.cpp \
    questionbox.cpp \
    vlcStreamer/vlc_on_qt.cpp \
    vlcStreamer/udpreciever.cpp

HEADERS  += mainwindow.h \
    chat.h \
    chatthreads.h \
    questionbox.h \
    vlcStreamer/vlc_on_qt.h \
    vlcStreamer/udpreciever.h

FORMS    += mainwindow.ui \
    questionbox.ui

LIBS += $$PWD/VideoStream/codec/libDAVC.a
LIBS += $$PWD/VideoStream/rtp/libDAVC_rtp.a

LIBS += -L../../lib/ \
    -lhamcast
INCLUDEPATH += /home/zagria/hamcast/libhamcast/hamcast/ /opt/local/include

LIBS += -L/usr/lib \
    -L/opt/local/lib \
    -lboost_thread-mt \
    -lboost_system-mt \
    -lboost_filesystem-mt \
    -lboost_regex-mt \
    -lboost_date_time-mt

QMAKE_CXXFLAGS += -D__STDC_CONSTANT_MACROS

#LIBS += -L /usr/lib/libavcodec.so
#LIBS+= -L /usr/lib/libavformat.so
#LIBS+= -L /usr/lib/libswscale.so
#LIBS+= -L /usr/lib/libSDL.so

LIBS += -lvlc
INCLUDEPATH+=/usr/include/vlc/

RESOURCES += \
    pics/pics.qrc
