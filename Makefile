.PHONY: test clean

CXX = clang++
CXXFLAGS = -std=c++17 -Iinclude -Wall -Wextra
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

test: toktest

clean:
	rm -f $(OBJ)/* $(TBIN)/*

toktest: $(TBIN)/tokenizer_test
	$<

$(TBIN)/tokenizer_test: $(TSRC)/tokenizer_test.cpp $(OBJ)/token.o | $(TBIN)
	$(CXX) $(CXXFLAGS) $(TESTLIBS) $^ -o $@


# Create directories

$(TBIN):
	mkdir -p $@

$(OBJ):
	mkdir -p $@
