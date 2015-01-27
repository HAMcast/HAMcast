#-------------------------------------------------
#
# Project created by QtCreator 2012-06-01T15:47:06
#
#-------------------------------------------------

TARGET = Test-mcpo-hamcast



SOURCES += src/main.cpp \
            src/Receiver.cpp

HEADERS += include/Receiver.h

LIBS    +="-L/home/berg/hamcast/hamcast/modules/mcpo/ariba-0.7.1b/source/ariba/.libs" -lariba \
        -lboost_system \
        "-L/home/berg/hamcast/hamcast/modules/mcpo/mcpo-0.5.1/source/mcpo/.libs/" -lmcpo

INCLUDEPATH+= /home/berg/hamcast/hamcast/modules/mcpo/ariba-0.7.1b/source/  \
     /home/berg/hamcast/hamcast/modules/mcpo/mcpo-0.5.1/source
