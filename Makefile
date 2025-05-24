# Makefile for compiling a single C++ file

# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -Wall -g

# Directories
OBJ_DIR = build
BIN_DIR = bin
SRC_DIR = src
INC_DIR = include

# Targets
TARGET = $(BIN_DIR)/main

# Sources
SRC = $(SRC_DIR)/main.cpp
KEYS_SRC = $(SRC_DIR)/keys.cpp

# Objects
OBJ = $(OBJ_DIR)/main.o
KEYS_OBJ = $(OBJ_DIR)/keys.o

CXXFLAGS += -I$(INC_DIR)

# Default target
all: $(TARGET)

# Link object file to create executable
$(TARGET): $(OBJ) $(KEYS_OBJ)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJ) $(KEYS_OBJ)

# Compile main file into object file
$(OBJ): $(SRC)
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $(SRC) -o $(OBJ)

# Compile keys module
$(KEYS_OBJ): $(KEYS_SRC)
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $(KEYS_SRC) -o $(KEYS_OBJ)

# Clean up build files
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)
