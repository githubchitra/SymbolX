# Makefile for Mini Compiler Front-End

# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Wpedantic -O2
INCLUDES = -Iinclude
LDFLAGS = 

# Directories
SRC_DIR = src
INC_DIR = include
OBJ_DIR = obj
BIN_DIR = bin

# Source files
SOURCES = $(SRC_DIR)/main.cpp \
          $(SRC_DIR)/token.cpp \
          $(SRC_DIR)/lexer.cpp \
          $(SRC_DIR)/ast_node.cpp \
          $(SRC_DIR)/symbol_table.cpp \
          $(SRC_DIR)/parser.cpp \
          $(SRC_DIR)/semantic_analyzer.cpp \
          $(SRC_DIR)/compiler.cpp \
          $(SRC_DIR)/cfg_grammar.cpp \
          $(SRC_DIR)/first_follow.cpp \
          $(SRC_DIR)/ll1_parser_table.cpp \
          $(SRC_DIR)/dynamic_ll1_parser.cpp \
          $(SRC_DIR)/grammar_interface.cpp \
          $(SRC_DIR)/ast_visualizer.cpp

# Object files
OBJECTS = $(SOURCES:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

# Executable
TARGET = $(BIN_DIR)/mini_compiler

# Default target
all: directories $(TARGET)

# Create necessary directories
directories:
	@if not exist $(OBJ_DIR) mkdir $(OBJ_DIR)
	@if not exist $(BIN_DIR) mkdir $(BIN_DIR)

# Link object files to create executable
$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) $(LDFLAGS)
	@echo "Build successful! Executable: $(TARGET)"

# Compile source files to object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Clean build files
clean:
	@if exist $(OBJ_DIR) rmdir /s /q $(OBJ_DIR)
	@if exist $(BIN_DIR) rmdir /s /q $(BIN_DIR)
	@echo "Clean complete!"

# Run the program
run: all
	$(TARGET)

# Test the compiler
test: all
	$(TARGET) test

# Test error detection
test-errors: all
	$(TARGET) error

# Run demonstration
demo: all
	$(TARGET) demo

# Rebuild everything
rebuild: clean all

# Debug build
debug: CXXFLAGS += -g -DDEBUG
debug: all

# Install (copy to system path)
install: all
	@echo "Installing mini_compiler to /usr/local/bin..."
	@copy $(TARGET) C:\Windows\System32\

# Uninstall
uninstall:
	@echo "Removing mini_compiler from system path..."
	@del C:\Windows\System32\mini_compiler.exe

# Help
help:
	@echo "Available targets:"
	@echo "  all         - Build the project"
	@echo "  clean       - Remove build files"
	@echo "  run         - Build and run the program"
	@echo "  test        - Build and run tests"
	@echo "  test-errors - Build and run error detection tests"
	@echo "  demo        - Build and run demonstration"
	@echo "  rebuild     - Clean and rebuild"
	@echo "  debug       - Build with debug symbols"
	@echo "  install     - Install to system path"
	@echo "  uninstall   - Remove from system path"
	@echo "  help        - Show this help message"

# Check for required tools
check-tools:
	@echo "Checking for required tools..."
	@where g++ >nul 2>&1 || (echo "Error: g++ not found. Please install MinGW or similar." && exit 1)
	@echo "All required tools found."

# Static analysis (if available)
analyze:
	@echo "Running static analysis..."
	@if exist "cppcheck.exe" cppcheck --enable=all $(SRC_DIR) || echo "cppcheck not found, skipping analysis"

# Generate documentation (if Doxygen is available)
docs:
	@echo "Generating documentation..."
	@if exist "doxygen.exe" doxygen Doxyfile || echo "Doxygen not found, skipping documentation generation"

.PHONY: all clean run test test-errors demo rebuild debug install uninstall help check-tools analyze docs
