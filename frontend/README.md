# Symbol Table Visualizer - Frontend

A modern, interactive web application for visualizing compiler design concepts including symbol tables, parse trees, scope hierarchies, and CFG grammar parsing.

## Features

### Core Functionality
- **Symbol Table Visualization**: Interactive grid view of symbol table entries with filtering and editing capabilities
- **Scope Hierarchy**: Visual representation of nested scopes in C programs
- **Code Parser**: Real-time C code parsing with syntax highlighting and error detection
- **CFG Grammar Parser**: Complete LL(1) grammar parsing with FIRST/FOLLOW sets, parsing tables, and step-by-step derivation
- **Parse Tree Visualization**: ASCII-based tree display for parsed expressions
- **Advanced Error Detection**: Comprehensive error detection including syntax, semantic, and logic errors
- **Real-time Analysis**: WebSocket-based real-time updates from the backend

### User Interface
- **Modern Design**: Clean, responsive interface with Tailwind CSS
- **Dark/Light Theme**: Toggle between themes with persistent preference
- **Tabbed Navigation**: 
  - Symbol Table: View and manage symbol table entries
  - Scope Hierarchy: Visualize nested scopes
  - Code Parser: Parse C code with real-time feedback
  - Parsing: CFG grammar parsing with LL(1) analysis
  - Error Detection: Advanced error analysis
  - Problems: View all detected issues
- **Real-time Status**: WebSocket connection status indicator
- **Quick Stats**: Dashboard showing analysis state, current scope, and system status

## Technology Stack

- **Frontend Framework**: React 18 with hooks
- **Styling**: Tailwind CSS
- **Icons**: Lucide React
- **Build Tool**: Create React App
- **Backend Integration**: WebSocket and REST API

## Installation

### Prerequisites
- Node.js (v16 or higher)
- npm or yarn
- Backend server running on port 8004

### Setup

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

4. Open your browser and navigate to:
```
http://localhost:3000
```

## Usage

### Symbol Table Tab
- View all symbols in the symbol table
- Click on symbols to view details
- Edit symbol properties
- Delete symbols
- See problem indicators for symbols with issues

### Scope Hierarchy Tab
- Visualize nested scopes in your code
- Click on scopes to see their contents
- Navigate through the scope hierarchy

### Code Parser Tab
- Enter or paste C code
- Parse the code to generate symbol table entries
- View real-time analysis results
- Navigate to specific lines with errors
- Step through the parsing process

### Parsing Tab (CFG Grammar Parser)
- Enter CFG grammar productions in standard format (e.g., `E -> E + T | T`)
- Parse the grammar to compute FIRST and FOLLOW sets
- Build LL(1) parsing tables
- Parse input strings according to the grammar
- View ASCII parse tree visualization
- See step-by-step derivation with → notation
- View detailed parsing steps (stack, input, action)

### Error Detection Tab
- Run advanced static code analysis
- Detect syntax, semantic, and logic errors
- Get suggestions for code improvement
- View warnings and tips

### Problems Tab
- See all detected problems in one place
- Navigate to problem locations in the code
- Filter by severity (error, warning, info)

## CFG Grammar Parser Features

The CFG Grammar Parser in the Parsing tab provides:

### Grammar Input
- Standard CFG format support (e.g., `E -> E + T | T`)
- Automatic symbol detection (non-terminals, terminals, start symbol)
- Grammar validation with error and warning detection
- Left recursion detection

### FIRST/FOLLOW Sets
- Automatic computation of FIRST sets for all symbols
- Automatic computation of FOLLOW sets for all non-terminals
- LL(1) grammar validation
- Conflict detection and reporting

### LL(1) Parsing Table
- Automatic table generation from grammar
- Conflict highlighting
- Visual table representation

### Dynamic Parsing
- Parse input strings using the generated LL(1) table
- ASCII parse tree visualization
- Step-by-step derivation with → notation
- Detailed parsing steps (stack, input, action)
- Accept/Reject status with error reporting

### Example Grammar
```
E -> E + T | E - T | T
T -> T * F | T / F | F
F -> ( E ) | id
```

### Example Input
```
id + id * id
```

## Project Structure

```
frontend/
├── public/
│   └── index.html
├── src/
│   ├── components/
│   │   ├── AdvancedErrorDetection.jsx
│   │   ├── CFGParser.jsx
│   │   ├── CodeParser.jsx
│   │   ├── ProblemsPanel.jsx
│   │   ├── ScopeTree.jsx
│   │   └── SymbolTableGrid.jsx
│   ├── services/
│   │   └── api.js
│   ├── utils/
│   │   └── cfgParser.js
│   ├── App.jsx
│   ├── index.css
│   └── index.js
├── package.json
└── README.md
```

## Backend Integration

The frontend communicates with the backend through:

1. **REST API**: For fetching symbols and analyzing code
2. **WebSocket**: For real-time updates on symbol table changes and scope changes

### API Endpoints
- `GET /api/symbols` - Fetch all symbols
- `POST /api/analyze` - Analyze code for errors
- WebSocket `/ws` - Real-time updates

## Development

### Adding New Features

1. Create a new component in `src/components/`
2. Import it in `App.jsx`
3. Add it to the tabs array
4. Add the corresponding icon from lucide-react

### Styling

The application uses Tailwind CSS for styling. Custom styles are defined in `src/index.css`.

### State Management

State is managed using React hooks (useState, useEffect, useRef). For complex state, consider using Context API or a state management library.

## Troubleshooting

### WebSocket Connection Issues
- Ensure the backend server is running on port 8004
- Check browser console for WebSocket errors
- Verify firewall settings

### Build Errors
- Clear node_modules and reinstall: `rm -rf node_modules && npm install`
- Check Node.js version compatibility
- Clear browser cache

### Performance Issues
- For large code files, consider implementing virtual scrolling
- Optimize re-renders using React.memo and useMemo
- Debounce real-time analysis input

## Browser Support

- Chrome (recommended)
- Firefox
- Safari
- Edge

## Contributing

Contributions are welcome! Please follow these guidelines:

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test thoroughly
5. Submit a pull request

## License

This project is provided for educational purposes.

## Acknowledgments

- Tailwind CSS for styling
- Lucide for icons
- React for the frontend framework
- The compiler theory community for inspiration

---

Built with ❤️ using React and Tailwind CSS
