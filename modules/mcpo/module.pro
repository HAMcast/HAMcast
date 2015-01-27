#-------------------------------------------------
#
# Project created by QtCreator 2012-05-02T11:13:15
#
#-------------------------------------------------




TARGET = module

LIBS+=  -L/home/berg/hamcast/hamcast/lib/ \
        "-L/home/berg/ariba/ariba-0.7.1/build/lib/" -lariba \
        -lboost_system \
         "-L/home/berg/hamcast/hamcast/modules/mcpo/mcpo-0.5.1/source/mcpo/.libs" -libmcpo

INCLUDEPATH += /opt/local/include/ \
    /home/berg/hamcast/hamcast/libhamcast \
    /home/berg/ariba/ariba-0.7.1/build/include/ \
    /home/berg/hamcast/hamcast/modules/mcpo/mcpo-0.5.1/source


CONFIG   += console

SOURCES += \
    src/mcpomodule.cpp \
    src/mcpoinstance.cpp \
    src/byte_message.cpp

HEADERS += \
    include/mcpomodule.h \
    include/mcpoinstance.h \
    include/byte_message.h

OTHER_FILES += \
    configexample.txt \
    CMakeLists.txt \
    src/configexample.txt
