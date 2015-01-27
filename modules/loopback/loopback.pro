TEMPLATE = lib
TARGET = loopback
QMAKE_CXXFLAGS += -Wno-missing-field-initializers
QT -= core \
    gui
macx:CONFIG = x86_64 \
    debug
INCLUDEPATH += /opt/local/include/ \
    ../../libhamcast
unix:LIBS += -L/opt/local/lib/ \
    -lboost_serialization-mt \
    -lboost_thread-mt \
    -lboost_system-mt \
    -lboost_date_time-mt \
    -lboost_filesystem-mt \
    -lboost_regex-mt \
    -L../../libhamcast/ \
    -lhamcast-0.4 \
    -ldl
SOURCES += loopback.cpp
