import React, { useState, useEffect, useRef } from 'react';
import { Play, RotateCcw, AlertCircle, CheckCircle, BookOpen, Layers, Cpu, HelpCircle, Sliders, ChevronRight } from 'lucide-react';

// ==========================================
// LEXER (TOKENIZER) IMPLEMENTATION
// ==========================================
const tokenize = (input) => {
  const tokens = [];
  let i = 0;
  while (i < input.length) {
    const char = input[i];
    if (/\s/.test(char)) {
      i++;
      continue;
    }
    if (/[0-9]/.test(char)) {
      let num = '';
      while (i < input.length && /[0-9.]/.test(input[i])) {
        num += input[i];
        i++;
      }
      tokens.push({ type: 'NUMBER', value: num, index: i - num.length });
      continue;
    }
    if (/[a-zA-Z_]/.test(char)) {
      let id = '';
      while (i < input.length && /[a-zA-Z0-9_]/.test(input[i])) {
        id += input[i];
        i++;
      }
      // Check if this is a supported function
      if (['sin', 'cos', 'tan', 'sqrt', 'log', 'exp', 'abs'].includes(id.toLowerCase())) {
        tokens.push({ type: 'FUNCTION', value: id, index: i - id.length });
      } else {
        tokens.push({ type: 'IDENTIFIER', value: id, index: i - id.length });
      }
      continue;
    }
    if (['+', '-', '*', '/', '^', '%'].includes(char)) {
      tokens.push({ type: 'OPERATOR', value: char, index: i });
      i++;
      continue;
    }
    if (char === '(') {
      tokens.push({ type: 'LPAREN', value: char, index: i });
      i++;
      continue;
    }
    if (char === ')') {
      tokens.push({ type: 'RPAREN', value: char, index: i });
      i++;
      continue;
    }
    throw new Error(`Unexpected character: "${char}" at position ${i + 1}`);
  }
  return tokens;
};

// ==========================================
// CST (CONCRETE SYNTAX TREE) PARSER
// Grammar:
// E  -> T E'
// E' -> + T E' | - T E' | ε
// T  -> F T'
// T' -> * F T' | / F T' | ε
// F  -> ( E ) | id | num | function ( E )
// ==========================================
class CSTParser {
  constructor(tokens) {
    this.tokens = tokens;
    this.index = 0;
    this.steps = [];
  }

  peek() {
    return this.tokens[this.index];
  }

  consume() {
    const tok = this.tokens[this.index++];
    if (tok) this.steps.push(`Consumed token "${tok.value}" of type ${tok.type}`);
    return tok;
  }

  match(type, value) {
    const token = this.peek();
    if (token && token.type === type && (!value || token.value === value)) {
      return this.consume();
    }
    return null;
  }

  parseE() {
    this.steps.push("Entering E -> T E'");
    const children = [];
    const tNode = this.parseT();
    children.push(tNode);
    const ePrimeNode = this.parseEPrime();
    children.push(ePrimeNode);
    return { type: 'E', value: 'Expression', children };
  }

  parseEPrime() {
    this.steps.push("Entering E'");
    const children = [];
    const token = this.peek();
    if (token && token.type === 'OPERATOR' && (token.value === '+' || token.value === '-')) {
      const opToken = this.consume();
      children.push({ type: 'terminal', value: opToken.value, isTerminal: true });
      const tNode = this.parseT();
      children.push(tNode);
      const ePrimeNode = this.parseEPrime();
      children.push(ePrimeNode);
      return { type: "E'", value: `AdditiveTail (${opToken.value})`, children };
    }
    // ε
    this.steps.push("E' matched ε");
    return { type: "E'", value: 'ε (empty)', children: [{ type: 'terminal', value: 'ε', isTerminal: true }] };
  }

  parseT() {
    this.steps.push("Entering T -> F T'");
    const children = [];
    const fNode = this.parseF();
    children.push(fNode);
    const tPrimeNode = this.parseTPrime();
    children.push(tPrimeNode);
    return { type: 'T', value: 'Term', children };
  }

  parseTPrime() {
    this.steps.push("Entering T'");
    const children = [];
    const token = this.peek();
    if (token && token.type === 'OPERATOR' && (token.value === '*' || token.value === '/')) {
      const opToken = this.consume();
      children.push({ type: 'terminal', value: opToken.value, isTerminal: true });
      const fNode = this.parseF();
      children.push(fNode);
      const tPrimeNode = this.parseTPrime();
      children.push(tPrimeNode);
      return { type: "T'", value: `MultiplicativeTail (${opToken.value})`, children };
    }
    // ε
    this.steps.push("T' matched ε");
    return { type: "T'", value: 'ε (empty)', children: [{ type: 'terminal', value: 'ε', isTerminal: true }] };
  }

