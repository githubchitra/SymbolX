# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -g -I./include
LDFLAGS = 

# Directories
SRC_DIR = src
INC_DIR = include
OBJ_DIR = obj
BIN_DIR = bin

# Source files
SOURCES = $(SRC_DIR)/main.c \
          $(SRC_DIR)/symbol_table.c \
          $(SRC_DIR)/hash_table.c \
          $(SRC_DIR)/scope.c \
          $(SRC_DIR)/data_types.c \
          $(SRC_DIR)/parse_tree.c \
          $(SRC_DIR)/error_detection.c

# Object files
OBJECTS = $(SOURCES:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

# Executable
TARGET = $(BIN_DIR)/symbol_table

# Default target
all: directories $(TARGET)

# Create necessary directories
directories:
	@if not exist $(OBJ_DIR) mkdir $(OBJ_DIR)
	@if not exist $(BIN_DIR) mkdir $(BIN_DIR)

# Link object files to create executable
$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) $(LDFLAGS)
	@echo "Build successful! Executable: $(TARGET)"

# Compile source files to object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build files
clean:
	@if exist $(OBJ_DIR) rmdir /s /q $(OBJ_DIR)
	@if exist $(BIN_DIR) rmdir /s /q $(BIN_DIR)
	@echo "Clean complete!"

# Run the program
run: all
	./$(TARGET)

# Rebuild everything
rebuild: clean all

# Test compilation
test: all
	@echo "Running basic tests..."
	./$(TARGET) --test

# Help target
help:
	@echo "Available targets:"
	@echo "  make          - Build the project"
	@echo "  make run      - Build and run the project"
	@echo "  make clean    - Remove build files"
	@echo "  make rebuild  - Clean and rebuild"
	@echo "  make test     - Build and run tests"
	@echo "  make help     - Show this help message"

.PHONY: all clean run rebuild directories help test
