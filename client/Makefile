CXX = g++
CXX_FLAGS = -pipe -fno-bounds-check -freg-struct-return -finline-limit=20000 -ftracer -s -minline-all-stringops -fexceptions -O3 -m128bit-long-double -std=c++14 -m64 -pthread
#CXX_FLAGS = -pipe -fexceptions -O3 -m128bit-long-double -std=c++14 -m64 -pthread
BIN = ../bin/client
BUILD_DIR = ../obj/Release/client

all:
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CXX_FLAGS) $(INCL) -c cmoondbclient.cpp -o $(BUILD_DIR)/cmoondbclient.o
	$(CXX) $(CXX_FLAGS) $(INCL) -c main.cpp -o $(BUILD_DIR)/main.o
	$(CXX) -o $(BIN) $(BUILD_DIR)/cmoondbclient.o $(BUILD_DIR)/main.o $(CXX_FLAGS)
