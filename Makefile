SRC=src
INCLUDE=include
BIN=bin
CXX=mpicxx

GTEST_ROOT=googletest/googletest
GTEST_LIB_DIR=googletest/build/googlemock/gtest
GTEST_LIB=googletest/build/googlemock/gtest/libgtest.a

.PHONY: all clean

all: $(BIN)/example.exe

$(BIN)/example.exe: $(BIN)/example.o
	$(CXX) $(CXXFLAGS) -o $(BIN)/example.exe $(BIN)/example.o -L$(GTEST_LIB_DIR) -lgtest

$(BIN)/example.o: $(SRC)/example.cpp $(INCLUDE)/gtest-mpi-listener.hpp
	-mkdir $(BIN)
	$(CXX) $(CXXFLAGS) -c -o $(BIN)/example.o -I$(GTEST_ROOT)/include \
	-Iinclude $(SRC)/example.cpp


clean:
	-rm -rf $(BIN)/*
