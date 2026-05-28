# Mini Compiler Front-End

A comprehensive C++ implementation of a compiler front-end that integrates a Parse Tree (Syntax Tree) with a Symbol Table for semantic analysis. This project demonstrates the fundamental concepts of compiler construction including lexical analysis, syntactic analysis, and semantic analysis.

## Features

### 🔍 **Lexical Analysis**
- Token recognition for C-like language subset
- Support for keywords, identifiers, literals, operators, and delimiters
- Error detection and reporting during tokenization
- Comment handling (single-line and multi-line)

### 🌳 **Parse Tree (AST) Construction**
- Abstract Syntax Tree (AST) with comprehensive node types
- Support for expressions, statements, declarations, and control flow
- Tree traversal and manipulation utilities
- Visual representation of program structure

### 📊 **Symbol Table Management**
- Hierarchical scope management
- Symbol information tracking (type, kind, scope, initialization state)
- Support for variables, functions, and parameters
- Redefinition detection and usage tracking

### 🔬 **Semantic Analysis**
- Type checking and compatibility validation
- Function call argument verification
- Variable initialization and usage analysis
- Return type validation
- Comprehensive error detection and reporting

### 🛠 **Error Handling**
- Detailed error messages with line and column information
- Error categorization (lexical, syntax, semantic)
- Warning system for potential issues
- Error recovery and synchronization

## Language Subset

The compiler supports a subset of C with the following features:

### **Data Types**
- `int` - Integer values
- `float` - Floating-point values  
- `char` - Character values
- `void` - Void type (for functions)

### **Operators**
- Arithmetic: `+`, `-`, `*`, `/`
- Comparison: `==`, `!=`, `<`, `>`, `<=`, `>=`
- Assignment: `=`

### **Control Structures**
- `if-else` statements
- `while` loops
- `for` loops
- `return` statements

### **Functions**
- Function declarations and definitions
- Parameters and return values
- Function calls with argument checking

### **Declarations**
- Variable declarations with optional initialization
- Function declarations with parameter lists
- Scope-based variable visibility

## Architecture

```
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│   Source Code   │───▶│   Lexer (Token  │───▶│  Parser (AST    │
│                 │    │   Generation)   │    │  Construction)  │
└─────────────────┘    └─────────────────┘    └─────────────────┘
                                                        │
                                                        ▼
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│   Error/Warning │◀───│  Semantic      │◀───│  Symbol Table   │
│   Reporting     │    │  Analyzer      │    │  Management     │
└─────────────────┘    └─────────────────┘    └─────────────────┘
```

## Building the Project

### Prerequisites
- C++17 compatible compiler (GCC, Clang, MSVC)
- Make utility
- Git (for cloning)

### Compilation

```bash
# Clone or navigate to the project directory
cd mini_compiler

# Build the project
make

# Or build with debug symbols
make debug

# Clean build files
make clean

# Rebuild everything
make rebuild
```

## Usage

### Interactive Mode
```bash
# Run the compiler in interactive mode
./mini_compiler
```

### Compile from File
```bash
# Compile a source file
./mini_compiler source.c
```

### Run Tests
```bash
# Run all test cases
./mini_compiler test

# Run error detection tests
./mini_compiler error

# Run demonstration
./mini_compiler demo
```

### Command Line Options
- `test` - Run all valid test cases
- `error` - Run error detection tests
- `demo` - Run comprehensive demonstration
- `help` - Show usage information
- `<filename>` - Compile specified source file

## Example Programs

### Simple Function
```c
int add(int a, int b) {
    return a + b;
}

int main() {
    int result = add(5, 3);
    return result;
}
```

### Control Flow
```c
int factorial(int n) {
    if (n <= 1) {
        return 1;
    } else {
        return n * factorial(n - 1);
    }
}

int main() {
    return factorial(5);
}
```

### Loops and Variables
```c
int sum(int n) {
    int result = 0;
    for (int i = 1; i <= n; i = i + 1) {
        result = result + i;
    }
    return result;
}

int main() {
    return sum(10);
}
```

## Error Detection Examples

### Undeclared Variable
```c
int main() {
    int x = undeclared_var + 5;  // Error: undeclared_var not declared
    return x;
}
```

