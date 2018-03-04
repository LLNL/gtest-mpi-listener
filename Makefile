###############################################################################
#
# Copyright (c) 2016-2018, Lawrence Livermore National Security, LLC.
# Produced at the Lawrence Livermore National Laboratory
# Written by Geoffrey M. Oxberry <oxberry1@llnl.gov>
# LLNL-CODE-739313
# All rights reserved.
#
# This file is part of gtest-mpi-listener. For details, see
# https://github.com/LLNL/gtest-mpi-listener
#
# Please also see the LICENSE and COPYRIGHT files for the BSD-3
# license and copyright information.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#
# * Redistributions of source code must retain the above copyright
#   notice, this list of conditions and the disclaimer below.
#
# * Redistributions in binary form must reproduce the above copyright
#   notice, this list of conditions and the disclaimer (as noted below)
#   in the documentation and/or other materials provided with the
#   distribution.
#
# * Neither the name of the LLNS/LLNL nor the names of its contributors
#   may be used to endorse or promote products derived from this
#   software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL LAWRENCE
# LIVERMORE NATIONAL SECURITY, LLC, THE U.S. DEPARTMENT OF ENERGY OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
# PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
# LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
# NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
################################################################################

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
