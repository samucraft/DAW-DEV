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
ACCEL_SRC = $(SRC_DIR)/accel.cpp
DISP_SRC = $(SRC_DIR)/disp.cpp
THEORY_SRC = $(SRC_DIR)/theory.cpp
CAM_SRC = $(SRC_DIR)/cam.cpp
LED_SRC = $(SRC_DIR)/led.cpp
ANALOG_SRC = $(SRC_DIR)/analog.cpp

# Objects
OBJ = $(OBJ_DIR)/main.o
KEYS_OBJ = $(OBJ_DIR)/keys.o
SIGN_OBJ = $(OBJ_DIR)/signal.o
SOUND_OBJ = $(OBJ_DIR)/sound.o
TOUCH_OBJ = $(OBJ_DIR)/touch.o
ACCEL_OBJ = $(OBJ_DIR)/accel.o
DISP_OBJ = $(OBJ_DIR)/disp.o
THEORY_OBJ = $(OBJ_DIR)/theory.o
CAM_OBJ = $(OBJ_DIR)/cam.o
LED_OBJ = $(OBJ_DIR)/led.o
ANALOG_OBJ = $(OBJ_DIR)/analog.o

CXXFLAGS += -I$(INC_DIR)

# Libraries
WIP_LIB = -lwiringPi
PA_LIB = -lportaudio
SND_LIB = -lsndfile
LED_LIB = -lws2811
LED_LIB_PATH = -L../rpi_ws281x -I../rpi_ws281x
LIBS = $(WIP_LIB) $(PA_LIB) $(SND_LIB) $(LED_LIB)

# Default target
all: $(TARGET)

# Link object files to create executable
$(TARGET): $(OBJ) $(KEYS_OBJ) $(SIGN_OBJ) $(SOUND_OBJ) $(TOUCH_OBJ) $(ACCEL_OBJ) $(DISP_OBJ) $(THEORY_OBJ) $(CAM_OBJ) $(LED_OBJ) $(ANALOG_OBJ)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJ) $(KEYS_OBJ) $(SIGN_OBJ) $(SOUND_OBJ) $(TOUCH_OBJ) $(ACCEL_OBJ) $(DISP_OBJ) $(THEORY_OBJ) $(CAM_OBJ) $(LED_OBJ) $(ANALOG_OBJ) $(LIBS) $(LED_LIB_PATH)

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

# Compile sound module
$(SOUND_OBJ): $(SOUND_SRC)
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $(SOUND_SRC) -o $(SOUND_OBJ)

# Compile touch module
$(TOUCH_OBJ): $(TOUCH_SRC)
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $(TOUCH_SRC) -o $(TOUCH_OBJ)

# Compile accel module
$(ACCEL_OBJ): $(ACCEL_SRC)
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $(ACCEL_SRC) -o $(ACCEL_OBJ)

# Compile disp module
$(DISP_OBJ): $(DISP_SRC)
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $(DISP_SRC) -o $(DISP_OBJ)

# Compile theory module
$(THEORY_OBJ): $(THEORY_SRC)
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $(THEORY_SRC) -o $(THEORY_OBJ)

# Compile camera module
$(CAM_OBJ): $(CAM_SRC)
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $(CAM_SRC) -o $(CAM_OBJ)

# Compile led module
$(LED_OBJ): $(LED_SRC)
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $(LED_SRC) -o $(LED_OBJ) $(LED_LIB_PATH)

# Compile analog potentiometers module
$(ANALOG_OBJ): $(ANALOG_SRC)
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $(ANALOG_SRC) -o $(ANALOG_OBJ)

# Clean up build files
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)
