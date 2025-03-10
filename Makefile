# Compiler and flags
CC = gcc
CFLAGS = -Wall -Werror -Wextra -std=c17

# Project structure
SRC_DIR = src
BUILD_DIR = build
TARGET = $(BUILD_DIR)/shall

# Automatically find source and object files
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRCS))

# Default target
all: $(TARGET)

# Link the final executable
$(TARGET): $(OBJS) | build
	$(CC) $(CFLAGS) $^ -o $@

# Ensure build directory exists
build:
	mkdir -p $(BUILD_DIR)

# Compile each .c file into a .o file
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | build
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up
clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean build
