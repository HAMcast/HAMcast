QT -= core gui
TEMPLATE = app
TARGET = test_receiver
macx:CONFIG = x86_64 debug
INCLUDEPATH += /opt/local/include/ \
	../../libhamcast/
unix:LIBS += -L/opt/local/lib/ \
	-lboost_serialization-mt \
	-lboost_thread-mt \
	-lboost_system-mt \
	-lboost_filesystem-mt \
	-lboost_regex-mt \
	-L../../libhamcast/ \
	-lhamcast-0.1
QMAKE_CXXFLAGS += -Wno-missing-field-initializers
SOURCES += main.cpp