### Type Mismatch
```c
int main() {
    int x = "hello";  // Error: cannot assign string to int
    return x;
}
```

### Function Call Mismatch
```c
int add(int a, int b) {
    return a + b;
}

int main() {
    int result = add(5);  // Error: function expects 2 arguments
    return result;
}
```

## Project Structure

```
mini_compiler/
├── include/                 # Header files
│   ├── token.h             # Token definitions
│   ├── lexer.h             # Lexer interface
│   ├── ast_node.h          # AST node classes
│   ├── symbol_table.h      # Symbol table management
│   ├── parser.h            # Parser interface
│   ├── semantic_analyzer.h # Semantic analyzer
│   └── compiler.h          # Main compiler class
├── src/                    # Implementation files
│   ├── token.cpp
│   ├── lexer.cpp
│   ├── ast_node.cpp
│   ├── symbol_table.cpp
│   ├── parser.cpp
│   ├── semantic_analyzer.cpp
│   ├── compiler.cpp
│   └── main.cpp
├── Makefile                # Build configuration
└── README.md               # This file
```

## Key Classes and Components

### **Token**
- Represents lexical tokens with type, value, and position
- Supports keywords, identifiers, literals, operators, and delimiters

### **Lexer**
- Performs lexical analysis on source code
- Generates token stream with error detection
- Handles comments and whitespace

### **ASTNode**
- Base class for all Abstract Syntax Tree nodes
- Supports hierarchical tree structure with parent-child relationships
- Specialized node types for different language constructs

### **SymbolTable**
- Manages symbol information with scope hierarchy
- Tracks variable declarations, function definitions, and parameters
- Provides symbol lookup and scope management

### **Parser**
- Performs syntactic analysis using recursive descent parsing
- Constructs AST from token stream
- Integrates with symbol table for declaration tracking

### **SemanticAnalyzer**
- Performs semantic analysis on AST
- Type checking, function call validation, and variable usage analysis
- Comprehensive error detection and reporting

### **Compiler**
- Main compiler class coordinating all phases
- Provides unified interface for compilation process
- Handles error aggregation and reporting

## Testing

The project includes comprehensive test cases covering:

- ✅ Valid programs (should compile successfully)
- ❌ Invalid programs (should detect errors)
- 🔍 Edge cases and boundary conditions
- 📊 Performance and memory management

### Running Tests
```bash
# Run all valid test cases
make test

# Run error detection tests
make test-errors

# Run specific test manually
./mini_compiler << EOF
int x = 5;
return x;
EOF
```

## Error Categories

### **Lexical Errors**
- Invalid characters
- Unterminated string literals
- Unterminated character literals

### **Syntax Errors**
- Missing semicolons
- Unmatched parentheses/braces
- Invalid token sequences

### **Semantic Errors**
- Undeclared variables/functions
- Type mismatches
- Invalid assignments
- Function call parameter mismatches
- Return type inconsistencies

## Performance Characteristics

- **Time Complexity**: O(n) for lexical analysis, O(n) for parsing, O(n) for semantic analysis
- **Space Complexity**: O(n) for AST and symbol table storage
- **Memory Management**: Smart pointers for automatic memory management
- **Error Recovery**: Synchronization points for error recovery

## Extensions and Future Work

Potential enhancements for the mini compiler:

- 🎯 **Additional Language Features**
  - Arrays and pointers
  - Structs and enums
  - More operators and expressions
  
- 🔄 **Optimization**
  - Constant folding
  - Dead code elimination
  - Peephole optimizations
  
- 📤 **Code Generation**
  - Target code generation
  - Intermediate representation
  - Register allocation
  
- 🔧 **Tool Integration**
  - IDE integration
  - Language server protocol
  - Debug information generation

## Contributing

Feel free to contribute to this project by:

1. Forking the repository
2. Creating a feature branch
3. Making your changes
4. Adding tests for new functionality
5. Submitting a pull request

## License

This project is provided for educational purposes. Feel free to use and modify it for learning and experimentation.

---

**Note**: This is a simplified compiler front-end designed for educational purposes. It demonstrates the core concepts of compiler construction while maintaining a clean, readable codebase suitable for learning and experimentation.
