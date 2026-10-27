CXX ?= $(shell which clang++ 2>/dev/null || which g++ 2>/dev/null || echo g++)
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra -Iinclude
BIN_DIR = bin
SRC_DIR = src
TEST_DIR = tests

all: $(BIN_DIR)/sars_engine $(BIN_DIR)/test_suite

engine: $(BIN_DIR)/sars_engine

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(BIN_DIR)/sars_engine: $(SRC_DIR)/main.cpp $(wildcard include/*.hpp) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $(SRC_DIR)/main.cpp

$(BIN_DIR)/test_suite: $(TEST_DIR)/test_suite.cpp $(wildcard include/*.hpp) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $(TEST_DIR)/test_suite.cpp

test: $(BIN_DIR)/test_suite
	./$(BIN_DIR)/test_suite

clean:
	rm -rf $(BIN_DIR)

.PHONY: all engine test clean