  parseF() {
    this.steps.push("Entering F");
    const children = [];
    const token = this.peek();
    
    if (!token) {
      throw new Error('Unexpected end of expression: expected number, variable, function, or "("');
    }

    if (token.type === 'FUNCTION') {
      const funcToken = this.consume();
      children.push({ type: 'terminal', value: funcToken.value, isTerminal: true });
      
      const nextToken = this.peek();
      if (!nextToken || nextToken.type !== 'LPAREN') {
        throw new Error(`Expected "(" after function "${funcToken.value}"`);
      }
      this.consume(); // consume '('
      children.push({ type: 'terminal', value: '(', isTerminal: true });
      
      const eNode = this.parseE();
      children.push(eNode);
      
      if (!this.match('RPAREN')) {
        throw new Error(`Mismatched parenthesis: expected ")" after function "${funcToken.value}" argument`);
      }
      children.push({ type: 'terminal', value: ')', isTerminal: true });
      
      return { type: 'F', value: `FuncCall (${funcToken.value})`, children };
    }

    if (token.type === 'LPAREN') {
      this.consume(); // '('
      children.push({ type: 'terminal', value: '(', isTerminal: true });
      
      const eNode = this.parseE();
      children.push(eNode);
      
      if (!this.match('RPAREN')) {
        throw new Error('Mismatched parenthesis: expected ")"');
      }
      children.push({ type: 'terminal', value: ')', isTerminal: true });
      return { type: 'F', value: 'Parenthesized', children };
    }

    if (token.type === 'NUMBER') {
      const numToken = this.consume();
      children.push({ type: 'terminal', value: numToken.value, isTerminal: true });
      return { type: 'F', value: `Number (${numToken.value})`, children };
    }

    if (token.type === 'IDENTIFIER') {
      const idToken = this.consume();
      children.push({ type: 'terminal', value: idToken.value, isTerminal: true });
      return { type: 'F', value: `Variable (${idToken.value})`, children };
    }

    throw new Error(`Unexpected token "${token.value}" of type ${token.type} in factor (F)`);
  }

  parse() {
    const tree = this.parseE();
    if (this.index < this.tokens.length) {
      throw new Error(`Extra tokens at end of expression: "${this.tokens[this.index].value}"`);
    }
    return tree;
  }
}

// ==========================================
// AST (ABSTRACT SYNTAX TREE) PARSER
// Precedence-based Pratt-ish parser
// ==========================================
class ASTParser {
  constructor(tokens) {
    this.tokens = tokens;
    this.index = 0;
  }

  peek() {
    return this.tokens[this.index];
  }

  consume() {
    return this.tokens[this.index++];
  }

  getPrecedence(op) {
    if (op === '+' || op === '-') return 1;
    if (op === '*' || op === '/' || op === '%') return 2;
    if (op === '^') return 3;
    return 0;
  }

  parsePrimary() {
    const token = this.peek();
    if (!token) {
      throw new Error('Unexpected end of expression');
    }

    if (token.type === 'NUMBER') {
      this.consume();
      return { type: 'number', value: token.value, children: [] };
    }

    if (token.type === 'IDENTIFIER') {
      this.consume();
      return { type: 'identifier', value: token.value, children: [] };
    }

    if (token.type === 'FUNCTION') {
      const funcName = this.consume().value;
      const nextToken = this.peek();
      if (!nextToken || nextToken.type !== 'LPAREN') {
        throw new Error(`Expected "(" after function "${funcName}"`);
      }
      this.consume(); // '('
      const arg = this.parseExpression(0);
      const closeToken = this.peek();
      if (!closeToken || closeToken.type !== 'RPAREN') {
        throw new Error(`Expected ")" after argument in function "${funcName}"`);
      }
      this.consume(); // ')'
      return { type: 'function', value: funcName, children: [arg] };
    }

    if (token.type === 'LPAREN') {
      this.consume(); // '('
      const expr = this.parseExpression(0);
      const nextToken = this.peek();
      if (!nextToken || nextToken.type !== 'RPAREN') {
        throw new Error('Mismatched parenthesis: expected ")"');
      }
      this.consume(); // ')'
      return expr;
    }

    // Unary negative/positive
    if (token.type === 'OPERATOR' && (token.value === '+' || token.value === '-')) {
      const op = this.consume().value;
      const operand = this.parsePrimary();
      return { type: 'unary', value: op, children: [operand] };
    }

    throw new Error(`Unexpected token "${token.value}"`);
  }

  parseExpression(minPrecedence = 0) {
    let left = this.parsePrimary();

    while (true) {
      const token = this.peek();
      if (!token || token.type !== 'OPERATOR') {
        break;
      }

      const precedence = this.getPrecedence(token.value);
      if (precedence < minPrecedence) {
        break;
      }

      this.consume(); // operator
      const op = token.value;
      
      // Right associativity for power (^)
      const nextMinPrecedence = op === '^' ? precedence : precedence + 1;
      const right = this.parseExpression(nextMinPrecedence);

      left = {
        type: 'operator',
        value: op,
        children: [left, right]
      };
    }

    return left;
  }

  parse() {
    const tree = this.parseExpression(0);
    if (this.index < this.tokens.length) {
      throw new Error(`Extra tokens remaining starting at "${this.tokens[this.index].value}"`);
    }
    return tree;
  }
}

// ==========================================
// DYNAMIC TREE GRAPH LAYOUT CALCULATOR
// Computes non-overlapping nodes coordinates
// ==========================================
const calculateTreeLayout = (node, depth = 0, index = { val: 0 }, spacingX = 65, spacingY = 70) => {
  if (!node) return null;

  const layoutNode = {
    type: node.type,
    value: node.value,
    isTerminal: node.isTerminal,
    depth,
    id: `${node.type}_${node.value}_${Math.random()}`
  };

  if (!node.children || node.children.length === 0) {
    // Leaf node column position
    layoutNode.x = index.val * spacingX;
    index.val += 1;
    layoutNode.children = [];
  } else {
    // Internal node recursive layout
    layoutNode.children = node.children.map(c => calculateTreeLayout(c, depth + 1, index, spacingX, spacingY));
    const firstChildX = layoutNode.children[0].x;
    const lastChildX = layoutNode.children[layoutNode.children.length - 1].x;
    layoutNode.x = (firstChildX + lastChildX) / 2;
  }

  layoutNode.y = depth * spacingY;
  return layoutNode;
};

