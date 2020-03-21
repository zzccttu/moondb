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

# INCLUDEPATH += C:\Qt\5.12.2\mingw73_64\include
INCLUDEPATH += C:\Boost\include\boost-1_70
INCLUDEPATH += $$PWD/gmp
INCLUDEPATH += $$PWD/iconv

LIBS += -LC:\Boost\lib -lboost_filesystem-mgw73-mt-s-x64-1_70 -lboost_system-mgw73-mt-s-x64-1_70 -lboost_locale-mgw73-mt-s-x64-1_70
LIBS += -lpthread -lwsock32 -lws2_32
LIBS += -L$$PWD/gmp -lgmp
LIBS += -L$$PWD/iconv -liconv

SOURCES += \
	main.cpp \
	src/cmoondb.cpp \
	src/cdatabase.cpp \
	src/ctable.cpp \
	src/clog.cpp \
    library/md5.cpp \
    library/sha1.cpp \
    library/base64.cpp \
	library/BigDecimal.cpp \
    src/cservice.cpp

HEADERS += \
	src/cmoondb.h \
	src/cfile.hpp \
	src/cfilesystem.hpp \
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
	library/md5.h \
	library/sha1.h \
	library/base64.h \
	library/BigDecimal.h \
    src/cservice.h

TARGET = ../../../bin/moondb
