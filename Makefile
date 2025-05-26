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
SIGN_SRC = $(SRC_DIR)/signal.cpp
SOUND_SRC = $(SRC_DIR)/sound.cpp
TOUCH_SRC = $(SRC_DIR)/touch.cpp

# Objects
OBJ = $(OBJ_DIR)/main.o
KEYS_OBJ = $(OBJ_DIR)/keys.o
SIGN_OBJ = $(OBJ_DIR)/signal.o
SOUND_OBJ = $(OBJ_DIR)/sound.o
TOUCH_OBJ = $(OBJ_DIR)/touch.o

CXXFLAGS += -I$(INC_DIR)

# Libraries
WIP_LIB = -lwiringPi
PA_LIB = -lportaudio
SND_LIB = -lsndfile
LIBS = $(WIP_LIB) $(PA_LIB) $(SND_LIB)

# Default target
all: $(TARGET)

# Link object file to create executable
$(TARGET): $(OBJ) $(KEYS_OBJ) $(SIGN_OBJ) $(SOUND_OBJ) $(TOUCH_OBJ)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJ) $(KEYS_OBJ) $(SIGN_OBJ) $(SOUND_OBJ) $(TOUCH_OBJ) $(LIBS)

# Compile main file into object file
$(OBJ): $(SRC)
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $(SRC) -o $(OBJ)

# Compile keys module
$(KEYS_OBJ): $(KEYS_SRC)
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $(KEYS_SRC) -o $(KEYS_OBJ)

# Compile signal module
$(SIGN_OBJ): $(SIGN_SRC)
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $(SIGN_SRC) -o $(SIGN_OBJ)

# Compile signal module
$(SOUND_OBJ): $(SOUND_SRC)
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $(SOUND_SRC) -o $(SOUND_OBJ)

# Compile signal module
$(TOUCH_OBJ): $(TOUCH_SRC)
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $(TOUCH_SRC) -o $(TOUCH_OBJ)

# Clean up build files
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)
