# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c17

# Project structure
SRC_DIR = src
BUILD_DIR = build
TARGET = $(BUILD_DIR)/shall

# Source and object files
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

# Debug output
$(info SRCS: $(SRCS))
$(info OBJS: $(OBJS))

# Ensure the build directory exists
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Default target
all: $(TARGET)

# Build the executable
$(TARGET): $(OBJS) 
	$(CC) $(CFLAGS) $^ -o $@

# Compile C source files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build files
clean:
	rm -rf $(BUILD_DIR)

# Phony targets
.PHONY: all clean $(BUILD_DIR)
