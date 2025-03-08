# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -std=c11

# Project structure
SRC_DIR = src
BUILD_DIR = build
TARGET = $(BUILD_DIR)/shall

# Source and object files
SRCS = $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(SRCS:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

# Create build directory if it doesn't exist
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Default target
all: $(BUILD_DIR) $(TARGET)

# Build the executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

# Compile C++ source files as C
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build files
clean:
	rm -rf $(BUILD_DIR)

# Phony targets
.PHONY: all clean
