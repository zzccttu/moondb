CXX = g++
INCL = -I /usr/local/include/boost -I gmp -I iconv
LIBS = /usr/local/lib/libgmp.a /usr/local/lib/libboost_filesystem.a /usr/local/lib/libboost_system.a /usr/local/lib/libboost_locale.a
#CXX_FLAGS = -pipe -fno-bounds-check -freg-struct-return -finline-limit=20000 -ftracer -s -minline-all-stringops -fexceptions -O3 -m128bit-long-double -std=c++14 -m64 -pthread
CXX_FLAGS = -pipe -fexceptions -O3 -m128bit-long-double -std=c++14 -m64 -pthread
BIN = ../bin/moondb
BUILD_DIR = ../obj/Release/moondb

all:
	mkdir -p $(BUILD_DIR)/src
	mkdir -p $(BUILD_DIR)/library
	$(CXX) $(CXX_FLAGS) $(INCL) -c library/base64.cpp -o $(BUILD_DIR)/library/base64.o
	$(CXX) $(CXX_FLAGS) $(INCL) -c library/BigDecimal.cpp -o $(BUILD_DIR)/library/BigDecimal.o
	$(CXX) $(CXX_FLAGS) $(INCL) -c library/md5.cpp -o $(BUILD_DIR)/library/md5.o
	$(CXX) $(CXX_FLAGS) $(INCL) -c library/sha1.cpp -o $(BUILD_DIR)/library/sha1.o
	$(CXX) $(CXX_FLAGS) $(INCL) -c src/cdatabase.cpp -o $(BUILD_DIR)/src/cdatabase.o
	$(CXX) $(CXX_FLAGS) $(INCL) -c src/clog.cpp -o $(BUILD_DIR)/src/clog.o
	$(CXX) $(CXX_FLAGS) $(INCL) -c src/cmoondb.cpp -o $(BUILD_DIR)/src/cmoondb.o
	$(CXX) $(CXX_FLAGS) $(INCL) -c src/ctable.cpp -o $(BUILD_DIR)/src/ctable.o
	$(CXX) $(CXX_FLAGS) $(INCL) -c src/cservice.cpp -o $(BUILD_DIR)/src/cservice.o
	$(CXX) $(CXX_FLAGS) $(INCL) -c main.cpp -o $(BUILD_DIR)/main.o
	$(CXX) -o $(BIN) $(BUILD_DIR)/src/cdatabase.o $(BUILD_DIR)/src/clog.o $(BUILD_DIR)/src/cmoondb.o $(BUILD_DIR)/src/ctable.o $(BUILD_DIR)/src/cservice.o $(BUILD_DIR)/library/base64.o $(BUILD_DIR)/library/BigDecimal.o $(BUILD_DIR)/library/md5.o $(BUILD_DIR)/library/sha1.o $(BUILD_DIR)/main.o $(CXX_FLAGS) $(LIBS)
