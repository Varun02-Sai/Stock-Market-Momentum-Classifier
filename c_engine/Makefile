# ============================================================================
# Makefile — Build System for the C Algorithm Engine
# Project:  Stock Market Momentum Classifier
#
# Usage:
#   make            — Build both debug and release binaries
#   make debug      — Build with debug symbols (-g) and no optimization (-O0)
#   make release    — Build with optimizations (-O2) and static linking
#   make clean      — Remove all compiled files
#
# Compiler Flags:
#   -Wall           — Enable all standard warnings
#   -Wextra         — Enable extra warnings
#   -pthread        — Enable POSIX threads support
#   -Iinclude       — Add include/ to the header search path
#   -static         — Link all libraries statically (no DLL dependencies)
#   -lm             — Link the math library (for exp(), sqrt())
#
# Topics: Makefile, Build Automation, Modular Compilation
# ============================================================================

# Compiler and base flags
CC = gcc
CFLAGS = -Wall -Wextra -pthread -Iinclude

# Directory layout
SRC_DIR = src
INC_DIR = include
OBJ_DIR = obj

# Automatically find all .c source files and map to .o object files
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

# Output binary names
TARGET_DEBUG = model_debug
TARGET_RELEASE = model_release

# Default target: build both
all: debug release

# Debug build: includes debug symbols, no optimization
# Use this for debugging with gdb or valgrind
debug: CFLAGS += -g -O0
debug: $(TARGET_DEBUG)

# Release build: optimized, statically linked
# Static linking ensures the binary runs without needing MinGW DLLs
release: CFLAGS += -O2
release: $(TARGET_RELEASE)

# Link debug binary
$(TARGET_DEBUG): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ -lm

# Link release binary
$(TARGET_RELEASE): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ -lm

# Compile individual .c files into .o object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Create the obj/ directory if it doesn't exist
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Clean up all build artifacts
clean:
	rm -rf $(OBJ_DIR) $(TARGET_DEBUG)* $(TARGET_RELEASE)*

# Declare targets that are not files
.PHONY: all debug release clean
