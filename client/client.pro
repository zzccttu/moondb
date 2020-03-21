TEMPLATE = app
CONFIG += console c++14
CONFIG -= app_bundle
CONFIG -= qt

LIBS += -lwsock32 -lws2_32

SOURCES += \
        main.cpp \
    cmoondbclient.cpp

TARGET = ../../../bin/client

HEADERS += \
    cmoondbclient.h \
    cpack.hpp \
    crunningerror.hpp \
    ctime.hpp \
    cany.hpp \
    definition.hpp \
    functions.hpp \
    crandom.hpp