// Flatten layed out tree for easy rendering of SVG nodes and connectors
const flattenTree = (layoutNode, list = [], parent = null) => {
  if (!layoutNode) return list;
  
  if (parent) {
    list.push({
      type: 'connection',
      x1: parent.x,
      y1: parent.y,
      x2: layoutNode.x,
      y2: layoutNode.y,
      id: `conn_${parent.id}_${layoutNode.id}`
    });
  }

  list.push({
    type: 'node',
    x: layoutNode.x,
    y: layoutNode.y,
    nodeType: layoutNode.type,
    value: layoutNode.value,
    isTerminal: layoutNode.isTerminal,
    id: layoutNode.id,
    originalNode: layoutNode
  });

  if (layoutNode.children) {
    layoutNode.children.forEach(c => flattenTree(c, list, layoutNode));
  }

  return list;
};

// Recursively evaluate AST with variable substitution
const evaluateAST = (node, variables = {}) => {
  if (!node) return 0;
  
  if (node.type === 'number') {
    return parseFloat(node.value);
  }
  
  if (node.type === 'identifier') {
    const val = variables[node.value];
    if (val === undefined || val === '') return 0;
    return parseFloat(val);
  }

  if (node.type === 'unary') {
    const operandVal = evaluateAST(node.children[0], variables);
    return node.value === '-' ? -operandVal : operandVal;
  }

  if (node.type === 'operator') {
    const leftVal = evaluateAST(node.children[0], variables);
    const rightVal = evaluateAST(node.children[1], variables);
    
    switch (node.value) {
      case '+': return leftVal + rightVal;
      case '-': return leftVal - rightVal;
      case '*': return leftVal * rightVal;
      case '/': return rightVal !== 0 ? leftVal / rightVal : NaN;
      case '%': return leftVal % rightVal;
      case '^': return Math.pow(leftVal, rightVal);
      default: return 0;
    }
  }

  if (node.type === 'function') {
    const argVal = evaluateAST(node.children[0], variables);
    switch (node.value.toLowerCase()) {
      case 'sin': return Math.sin(argVal);
      case 'cos': return Math.cos(argVal);
      case 'tan': return Math.tan(argVal);
      case 'sqrt': return Math.sqrt(argVal);
      case 'log': return Math.log(argVal);
      case 'exp': return Math.exp(argVal);
      case 'abs': return Math.abs(argVal);
      default: return 0;
    }
  }

  return 0;
};

// Detect unique variables in the AST
const findVariables = (node, vars = new Set()) => {
  if (!node) return vars;
  if (node.type === 'identifier') {
    vars.add(node.value);
  }
  if (node.children) {
    node.children.forEach(c => findVariables(c, vars));
  }
  return vars;
};

