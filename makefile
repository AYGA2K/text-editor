CXX = g++
CXXFLAGS = -Wall -Wextra -O2
SRC = $(wildcard src/*.cpp)
OUT = build/main

# Default target
all: build

# Build the project 
build:
	mkdir -p build
	$(CXX) $(CXXFLAGS) -o $(OUT) $(SRC)

# Run program
run: build
	./$(OUT)

#
run-file:build
	./$(OUT) ./test.txt

# Clean
clean:
	rm -rf build

.PHONY: all build run clean
