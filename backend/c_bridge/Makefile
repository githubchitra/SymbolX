# Makefile for C bridge library

CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -fPIC -O2
INCLUDES = -I../../include
LDFLAGS = -shared

# Source files
BRIDGE_SRC = bridge.c
SYMBOL_TABLE_SRC = ../../src/symbol_table.c ../../src/hash_table.c ../../src/scope.c ../../src/data_types.c

# Output
TARGET_LIB = symbol_table.so

# Default target
all: $(TARGET_LIB)

# Build shared library
$(TARGET_LIB): $(BRIDGE_SRC) $(SYMBOL_TABLE_SRC)
	$(CC) $(CFLAGS) $(INCLUDES) $(LDFLAGS) -o $(TARGET_LIB) $(BRIDGE_SRC) $(SYMBOL_TABLE_SRC)

# Clean build files
clean:
	rm -f $(TARGET_LIB)

# Install library (optional)
install: $(TARGET_LIB)
	cp $(TARGET_LIB) /usr/local/lib/

# Uninstall library (optional)
uninstall:
	rm -f /usr/local/lib/$(TARGET_LIB)

# Test the bridge
test: $(TARGET_LIB)
	python3 -c "
import ctypes
import os
lib = ctypes.CDLL('./$(TARGET_LIB)')
print('Library loaded successfully')
print('Available functions:')
for name in dir(lib):
    if not name.startswith('_'):
        print(f'  {name}')
"

.PHONY: all clean install uninstall test
