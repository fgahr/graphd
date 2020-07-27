.PHONY: test clean

PROGNAME = graphd

CXX = clang++
CXXFLAGS = -std=c++17 -Iinclude -Wall -Wextra -Wpedantic
OPT = -O2
TESTLIBS = -lgtest -lgtest_main

SRC = src
INC = include
BIN = bin
OBJ = obj
TSRC = test/src
TBIN = test/bin

all: $(BIN)/$(PROGNAME)

vpath %.cpp $(SRC)
vpath %.hpp $(INC)/graphd:$(INC)/graphd/input:$(INC)/graphd/input/parser

# TODO: Generate from list of source files
ALLOBJS = $(OBJ)/parse.o $(OBJ)/reduce.o $(OBJ)/expr.o $(OBJ)/token.o $(OBJ)/graph.o

$(BIN)/$(PROGNAME): main.cpp $(ALLOBJS) | $(BIN)
	$(CXX) $(CXXFLAGS) $(OPT) $^ -o $@

$(OBJ)/%.o: %.cpp %.hpp | $(OBJ)
	$(CXX) -c $(CXXFLAGS) $(OPT) $< -o $@

test: token_test parse_test graph_test

clean:
	rm -f $(BIN)/* $(OBJ)/* $(TBIN)/*

%_test: $(TBIN)/%_test
	$<

$(TBIN)/%_test: $(TSRC)/%_test.cpp $(ALLOBJS) | $(TBIN)
	$(CXX) $(CXXFLAGS) $(TESTLIBS) $^ -o $@

$(BIN) $(TBIN) $(OBJ):
	mkdir -p $@
