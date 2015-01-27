QT -= core gui
TEMPLATE = app
TARGET = test_sender
LIBS += -L/usr/local/lib/ \
    -lhamcast-0.2
INCLUDEPATH += /usr/local/include/hamcast-0.2/
LIBS += -L/usr/lib/ \
    -lboost_serialization-mt \
    -lboost_thread-mt \
    -lboost_system-mt \
    -lboost_filesystem-mt \
    -lboost_regex-mt
