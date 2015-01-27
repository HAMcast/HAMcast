QT -= core gui
TEMPLATE = app
TARGET = test_sender
macx:CONFIG = x86_64 debug
INCLUDEPATH += /opt/local/include/ \
	../../libhamcast/
LIBS += -L/home/zagaria/hamcast/lib/ \
    -lhamcast
unix:LIBS += -L/opt/local/lib/ \
	-lboost_serialization-mt \
	-lboost_thread-mt \
	-lboost_system-mt \
	-lboost_filesystem-mt \
        -lboost_regex-mt
QMAKE_CXXFLAGS += -Wno-missing-field-initializers
SOURCES += main.cpp
