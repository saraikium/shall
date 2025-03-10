# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c17

# Project structure
SRC_DIR = src
BUILD_DIR = build
TARGET = $(BUILD_DIR)/shall

# Hardcoded source and object files
SRCS = src/main.c src/shall.c
OBJS = build/main.o build/shall.o

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

# Ensure build directory exists
build:
	mkdir -p build

# Compile each file explicitly
build/main.o: src/main.c 
	$(CC) $(CFLAGS) -c $< -o build/main.o

build/shall.o: src/shall.c | build
	$(CC) $(CFLAGS) -c $< -o build/shall.o


# Clean up
clean:
	rm -rf build

.PHONY: all clean build
