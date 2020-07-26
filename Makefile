.PHONY: test clean

CXX = clang++
CXXFLAGS = -std=c++17 -Iinclude -Wall -Wextra -Wpedantic
OPT = -O2
TESTLIBS = -lgtest -lgtest_main

SRC = src
BIN = bin
OBJ = bin/obj
TSRC = test/src
TBIN = test/bin
GINC = include/graphd

$(OBJ)/%.o: $(SRC)/%.cpp | $(OBJ)
	$(CXX) -c $(CXXFLAGS) $(OPT) $^ -o $@

test: toktest partest

clean:
	rm -f $(OBJ)/* $(TBIN)/*

toktest: $(TBIN)/token_test
	$<

partest: $(TBIN)/parse_test
	$<

$(TBIN)/token_test: $(TSRC)/token_test.cpp $(OBJ)/token.o | $(TBIN)
	$(CXX) $(CXXFLAGS) $(TESTLIBS) $^ -o $@

$(TBIN)/parse_test: $(TSRC)/parse_test.cpp $(OBJ)/parse.o $(OBJ)/reduce.o $(OBJ)/expr.o $(OBJ)/token.o | $(TBIN)
	$(CXX) $(CXXFLAGS) $(TESTLIBS) $^ -o $@

# Create directories

$(TBIN):
	mkdir -p $@

$(OBJ):
	mkdir -p $@
