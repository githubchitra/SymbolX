# Symbol Table Visualizer - Complete Compiler Design Tool

A comprehensive educational tool for visualizing compiler design concepts, including symbol tables, parse trees, scope hierarchies, and CFG grammar parsing. This project combines a C-based backend with a modern React frontend.

## Project Overview

This project demonstrates key compiler design concepts through interactive visualization:
- **Symbol Table Management**: Track variables, functions, and their properties across different scopes
- **Scope Hierarchy**: Visualize nested scopes in C programs
- **Parse Tree Generation**: Build and display abstract syntax trees
- **CFG Grammar Parsing**: Complete LL(1) grammar analysis with FIRST/FOLLOW sets and parsing tables
- **Error Detection**: Advanced static code analysis for syntax, semantic, and logic errors

## Project Structure

```
symbol-table-PBL/
├── backend/              # FastAPI backend with C bridge
│   ├── c_bridge/        # C library for symbol table operations
│   ├── app.py           # FastAPI application
│   └── requirements.txt # Python dependencies
├── frontend/            # React frontend application
│   ├── src/
│   │   ├── components/  # React components
│   │   ├── services/    # API services
│   │   └── utils/       # Utility functions (CFG parser)
│   ├── public/          # Static assets
│   ├── package.json     # Node dependencies
│   └── README.md        # Frontend documentation
├── mini_compiler/       # C++ mini compiler implementation
│   ├── include/         # Header files
│   ├── src/             # Source files
│   └── Makefile         # Build configuration
└── README.md            # This file
```

## Features

### Backend (C + FastAPI)
- **Symbol Table Operations**: Add, update, delete, and query symbols
- **Scope Management**: Track scope entry and exit
- **Real-time Updates**: WebSocket support for live updates
- **C Bridge**: Efficient C library integration via ctypes
- **Code Analysis**: Static analysis for error detection

### Frontend (React)
- **Symbol Table Grid**: Interactive table with filtering and editing
- **Scope Tree Visualization**: Hierarchical scope display
- **Code Parser**: Real-time C code parsing with syntax highlighting
- **CFG Grammar Parser**: LL(1) grammar analysis with:
  - FIRST/FOLLOW set computation
  - Parsing table generation
  - ASCII parse tree visualization
  - Step-by-step derivation
- **Error Detection Panel**: Comprehensive error analysis
- **Dark/Light Theme**: Modern UI with theme toggle
- **Real-time Updates**: WebSocket integration

### Mini Compiler (C++)
- **Lexical Analysis**: Tokenization and lexical error detection
- **Syntactic Analysis**: Recursive descent parsing with AST generation
- **Semantic Analysis**: Type checking and symbol validation
- **AST Visualization**: Multiple output formats (DOT, JSON, XML, Mermaid)
- **CFG Grammar Parsing**: Dynamic LL(1) parser for custom grammars
- **Complete Compiler Pipeline**: From source code to semantic analysis

## Installation

### Prerequisites
- **Backend**: Python 3.8+, GCC compiler
- **Frontend**: Node.js 16+, npm or yarn
- **Mini Compiler**: C++17 compatible compiler (GCC, Clang, MSVC)

### Backend Setup

1. Navigate to the backend directory:
```bash
cd backend
```

2. Create a virtual environment:
```bash
python -m venv venv
source venv/bin/activate  # On Windows: venv\Scripts\activate
```

3. Install dependencies:
```bash
pip install -r requirements.txt
```

4. Build the C bridge:
```bash
cd c_bridge
gcc -shared -o symbol_table.dll symbol_table.c
cd ..
```

5. Start the backend server:
```bash
python app.py
```

The backend will run on `http://localhost:8004`

### Frontend Setup

1. Navigate to the frontend directory:
```bash
cd frontend
```

2. Install dependencies:
```bash
npm install
```

3. Start the development server:
```bash
npm start
```

The frontend will run on `http://localhost:3000`

### Mini Compiler Setup

1. Navigate to the mini_compiler directory:
```bash
cd mini_compiler
```

2. Build the project:
```bash
make
```

3. Run the compiler:
```bash
./compiler                    # Interactive mode
./compiler test                # Run all tests
./compiler cfg                 # CFG grammar parsing mode
./compiler <filename>         # Compile a file
```

## Usage

### Symbol Table Visualizer

