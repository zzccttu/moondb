#-------------------------------------------------
#
# Project created by QtCreator 2019-04-19T14:52:02
#
#-------------------------------------------------

#QT       -= core gui
#TEMPLATE = lib

TEMPLATE = app
CONFIG += console c++14
CONFIG -= app_bundle
CONFIG -= qt

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += E:\Library\Boost\include\boost-1_73
INCLUDEPATH += C:\mingw64\lib\gcc\x86_64-w64-mingw32\8.1.0\include
INCLUDEPATH += $$PWD

LIBS += -LE:\Library\Boost\lib -lboost_filesystem-mgw8-mt-s-x64-1_73 -lboost_system-mgw8-mt-s-x64-1_73 -lboost_locale-mgw8-mt-s-x64-1_73
LIBS += -LC:\mingw64\lib\gcc\x86_64-w64-mingw32\8.1.0 -lstdc++ -lquadmath
LIBS += -lpthread -lwsock32 -lws2_32
LIBS += -L$$PWD/iconv -liconv
LIBS += -L$$PWD/gmp -lgmp
LIBS += -L$$PWD/sqlite -lsqlite3

SOURCES += \
	main.cpp \
	library/md5.cpp \
	library/sha1.cpp \
	library/base64.cpp \
	src/cdecimal64.cpp \
	src/cdecimal128.cpp \
	src/cmoondb.cpp \
	src/cdatabase.cpp \
	src/csqlite.cpp \
	src/ctable.cpp \
	src/clog.cpp \
	src/cservice.cpp \
	src/csqlparser.cpp

HEADERS += \
	library/md5.h \
	library/sha1.h \
	library/base64.h \
	src/cdecimal64.h \
	src/cdecimal128.h \
	src/cmoondb.h \
	src/cfile.hpp \
	src/cfilesystem.hpp \
	src/cmultimutex.hpp \
	src/csqlite.h \
	src/cstring.hpp \
	src/crandom.hpp \
	src/csystemerror.hpp \
	src/header.h \
	src/ctime.hpp \
	src/cdatabase.h \
	src/functions.hpp \
	src/crunningerror.hpp \
	src/setting.h \
	src/ciconv.hpp \
	src/cpack.hpp \
	src/cmap.hpp \
	src/cfixedmemorystorage.hpp \
	src/definition.hpp \
	src/ctable.h \
	src/cfixedmap.hpp \
	src/cparsexml.hpp \
	src/cany.hpp \
	src/clog.h \
	src/cqueue.hpp \
	src/cservice.h \
	src/csqlparser.h

TARGET = ../../../bin/moondb