const ExpressionParser = () => {
  const [expression, setExpression] = useState('(x + 5) * sin(y)');
  const [treeType, setTreeType] = useState('AST'); // CST or AST
  const [tokens, setTokens] = useState([]);
  const [parseTree, setParseTree] = useState(null);
  const [parseSteps, setParseSteps] = useState([]);
  const [error, setError] = useState(null);
  const [variables, setVariables] = useState({ x: '10', y: '0' });
  const [variableList, setVariableList] = useState(['x', 'y']);
  const [evaluatedResult, setEvaluatedResult] = useState(null);
  const [hoveredNodeInfo, setHoveredNodeInfo] = useState(null);
  const [activeGuideTab, setActiveGuideTab] = useState('lexer');

  // Trigger parsing in real-time when expression or treeType changes
  useEffect(() => {
    if (!expression.trim()) {
      setTokens([]);
      setParseTree(null);
      setParseSteps([]);
      setError(null);
      setEvaluatedResult(null);
      return;
    }

    try {
      setError(null);
      // 1. Lexical Analysis
      const tokList = tokenize(expression);
      setTokens(tokList);

      // 2. Syntactic Analysis & Dynamic Parsing
      let parsedNode = null;
      if (treeType === 'CST') {
        const cstParser = new CSTParser(tokList);
        parsedNode = cstParser.parse();
        setParseSteps(cstParser.steps);
      } else {
        const astParser = new ASTParser(tokList);
        parsedNode = astParser.parse();
        setParseSteps([
          "Lexing complete. Created tokens.",
          "Parsing Abstract Syntax Tree using operator precedence...",
          "Resolved operator priorities: ^ > (*, /, %) > (+, -)"
        ]);
        
        // Find variables dynamically
        const varsFound = Array.from(findVariables(parsedNode));
        setVariableList(varsFound);
        
        // Retain existing values, add missing ones as '0'
        setVariables(prev => {
          const next = { ...prev };
          varsFound.forEach(v => {
            if (next[v] === undefined) next[v] = '0';
          });
          return next;
        });
      }

      setParseTree(parsedNode);

    } catch (err) {
      console.error(err);
      setError(err.message);
      setParseTree(null);
      setEvaluatedResult(null);
    }
  }, [expression, treeType]);

  // Handle dynamic AST evaluation
  useEffect(() => {
    if (treeType === 'AST' && parseTree) {
      try {
        const res = evaluateAST(parseTree, variables);
        setEvaluatedResult(res);
      } catch (err) {
        setEvaluatedResult(NaN);
      }
    } else {
      setEvaluatedResult(null);
    }
  }, [parseTree, variables, treeType]);

  // Handle variable slider/input changes
  const handleVariableChange = (name, value) => {
    setVariables(prev => ({
      ...prev,
      [name]: value
    }));
  };

  // Generate layouts for SVG rendering
  let svgWidth = 600;
  let svgHeight = 350;
  let minX = 0;
  let maxX = 1;
  let elements = [];
  
  if (parseTree) {
    const spacingX = treeType === 'CST' ? 45 : 70;
    const spacingY = treeType === 'CST' ? 60 : 75;
    const layout = calculateTreeLayout(parseTree, 0, { val: 0 }, spacingX, spacingY);
    elements = flattenTree(layout);
    
    const nodes = elements.filter(el => el.type === 'node');
    if (nodes.length > 0) {
      const xs = nodes.map(n => n.x);
      minX = Math.min(...xs);
      maxX = Math.max(...xs);
      const ys = nodes.map(n => n.y);
      svgHeight = Math.max(...ys) + 70;
    }
    
    // Auto scale and translate layout dynamically inside viewbox
    svgWidth = maxX - minX + 120;
    elements = elements.map(el => {
      if (el.type === 'node') {
        return { ...el, x: el.x - minX + 60 };
      } else {
        return {
          ...el,
          x1: el.x1 - minX + 60,
          x2: el.x2 - minX + 60
        };
      }
    });
  }

  // Node styles / colors depending on node type
  const getNodeColorClasses = (nodeType, isTerminal) => {
    if (isTerminal) {
      return {
        bg: 'fill-emerald-50 dark:fill-emerald-950/40',
        border: 'stroke-emerald-500 dark:stroke-emerald-400',
        text: 'fill-emerald-800 dark:fill-emerald-200',
        label: 'Terminal'
      };
    }
    
    switch (nodeType) {
      case 'E':
      case 'E\'':
      case 'operator':
        return {
          bg: 'fill-indigo-50 dark:fill-indigo-950/40',
          border: 'stroke-indigo-500 dark:stroke-indigo-400',
          text: 'fill-indigo-800 dark:fill-indigo-200',
          label: nodeType === 'operator' ? 'Operator' : 'Expression'
        };
      case 'T':
      case 'T\'':
      case 'function':
        return {
          bg: 'fill-fuchsia-50 dark:fill-fuchsia-950/40',
          border: 'stroke-fuchsia-500 dark:stroke-fuchsia-400',
          text: 'fill-fuchsia-800 dark:fill-fuchsia-200',
          label: nodeType === 'function' ? 'Function' : 'Term'
        };
      case 'F':
      case 'number':
        return {
          bg: 'fill-amber-50 dark:fill-amber-950/40',
          border: 'stroke-amber-500 dark:stroke-amber-400',
          text: 'fill-amber-800 dark:fill-amber-200',
          label: nodeType === 'number' ? 'Constant' : 'Factor'
        };
      case 'identifier':
        return {
          bg: 'fill-sky-50 dark:fill-sky-950/40',
          border: 'stroke-sky-500 dark:stroke-sky-400',
          text: 'fill-sky-800 dark:fill-sky-200',
          label: 'Variable'
        };
      default:
        return {
          bg: 'fill-slate-50 dark:fill-slate-900',
          border: 'stroke-slate-500 dark:stroke-slate-400',
          text: 'fill-slate-800 dark:fill-slate-200',
          label: nodeType
        };
    }
  };

  // Node Hover Evaluation Helper
  const handleNodeMouseEnter = (el) => {
    if (treeType === 'AST') {
      try {
        const value = evaluateAST(el.originalNode, variables);
        setHoveredNodeInfo({
          type: el.nodeType,
          symbol: el.value,
          evaluated: isNaN(value) ? 'Undefined / Div-by-Zero' : value.toFixed(4)
        });
      } catch (err) {
        setHoveredNodeInfo({
          type: el.nodeType,
          symbol: el.value,
          evaluated: 'Error'
        });
      }
    } else {
      setHoveredNodeInfo({
        type: el.nodeType,
        symbol: el.value,
        evaluated: null
      });
    }
  };

  return (
    <div className="bg-slate-50 dark:bg-slate-950 min-h-screen text-slate-800 dark:text-slate-100 flex flex-col gap-6">
      
      {/* Intro Header Card with Glassmorphism */}
      <div className="relative overflow-hidden rounded-2xl bg-gradient-to-br from-indigo-500 via-purple-600 to-pink-500 text-white p-6 shadow-xl border border-indigo-400/30">
        <div className="absolute top-0 right-0 p-8 opacity-10 pointer-events-none">
          <Layers className="h-44 w-44 rotate-12" />
        </div>
        <div className="max-w-3xl">
          <span className="bg-white/20 text-white text-[10px] font-bold uppercase tracking-wider px-2.5 py-1 rounded-full border border-white/25">
            Compiler Engine Module
          </span>
          <h2 className="text-3xl font-extrabold mt-3 tracking-tight">Interactive Expression Parser</h2>
          <p className="mt-2 text-indigo-100 text-sm leading-relaxed">
            Enter mathematical and algebraic expressions to parse them into Concrete Syntax Trees (CST) and Abstract Syntax Trees (AST) in real-time. Understand compiler lexical analysis, derivation structures, grammar associativity, and AST evaluations instantly!
          </p>
        </div>
      </div>

      {/* Main Grid Layout */}
      <div className="grid grid-cols-1 xl:grid-cols-3 gap-6">
        
        {/* Left Column: Inputs, Variables, Tokens */}
        <div className="xl:col-span-1 flex flex-col gap-6">
          
          {/* Section: Expression Input */}
          <div className="bg-white dark:bg-slate-900 rounded-2xl p-5 shadow-lg border border-slate-100 dark:border-slate-800/80">
            <h3 className="text-sm font-bold uppercase tracking-wider text-slate-400 dark:text-slate-500 mb-4 flex items-center gap-2">
              <Sliders className="h-4 w-4 text-indigo-500" />
              Configure Expression
            </h3>
            
            <div className="space-y-4">
              <div>
                <label className="text-xs font-semibold text-slate-500 dark:text-slate-400 block mb-1.5">
                  Enter Mathematical Expression
                </label>
                <div className="relative">
                  <input
                    type="text"
                    value={expression}
                    onChange={(e) => setExpression(e.target.value)}
                    className="w-full font-mono text-base bg-slate-50 dark:bg-slate-950 px-4 py-3 rounded-xl border border-slate-200 dark:border-slate-800 focus:outline-none focus:ring-2 focus:ring-indigo-500 dark:focus:ring-indigo-400 text-slate-900 dark:text-slate-100"
                    placeholder="e.g. (x + 5) * sin(y)"
                  />
                  {error ? (
                    <div className="absolute right-3 top-3.5 text-red-500" title={error}>
                      <AlertCircle className="h-5 w-5 animate-pulse" />
                    </div>
                  ) : expression.trim() ? (
                    <div className="absolute right-3 top-3.5 text-emerald-500" title="Valid expression syntax!">
                      <CheckCircle className="h-5 w-5" />
                    </div>
                  ) : null}
                </div>
              </div>

              {/* Selector for CST vs AST */}
              <div>
                <label className="text-xs font-semibold text-slate-500 dark:text-slate-400 block mb-2">
                  Select Tree Representation
                </label>
                <div className="grid grid-cols-2 gap-2 bg-slate-100 dark:bg-slate-950 p-1 rounded-xl">
                  <button
                    onClick={() => setTreeType('AST')}
                    className={`py-2 text-xs font-bold rounded-lg transition-all ${
                      treeType === 'AST'
                        ? 'bg-indigo-600 text-white shadow-sm'
                        : 'text-slate-500 hover:text-slate-800 dark:text-slate-400 dark:hover:text-slate-200'
                    }`}
                  >
                    AST (Abstract Tree)
                  </button>
                  <button
                    onClick={() => setTreeType('CST')}
                    className={`py-2 text-xs font-bold rounded-lg transition-all ${
                      treeType === 'CST'
                        ? 'bg-indigo-600 text-white shadow-sm'
                        : 'text-slate-500 hover:text-slate-800 dark:text-slate-400 dark:hover:text-slate-200'
                    }`}
                  >
                    CST (Concrete Parser)
                  </button>
                </div>
              </div>
            </div>

            {error && (
              <div className="mt-4 p-3 bg-red-50 dark:bg-red-950/20 border border-red-200 dark:border-red-800/40 rounded-xl flex gap-3 text-xs text-red-600 dark:text-red-400">
                <AlertCircle className="h-4 w-4 shrink-0 mt-0.5" />
                <div>
                  <span className="font-bold">Syntax Error:</span> {error}
                </div>
              </div>
            )}
          </div>

          {/* Section: Live Variable Substituting Evaluator (Only AST) */}
          {treeType === 'AST' && parseTree && variableList.length > 0 && (
            <div className="bg-white dark:bg-slate-900 rounded-2xl p-5 shadow-lg border border-slate-100 dark:border-slate-800/80">
              <h3 className="text-sm font-bold uppercase tracking-wider text-slate-400 dark:text-slate-500 mb-4 flex items-center gap-2">
                <Cpu className="h-4 w-4 text-fuchsia-500" />
                Value substitution
              </h3>
              
              <div className="space-y-4">
                {variableList.map(vName => (
                  <div key={vName} className="space-y-1">
                    <div className="flex justify-between items-center text-xs">
                      <span className="font-bold font-mono text-indigo-600 dark:text-indigo-400">{vName}</span>
                      <span className="text-slate-500 font-mono font-semibold">{variables[vName] || '0'}</span>
                    </div>
                    <div className="flex items-center gap-3">
                      <input
                        type="range"
                        min="-20"
                        max="20"
                        step="0.5"
                        value={variables[vName] || '0'}
                        onChange={(e) => handleVariableChange(vName, e.target.value)}
                        className="w-full accent-indigo-600 dark:accent-indigo-400"
                      />
                      <input
                        type="number"
                        step="0.1"
                        value={variables[vName] || '0'}
                        onChange={(e) => handleVariableChange(vName, e.target.value)}
                        className="w-16 text-center font-mono text-xs bg-slate-50 dark:bg-slate-950 py-1 border border-slate-200 dark:border-slate-800 rounded-lg text-slate-900 dark:text-slate-100"
                      />
                    </div>
                  </div>
                ))}

                <div className="pt-3 border-t border-slate-100 dark:border-slate-800/80 flex justify-between items-center">
                  <span className="text-xs font-bold text-slate-500">Evaluated AST Result:</span>
                  <span className="font-mono text-lg font-bold text-emerald-600 dark:text-emerald-400">
                    {evaluatedResult === null || isNaN(evaluatedResult)
                      ? 'Error / Div-by-zero'
                      : evaluatedResult.toFixed(5)}
                  </span>
                </div>
              </div>
            </div>
          )}

          {/* Section: Lexical Analysis Tokens */}
          <div className="bg-white dark:bg-slate-900 rounded-2xl p-5 shadow-lg border border-slate-100 dark:border-slate-800/80 flex-1 min-h-[220px]">
            <h3 className="text-sm font-bold uppercase tracking-wider text-slate-400 dark:text-slate-500 mb-4 flex items-center gap-2">
              <Layers className="h-4 w-4 text-emerald-500" />
              Lexical Analysis Tokens
            </h3>

            {tokens.length === 0 ? (
              <div className="h-full flex flex-col justify-center items-center text-center text-slate-400 py-10">
                <RotateCcw className="h-8 w-8 mb-2 animate-spin text-slate-300" />
                <p className="text-xs font-medium">Type an expression to extract lexical tokens</p>
              </div>
            ) : (
              <div className="flex flex-wrap gap-2 overflow-y-auto max-h-[300px]">
                {tokens.map((tok, idx) => {
                  let badgeColor = "bg-slate-100 dark:bg-slate-800 text-slate-700 dark:text-slate-300";
                  if (tok.type === 'NUMBER') badgeColor = "bg-amber-50 dark:bg-amber-950/40 text-amber-700 dark:text-amber-300 border border-amber-200/50";
                  else if (tok.type === 'IDENTIFIER') badgeColor = "bg-sky-50 dark:bg-sky-950/40 text-sky-700 dark:text-sky-300 border border-sky-200/50";
                  else if (tok.type === 'OPERATOR') badgeColor = "bg-indigo-50 dark:bg-indigo-950/40 text-indigo-700 dark:text-indigo-300 border border-indigo-200/50";
                  else if (tok.type === 'FUNCTION') badgeColor = "bg-fuchsia-50 dark:bg-fuchsia-950/40 text-fuchsia-700 dark:text-fuchsia-300 border border-fuchsia-200/50";
                  else if (tok.type === 'LPAREN' || tok.type === 'RPAREN') badgeColor = "bg-emerald-50 dark:bg-emerald-950/40 text-emerald-700 dark:text-emerald-300 border border-emerald-200/50";

                  return (
                    <div
                      key={idx}
                      className={`flex flex-col px-3 py-1.5 rounded-lg text-xs font-mono transition-all ${badgeColor}`}
                    >
                      <span className="text-[9px] uppercase font-bold text-slate-400 dark:text-slate-500 tracking-wider">
                        {tok.type}
                      </span>
                      <span className="font-bold text-sm leading-tight mt-0.5">{tok.value}</span>
                    </div>
                  );
                })}
              </div>
            )}
          </div>

        </div>

        {/* Middle/Right Columns: SVG Canvas & Real-time Tree Visualizer */}
        <div className="xl:col-span-2 flex flex-col gap-6">
          
          <div className="bg-white dark:bg-slate-900 rounded-2xl p-5 shadow-lg border border-slate-100 dark:border-slate-800/80 flex flex-col min-h-[480px]">
            <div className="flex justify-between items-center border-b border-slate-100 dark:border-slate-800/80 pb-4 mb-4">
              <div>
                <h3 className="font-extrabold text-lg text-slate-950 dark:text-slate-50">
                  {treeType === 'AST' ? 'Abstract Syntax Tree (AST)' : 'Concrete Syntax Tree (CST)'}
                </h3>
                <p className="text-xs text-slate-500 dark:text-slate-400">
                  {treeType === 'AST' 
                    ? 'A simplified parse tree that omits grammar punctuation and keeps operators and values.' 
                    : 'A complete syntax derivation showing all production rules applied by the compiler grammar.'}
                </p>
              </div>

              {/* Hover node preview box */}
              {hoveredNodeInfo && (
                <div className="bg-indigo-50 dark:bg-indigo-950/30 border border-indigo-100 dark:border-indigo-900 px-3 py-1.5 rounded-xl text-xs font-mono flex items-center gap-2">
                  <span className="font-bold text-indigo-600 dark:text-indigo-400">{hoveredNodeInfo.type}:</span>
                  <span className="font-bold">{hoveredNodeInfo.symbol}</span>
                  {hoveredNodeInfo.evaluated !== null && (
                    <>
                      <ChevronRight className="h-3 w-3 text-slate-400" />
                      <span className="font-bold text-emerald-600 dark:text-emerald-400">eval: {hoveredNodeInfo.evaluated}</span>
                    </>
                  )}
                </div>
              )}
            </div>

            {/* Tree graph rendering container */}
            <div className="flex-1 flex justify-center items-center bg-slate-50 dark:bg-slate-950/60 rounded-xl relative overflow-auto p-4 border border-slate-100 dark:border-slate-900 min-h-[380px]">
              {parseTree ? (
                <svg
                  width="100%"
                  height="100%"
                  viewBox={`0 0 ${svgWidth} ${svgHeight}`}
                  className="max-w-full max-h-[420px]"
                >
                  {/* Render Connections */}
                  {elements
                    .filter(el => el.type === 'connection')
                    .map(conn => (
                      <g key={conn.id}>
                        {/* Shadow line */}
                        <line
                          x1={conn.x1}
                          y1={conn.y1}
                          x2={conn.x2}
                          y2={conn.y2}
                          stroke="#e2e8f0"
                          strokeWidth="5"
                          className="dark:hidden"
                          strokeLinecap="round"
                        />
                        {/* Core line */}
                        <line
                          x1={conn.x1}
                          y1={conn.y1}
                          x2={conn.x2}
                          y2={conn.y2}
                          className="stroke-slate-300 dark:stroke-slate-700 transition-all duration-300"
                          strokeWidth="2.5"
                          strokeLinecap="round"
                        />
                      </g>
                    ))}

                  {/* Render Nodes */}
                  {elements
                    .filter(el => el.type === 'node')
                    .map(el => {
                      const color = getNodeColorClasses(el.nodeType, el.isTerminal);
                      const isOpOrTerminal = el.nodeType === 'operator' || el.isTerminal;
                      
                      return (
                        <g
                          key={el.id}
                          transform={`translate(${el.x}, ${el.y})`}
                          className="cursor-pointer group"
                          onMouseEnter={() => handleNodeMouseEnter(el)}
                          onMouseLeave={() => setHoveredNodeInfo(null)}
                        >
                          {/* Node circle / shape */}
                          <circle
                            r="22"
                            className={`${color.bg} ${color.border} stroke-[2.5] filter drop-shadow-sm group-hover:scale-110 transition-transform duration-200`}
                          />

                          {/* Node Main value text */}
                          <text
                            textAnchor="middle"
                            dy="4"
                            className={`${color.text} font-mono font-bold text-[13px] fill-current pointer-events-none select-none`}
                          >
                            {el.value}
                          </text>

                          {/* Node label preview (hover details) */}
                          <title>
                            {`${color.label}: "${el.value}"`}
                          </title>
                          
                          {/* Tiny description bubble above node */}
                          <text
                            y="-28"
                            textAnchor="middle"
                            className="fill-slate-400 dark:fill-slate-500 font-sans font-bold text-[8px] uppercase tracking-wider pointer-events-none select-none"
                          >
                            {el.nodeType}
                          </text>
                        </g>
                      );
                    })}
                </svg>
              ) : (
                <div className="text-center text-slate-400 py-14">
                  <AlertCircle className="h-10 w-10 mx-auto text-slate-300 dark:text-slate-700 mb-3" />
                  <p className="text-sm font-bold">No valid parse tree to display</p>
                  <p className="text-xs text-slate-400 mt-1">Please fix expression errors above.</p>
                </div>
              )}
            </div>

            {/* Instruction Footer */}
            {treeType === 'AST' && parseTree && (
              <div className="mt-4 p-3 bg-indigo-50/50 dark:bg-indigo-950/20 rounded-xl text-center text-xs text-indigo-700 dark:text-indigo-400 font-medium">
                💡 <span className="font-bold">Pro-tip:</span> Hover over any tree node to see its calculated value under current slider substitutions!
              </div>
            )}
          </div>

          {/* Section: Compiler Compiling Steps (Lexing + Parsing Logs) */}
          <div className="bg-white dark:bg-slate-900 rounded-2xl p-5 shadow-lg border border-slate-100 dark:border-slate-800/80">
            <h3 className="text-sm font-bold uppercase tracking-wider text-slate-400 dark:text-slate-500 mb-3 flex items-center gap-2">
              <Play className="h-4 w-4 text-emerald-500" />
              Parsing Derivation steps & logs
            </h3>

            <div className="bg-slate-50 dark:bg-slate-950 rounded-xl p-4 max-h-[140px] overflow-y-auto font-mono text-xs text-slate-600 dark:text-slate-400 space-y-2 border border-slate-100 dark:border-slate-900">
              {parseSteps.map((step, idx) => (
                <div key={idx} className="flex gap-2">
                  <span className="text-indigo-500 dark:text-indigo-400 font-bold shrink-0">{`[Step ${idx + 1}]`}</span>
                  <span>{step}</span>
                </div>
              ))}
            </div>
          </div>

        </div>

      </div>

      {/* Accordion: Educational Compiler Principles Guide */}
      <div className="bg-white dark:bg-slate-900 rounded-2xl p-6 shadow-lg border border-slate-100 dark:border-slate-800/80 mt-2">
        <h3 className="font-extrabold text-xl mb-4 flex items-center gap-2.5 text-slate-950 dark:text-slate-50">
          <BookOpen className="h-5 w-5 text-indigo-500" />
          Interactive Compiler Theory Guide
        </h3>

        <div className="grid grid-cols-1 lg:grid-cols-4 gap-4">
          <div className="flex flex-col gap-1 border-r border-slate-100 dark:border-slate-800 pr-4">
            <button
              onClick={() => setActiveGuideTab('lexer')}
              className={`text-left px-3 py-2.5 rounded-xl font-bold text-xs flex justify-between items-center transition-all ${
                activeGuideTab === 'lexer'
                  ? 'bg-indigo-50 text-indigo-700 dark:bg-indigo-950/40 dark:text-indigo-300'
                  : 'text-slate-500 hover:bg-slate-50 dark:hover:bg-slate-800/40'
              }`}
            >
              <span>1. Lexical Analysis (Lexing)</span>
              <ChevronRight className="h-3.5 w-3.5" />
            </button>
            <button
              onClick={() => setActiveGuideTab('syntax')}
              className={`text-left px-3 py-2.5 rounded-xl font-bold text-xs flex justify-between items-center transition-all ${
                activeGuideTab === 'syntax'
                  ? 'bg-indigo-50 text-indigo-700 dark:bg-indigo-950/40 dark:text-indigo-300'
                  : 'text-slate-500 hover:bg-slate-50 dark:hover:bg-slate-800/40'
              }`}
            >
              <span>2. Syntactic Analysis (Parsing)</span>
              <ChevronRight className="h-3.5 w-3.5" />
            </button>
            <button
              onClick={() => setActiveGuideTab('cst_vs_ast')}
              className={`text-left px-3 py-2.5 rounded-xl font-bold text-xs flex justify-between items-center transition-all ${
                activeGuideTab === 'cst_vs_ast'
                  ? 'bg-indigo-50 text-indigo-700 dark:bg-indigo-950/40 dark:text-indigo-300'
                  : 'text-slate-500 hover:bg-slate-50 dark:hover:bg-slate-800/40'
              }`}
            >
              <span>3. CST vs. AST Abstraction</span>
              <ChevronRight className="h-3.5 w-3.5" />
            </button>
            <button
              onClick={() => setActiveGuideTab('precedence')}
              className={`text-left px-3 py-2.5 rounded-xl font-bold text-xs flex justify-between items-center transition-all ${
                activeGuideTab === 'precedence'
                  ? 'bg-indigo-50 text-indigo-700 dark:bg-indigo-950/40 dark:text-indigo-300'
                  : 'text-slate-500 hover:bg-slate-50 dark:hover:bg-slate-800/40'
              }`}
            >
              <span>4. Pratt Parsing & Precedence</span>
              <ChevronRight className="h-3.5 w-3.5" />
            </button>
          </div>

          <div className="lg:col-span-3 pl-2 text-sm leading-relaxed text-slate-600 dark:text-slate-400">
            {activeGuideTab === 'lexer' && (
              <div className="space-y-3">
                <h4 className="font-extrabold text-base text-slate-900 dark:text-slate-200">What is a Lexer?</h4>
                <p>
                  The first phase of a compiler is **Lexical Analysis (Scanning)**. It converts a raw stream of source code characters into a linear list of categorized words called **Tokens**.
                </p>
                <p>
                  For example, the expression <code className="bg-slate-100 dark:bg-slate-950 px-1.5 py-0.5 rounded font-mono font-bold text-xs">x + 5.0</code> is split into:
                </p>
                <ul className="list-disc pl-5 space-y-1 text-xs">
                  <li><span className="font-bold text-sky-500">IDENTIFIER</span>: <code className="font-mono">x</code></li>
                  <li><span className="font-bold text-indigo-500">OPERATOR</span>: <code className="font-mono">+</code></li>
                  <li><span className="font-bold text-amber-500">NUMBER</span>: <code className="font-mono">5.0</code></li>
                </ul>
                <p>
                  Lexers use regular expressions and state machines to ignore whitespace and comments, and immediately flag invalid characters.
                </p>
              </div>
            )}

            {activeGuideTab === 'syntax' && (
              <div className="space-y-3">
                <h4 className="font-extrabold text-base text-slate-900 dark:text-slate-200">Syntactic Analysis (The Parser)</h4>
                <p>
                  The **Parser** consumes the token list and organizes them into a hierarchical node structure called a **Parse Tree** based on formal rules named a **Context-Free Grammar (CFG)**.
                </p>
                <p>
                  A CFG declares the structure of expressions recursively, for example:
                  <code className="block bg-slate-100 dark:bg-slate-950 p-2 rounded font-mono text-xs mt-1.5 leading-relaxed">
                    E -> T E'<br />
                    E' -> + T E' | ε
                  </code>
                  This rewritten right-recursive grammar resolves left-recursion, allowing simple top-down **Recursive Descent Parsing** where every grammar rule corresponds to a parsing function in our parser class.
                </p>
              </div>
            )}

            {activeGuideTab === 'cst_vs_ast' && (
              <div className="space-y-3">
                <h4 className="font-extrabold text-base text-slate-900 dark:text-slate-200">Concrete Syntax Trees (CST) vs. AST</h4>
                <p>
                  Understanding the difference between CSTs and ASTs is a milestone in learning compilers:
                </p>
                <div className="grid grid-cols-1 md:grid-cols-2 gap-4 my-2 text-xs">
                  <div className="bg-slate-50 dark:bg-slate-950/40 p-3 rounded-xl border border-slate-100 dark:border-slate-800">
                    <h5 className="font-extrabold text-indigo-600 dark:text-indigo-400 mb-1">CST (Concrete Syntax Tree)</h5>
                    <p>Also known as a **Derivation Tree**. It matches the grammar rules exactly. Includes details like parentheses, semicolons, and epsilon (empty string ε) nodes. Used heavily for full syntactic verification.</p>
                  </div>
                  <div className="bg-slate-50 dark:bg-slate-950/40 p-3 rounded-xl border border-slate-100 dark:border-slate-800">
                    <h5 className="font-extrabold text-emerald-600 dark:text-emerald-400 mb-1">AST (Abstract Syntax Tree)</h5>
                    <p>A simplified tree mapping the semantic operation structure. Punctuation (like parens) is discarded, and operators become actual root nodes with their operands as children. ASTs are used in evaluation, optimization, and code generation.</p>
                  </div>
                </div>
              </div>
            )}

            {activeGuideTab === 'precedence' && (
              <div className="space-y-3">
                <h4 className="font-extrabold text-base text-slate-900 dark:text-slate-200">Operator Precedence & Pratt Parsing</h4>
                <p>
                  Computers read tokens linearly, but algebra requires mathematical operator priorities. For example, in <code className="bg-slate-100 dark:bg-slate-950 px-1 rounded font-mono text-xs">3 + 4 * 5</code>, multiplication must occur before addition.
                </p>
                <p>
                  Compilers achieve this by defining priority numbers for operators:
                </p>
                <ul className="list-disc pl-5 space-y-1 text-xs font-mono">
                  <li>Priority 1: <code className="font-bold">+</code> and <code className="font-bold">-</code></li>
                  <li>Priority 2: <code className="font-bold">*</code>, <code className="font-bold">/</code> and <code className="font-bold">%</code></li>
                  <li>Priority 3: <code className="font-bold">^</code> (Power operator)</li>
                </ul>
                <p>
                  Precedence climbing parsers compare the current operator precedence to bind elements. Since multiplication has a higher precedence (2) than addition (1), the subtree for <code className="bg-slate-100 dark:bg-slate-950 px-1 rounded font-mono text-xs">4 * 5</code> is built first, and then correctly attached as the right child of the <code className="bg-slate-100 dark:bg-slate-950 px-1 rounded font-mono text-xs">+</code> operator node.
                </p>
              </div>
            )}
          </div>
        </div>
      </div>

    </div>
  );
};

export default ExpressionParser;
