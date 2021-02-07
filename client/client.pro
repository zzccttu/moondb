TEMPLATE = app
CONFIG += console c++14
CONFIG -= app_bundle
CONFIG -= qt

INCLUDEPATH += C:\mingw64\lib\gcc\x86_64-w64-mingw32\8.1.0\include
LIBS += -lwsock32 -lws2_32
LIBS += -LC:\mingw64\lib\gcc\x86_64-w64-mingw32\8.1.0 -lquadmath

SOURCES += \
    main.cpp \
    cmoondbclient.cpp

HEADERS += \
    cmoondbclient.h \
    cpack.hpp \
    crunningerror.hpp \
    ctime.hpp \
    cany.hpp \
    definition.hpp \
    functions.hpp \
    crandom.hpp

TARGET = ../../../bin/client
