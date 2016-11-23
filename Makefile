SRC=src
BIN=bin
CXX=mpicxx

GTEST_ROOT=googletest/googletest
GTEST_LIB=googletest/build/googlemock/gtest/libgtest.a

$(BIN)/example.exe: $(SRC)/example.cpp
	-mkdir $(BIN)
	$(CXX) $(CXXFLAGS) -o $(BIN)/example.exe -I$(GTEST_ROOT)/include \
	-Iinclude $(GTEST_LIB) $(SRC)/example.cpp

clean:
	-rm -rf $(BIN)/*
