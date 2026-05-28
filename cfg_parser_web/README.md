# CFG Grammar Parser Web Application

A modern, interactive web-based Context-Free Grammar (CFG) parser with ASCII parse tree visualization and step-by-step parsing analysis.

## Features

### 🎯 Core Functionality
- **Grammar Input**: Enter CFG productions in standard format (e.g., `E -> E + T | T`)
- **Automatic Symbol Detection**: Identifies non-terminals, terminals, and start symbol
- **Grammar Validation**: Detects syntax errors and left recursion
- **FIRST/FOLLOW Set Computation**: Automatic calculation and display
- **LL(1) Parsing Table Generation**: Builds parsing tables with conflict detection
- **Dynamic Parsing**: Parse input strings according to the grammar
- **ASCII Parse Tree Visualization**: Text-based tree display
- **Step-by-Step Derivation**: Shows derivation steps with → notation
- **Parsing Steps Table**: Detailed stack/input/action display

### 🎨 User Interface
- **Modern Design**: Clean, responsive interface with Tailwind CSS
- **Dark/Light Theme**: Toggle between themes
- **Tabbed Navigation**: Separate tabs for different views
  - Grammar tab: View grammar details and productions
  - FIRST/FOLLOW tab: View computed sets
  - Parsing Table tab: View LL(1) parsing table
  - Parsing Steps tab: ASCII parse tree and step-by-step derivation
- **Export Functionality**: Export parsing results as PDF
- **Save/Load Grammar**: Save and load grammar files

### 🔧 Technical Features
- **React 18**: Modern React with hooks
- **Tailwind CSS**: Utility-first CSS framework
- **jsPDF**: PDF export functionality
- **Lucide React**: Modern icon library

## Installation

### Prerequisites
- Node.js (v16 or higher)
- npm or yarn

### Setup

1. Navigate to the project directory:
```bash
cd cfg_parser_web
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

### Basic Workflow

1. **Enter Grammar**: Type or paste your CFG grammar in the grammar input section
   - Format: `NonTerminal -> Production1 | Production2`
   - Example: `E -> E + T | T`

2. **Parse Grammar**: Click "Parse Grammar" to analyze the grammar
   - System will detect symbols and validate the grammar
   - FIRST/FOLLOW sets will be computed
   - LL(1) parsing table will be built

3. **Enter Input String**: Type the string you want to parse
   - Example: `id + id * id`

4. **Parse Input**: Click "Parse Input" to parse the string
   - Parse tree will be generated
   - Parsing steps will be recorded

5. **Explore Results**: Use tabs to view different aspects:
   - **Grammar**: View productions and symbol information
   - **FIRST/FOLLOW**: View computed sets
   - **Parsing Table**: View LL(1) table
   - **Parsing Steps**: ASCII parse tree and step-by-step derivation

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

## Grammar Format

### Supported Formats
- **Standard**: `E -> E + T | T`
- **Multi-line**: Each production on separate line
- **Comments**: Lines starting with `#` are ignored
- **Epsilon**: Use `ε` or `epsilon`

### Symbol Rules
- **Non-terminals**: Uppercase letters or multi-character names
- **Terminals**: Lowercase letters or quoted strings
- **Operators**: `+`, `-`, `*`, `/`, `(`, `)`, etc.

## Features in Detail

### Grammar Validation
- Syntax error detection
- Left recursion detection
- Ambiguity warnings
- LL(1) conflict detection

### FIRST/FOLLOW Sets
- Automatic computation
- Visual display in tabular format
- LL(1) grammar validation
- Conflict reporting

### LL(1) Parsing Table
- Automatic table generation
- Conflict highlighting
- Export as CSV
- Visual representation

### Parse Tree Visualization
- ASCII-based tree display
- Hierarchical structure with tree characters
- Clear parent-child relationships
- Export as PDF

### Step-by-Step Parsing
- Derivation steps with → notation
- Stack display
- Input buffer display
- Action description
- Export as PDF

## Advanced Features

### Export Options
- **Parsing Results**: Export parse tree and derivation as PDF
- **Grammar**: Save as text file
- **Parsing Table**: Export as CSV

### Theme Support
- Light theme (default)
- Dark theme
- Persistent theme preference

### File Operations
- Load grammar from file
- Save grammar to file
- Example grammar loading

## Browser Compatibility

- Chrome (recommended)
- Firefox
- Safari
- Edge

## Technology Stack

- **Frontend**: React 18
- **Styling**: Tailwind CSS
- **Icons**: Lucide React
- **Export**: jsPDF
- **Build**: Create React App

## Project Structure

```
cfg_parser_web/
├── public/
│   └── index.html
├── src/
│   ├── components/
│   │   ├── GrammarTab.jsx
│   │   ├── FirstFollowTab.jsx
│   │   ├── ParsingTableTab.jsx
│   │   └── ParsingStepsTab.jsx
│   ├── utils/
│   │   └── cfgParser.js
│   ├── App.jsx
│   ├── index.js
│   └── index.css
├── package.json
├── tailwind.config.js
├── postcss.config.js
└── README.md
```

## Development

### Available Scripts

- `npm start` - Start development server
- `npm build` - Build for production
- `npm test` - Run tests
- `npm eject` - Eject from Create React App

### Customization

To customize the application:
1. Modify components in `src/components/`
2. Update parsing logic in `src/utils/cfgParser.js`
3. Change styling in `tailwind.config.js`
4. Update theme colors in `src/index.css`

## Troubleshooting

### Common Issues

**Grammar parsing fails**
- Check for proper arrow format (`->`)
- Ensure non-terminals are uppercase
- Verify production syntax

**Parsing table has conflicts**
- Grammar may not be LL(1)
- Check for left recursion
- Consider left factoring

**Parse tree not displaying**
- Ensure parsing was successful
- Check for parsing errors
- Verify React Flow is properly loaded

## Future Enhancements

- [ ] LR(0)/SLR parser support
- [ ] Error recovery visualization
- [ ] More export formats (SVG, JSON)
- [ ] Grammar transformation tools
- [ ] Left factoring suggestions
- [ ] Ambiguity detection and resolution
- [ ] Multiple grammar comparison
- [ ] Grammar library with examples

## Contributing

Contributions are welcome! Please feel free to submit issues or pull requests.

## License

This project is provided for educational purposes.

## Acknowledgments

- Tailwind CSS for styling
- Lucide for icons
- The compiler theory community for inspiration

---

Built with ❤️ using React and Tailwind CSS