1. Start both the backend and frontend servers
2. Open `http://localhost:3000` in your browser
3. Use the **Code Parser** tab to enter C code
4. Click **Parse** to generate the symbol table
5. Navigate through different tabs to explore:
   - **Symbol Table**: View all symbols with details
   - **Scope Hierarchy**: Visualize nested scopes
   - **Parsing**: CFG grammar parsing with LL(1) analysis
   - **Error Detection**: Run advanced static analysis
   - **Problems**: View all detected issues

### CFG Grammar Parser

1. Navigate to the **Parsing** tab
2. Enter a CFG grammar in standard format:
   ```
   E -> E + T | T
   T -> T * F | F
   F -> ( E ) | id
   ```
3. Click **Parse Grammar** to compute FIRST/FOLLOW sets
4. Enter an input string: `id + id * id`
5. Click **Parse Input** to see:
   - ASCII parse tree
   - Step-by-step derivation
   - Detailed parsing steps

### Mini Compiler

1. Run the compiler in interactive mode:
   ```bash
   ./compiler
   ```
2. Or use CFG grammar parsing mode:
   ```bash
   ./compiler cfg
   ```
3. Load a grammar interactively or from a file
4. Analyze the grammar to compute FIRST/FOLLOW sets
5. Build the parsing table
6. Parse input strings and view the parse tree

## Technology Stack

### Backend
- **Language**: Python 3.8+
- **Framework**: FastAPI
- **C Bridge**: ctypes for C library integration
- **WebSocket**: Real-time updates
- **Build**: GCC for C compilation

### Frontend
- **Framework**: React 18
- **Styling**: Tailwind CSS
- **Icons**: Lucide React
- **Build**: Create React App
- **State**: React Hooks

### Mini Compiler
- **Language**: C++17
- **Build**: Makefile
- **Features**: Lexical, syntactic, and semantic analysis

## API Documentation

### REST API Endpoints

#### Symbol Table Operations
- `GET /api/symbols` - Get all symbols
- `POST /api/symbols` - Add a new symbol
- `PUT /api/symbols/{name}` - Update a symbol
- `DELETE /api/symbols/{name}` - Delete a symbol

#### Code Analysis
- `POST /api/analyze` - Analyze code for errors

### WebSocket Events

#### Client → Server
- No specific events required (server broadcasts updates)

#### Server → Client
- `symbol_added` - New symbol added to table
- `symbol_updated` - Symbol properties updated
- `scope_entered` - Entered a new scope
- `scope_exited` - Exited a scope

## Development

### Adding New Features

#### Backend
1. Add new endpoints in `app.py`
2. Update C bridge functions in `c_bridge/symbol_table.c`
3. Rebuild the C library
4. Update API documentation

#### Frontend
1. Create new components in `frontend/src/components/`
2. Add API service functions in `frontend/src/services/api.js`
3. Update state management in `frontend/src/App.jsx`
4. Add navigation tabs as needed

#### Mini Compiler
1. Add new classes in `include/` directory
2. Implement in `src/` directory
3. Update Makefile with new source files
4. Add test cases

## Testing

### Backend Tests
```bash
cd backend
python -m pytest tests/
```

### Frontend Tests
```bash
cd frontend
npm test
```

### Mini Compiler Tests
```bash
cd mini_compiler
./compiler test
```

## Troubleshooting

### Backend Issues
- **C Bridge Errors**: Ensure GCC is installed and the DLL is built correctly
- **Port Conflicts**: Change the port in `app.py` if 8004 is in use
- **WebSocket Connection**: Check firewall settings and ensure backend is running

### Frontend Issues
- **Build Errors**: Clear node_modules and reinstall dependencies
- **WebSocket Connection**: Ensure backend is running on port 8004
- **Performance**: For large files, consider implementing virtual scrolling

### Mini Compiler Issues
- **Compilation Errors**: Ensure C++17 compatible compiler is used
- **Linking Errors**: Check Makefile configuration in mini_compiler directory
- **Runtime Errors**: Verify input format matches expected grammar

## Contributing

Contributions are welcome! Please follow these guidelines:

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Add tests for new features
5. Update documentation
6. Submit a pull request

## License

This project is provided for educational purposes.

## Acknowledgments

- **Compiler Theory**: Based on standard compiler design principles
- **FastAPI**: Modern Python web framework
- **React**: JavaScript library for building user interfaces
- **Tailwind CSS**: Utility-first CSS framework
- **Lucide**: Beautiful icon library
- The compiler theory community for inspiration and resources

## Contact

For questions or suggestions, please open an issue on the project repository.

---

Built with ❤️ for educational purposes
