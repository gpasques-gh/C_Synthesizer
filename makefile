# Compiler
CC = gcc

# Project name (binary name)
TARGET = synth

# Directories
SRC_DIR = src
INC_DIR = include
BIN_DIR = bin
OBJ_DIR = obj

# Files
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

# Flags
CFLAGS = -Wall -Wextra -O2 -I$(INC_DIR) $(shell pkg-config --cflags sdl2 SDL2_ttf)
LDFLAGS = -lasound -lncurses -lm $(shell pkg-config --libs sdl2 SDL2_ttf)

# Default rule
all: $(BIN_DIR)/$(TARGET)

# Link
$(BIN_DIR)/$(TARGET): $(OBJS) | $(BIN_DIR)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

# Compile
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Create directories if needed
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Clean
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)/$(TARGET)

# Rebuild
re: clean all

run:
	./$(BIN_DIR)/$(TARGET)

.PHONY: all clean re
