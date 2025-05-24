# Makefile for compiling a single C++ file

# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -Wall -g

# Directories
OBJ_DIR = build
BIN_DIR = bin
SRC_DIR = src

# Targets
TARGET = $(BIN_DIR)/main

# Sources
SRC = $(SRC_DIR)/main.cpp

# Objects
OBJ = $(OBJ_DIR)/main.o

# Default target
all: $(TARGET)

# Link object file to create executable
$(TARGET): $(OBJ)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJ)

# Compile source file into object file
$(OBJ): $(SRC)
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $(SRC) -o $(OBJ)

# Clean up build files
clean:
	rm -f $(OBJ_DIR) $(BIN_DIR)
