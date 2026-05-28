// CFG Grammar Parser and Analysis in JavaScript

export class CFGGrammar {
  constructor() {
    this.productions = [];
    this.nonTerminals = new Set();
    this.terminals = new Set();
    this.startSymbol = null;
    this.errors = [];
    this.warnings = [];
  }

  parseGrammar(grammarString) {
    this.clear();
    const lines = grammarString.split('\n').filter(line => line.trim() && !line.trim().startsWith('#'));
    
    for (const line of lines) {
      this.parseRule(line.trim());
    }
    
    if (this.productions.length > 0 && !this.startSymbol) {
      this.startSymbol = this.productions[0].lhs;
    }
    
    this.detectSymbols();
    this.validateGrammar();
    
    return this.errors.length === 0;
  }

  parseRule(ruleString) {
    const arrowMatch = ruleString.match(/^(.+?)\s*->\s*(.+)$/);
    if (!arrowMatch) {
      this.errors.push(`Invalid rule format: ${ruleString}`);
      return;
    }

    const lhs = arrowMatch[1].trim();
    const rhsAlternatives = arrowMatch[2].split('|').map(alt => alt.trim());

    for (const rhs of rhsAlternatives) {
      const symbols = rhs.split(/\s+/).filter(s => s);
      this.productions.push({
        lhs,
        rhs: symbols,
        original: `${lhs} -> ${rhs}`
      });
    }
  }

  detectSymbols() {
    this.nonTerminals.clear();
    this.terminals.clear();

    for (const prod of this.productions) {
      this.nonTerminals.add(prod.lhs);
      for (const symbol of prod.rhs) {
        if (symbol === 'ε' || symbol === 'epsilon') {
          continue;
        }
        if (this.isNonTerminal(symbol)) {
          this.nonTerminals.add(symbol);
        } else {
          this.terminals.add(symbol);
        }
      }
    }
  }

  isNonTerminal(symbol) {
    return /^[A-Z]/.test(symbol) || symbol.length > 1;
  }

  validateGrammar() {
    if (!this.startSymbol) {
      this.errors.push('No start symbol defined');
    }

    if (this.productions.length === 0) {
      this.errors.push('No productions defined');
    }

    // Check for left recursion
    for (const prod of this.productions) {
      if (prod.rhs.length > 0 && prod.rhs[0] === prod.lhs) {
        this.warnings.push(`Left recursion detected: ${prod.original}`);
      }
    }
  }

  clear() {
    this.productions = [];
    this.nonTerminals.clear();
    this.terminals.clear();
    this.startSymbol = null;
    this.errors = [];
    this.warnings = [];
  }

  getProductionsForSymbol(symbol) {
    return this.productions.filter(p => p.lhs === symbol);
  }
}

export class FirstFollowSets {
  constructor(grammar) {
    this.grammar = grammar;
    this.firstSets = new Map();
    this.followSets = new Map();
  }

  computeFirst() {
    this.firstSets.clear();
    
    // Initialize terminals
    for (const terminal of this.grammar.terminals) {
      this.firstSets.set(terminal, new Set([terminal]));
    }
    
    // Initialize non-terminals
    for (const nt of this.grammar.nonTerminals) {
      this.firstSets.set(nt, new Set());
    }
    
    // Iteratively compute First sets
    let changed = true;
    while (changed) {
      changed = false;
      
      for (const nt of this.grammar.nonTerminals) {
        const originalFirst = new Set(this.firstSets.get(nt));
        this.computeFirstForSymbol(nt);
        
        if (!this.setsEqual(originalFirst, this.firstSets.get(nt))) {
          changed = true;
        }
      }
    }
  }

  computeFirstForSymbol(symbol) {
    if (this.grammar.terminals.has(symbol)) {
      this.firstSets.set(symbol, new Set([symbol]));
      return;
    }

    const productions = this.grammar.getProductionsForSymbol(symbol);
    for (const prod of productions) {
      let allHaveEpsilon = true;
      
      for (const rhsSymbol of prod.rhs) {
        if (rhsSymbol === 'ε' || rhsSymbol === 'epsilon') {
          this.firstSets.get(symbol).add('ε');
          break;
        }

        const firstOfRhs = this.firstSets.get(rhsSymbol) || new Set();
        
        for (const fs of firstOfRhs) {
          if (fs !== 'ε') {
            this.firstSets.get(symbol).add(fs);
          }
        }

        if (!firstOfRhs.has('ε')) {
          allHaveEpsilon = false;
          break;
        }
      }

      if (allHaveEpsilon) {
        this.firstSets.get(symbol).add('ε');
      }
    }
  }

  computeFollow() {
    this.followSets.clear();
    
    // Initialize all non-terminals with empty sets
    for (const nt of this.grammar.nonTerminals) {
      this.followSets.set(nt, new Set());
    }
    
    // Add $ to follow set of start symbol
    this.followSets.get(this.grammar.startSymbol).add('$');
    
    // Iteratively compute Follow sets
    let changed = true;
    while (changed) {
      changed = false;
      
      for (const prod of this.grammar.productions) {
        for (let i = 0; i < prod.rhs.length; i++) {
          const B = prod.rhs[i];
          
          if (!this.grammar.nonTerminals.has(B)) continue;
          
          const beta = prod.rhs.slice(i + 1);
          const firstBeta = this.computeFirstOfString(beta);
          
          // Add First(beta) - {ε} to Follow(B)
          for (const fb of firstBeta) {
            if (fb !== 'ε') {
              if (this.followSets.get(B).add(fb)) {
                changed = true;
              }
            }
          }
          
          // If ε in First(beta), add Follow(A) to Follow(B)
          if (firstBeta.has('ε')) {
            const followA = this.followSets.get(prod.lhs);
            for (const fa of followA) {
              if (this.followSets.get(B).add(fa)) {
                changed = true;
              }
            }
          }
        }
      }
    }
  }

  computeFirstOfString(symbols) {
    const result = new Set();
    let allHaveEpsilon = true;
    
    for (const symbol of symbols) {
      const firstOfSymbol = this.firstSets.get(symbol) || new Set();
      
      for (const fs of firstOfSymbol) {
        if (fs !== 'ε') {
          result.add(fs);
        }
      }
      
      if (!firstOfSymbol.has('ε')) {
        allHaveEpsilon = false;
        break;
      }
    }
    
    if (allHaveEpsilon) {
      result.add('ε');
    }
    
    return result;
  }

  computeAll() {
    this.computeFirst();
    this.computeFollow();
  }

  setsEqual(set1, set2) {
    if (set1.size !== set2.size) return false;
    for (const item of set1) {
      if (!set2.has(item)) return false;
    }
    return true;
  }

  isLL1() {
    return this.getLL1Conflicts().length === 0;
  }

  getLL1Conflicts() {
    const conflicts = [];
    
    for (const nt of this.grammar.nonTerminals) {
      const productions = this.grammar.getProductionsForSymbol(nt);
      const seenTerminals = new Set();
      
      for (const prod of productions) {
        const firstOfRhs = this.computeFirstOfString(prod.rhs);
        
        for (const ft of firstOfRhs) {
          if (ft !== 'ε' && seenTerminals.has(ft)) {
            conflicts.push(`First/First conflict for ${nt} on terminal ${ft}`);
          }
          if (ft !== 'ε') {
            seenTerminals.add(ft);
          }
        }
        
        if (firstOfRhs.has('ε')) {
          const followOfNt = this.followSets.get(nt);
          for (const ft of followOfNt) {
            if (seenTerminals.has(ft)) {
              conflicts.push(`First/Follow conflict for ${nt} on terminal ${ft}`);
            }
            seenTerminals.add(ft);
          }
        }
      }
    }
    
    return conflicts;
  }
}

export class LL1ParserTable {
  constructor(grammar, firstFollow) {
    this.grammar = grammar;
    this.firstFollow = firstFollow;
    this.table = new Map();
    this.conflicts = [];
  }

  build() {
    this.table.clear();
    this.conflicts = [];
    
    const terminals = [...this.grammar.terminals, '$'];
    
    for (const nt of this.grammar.nonTerminals) {
      this.table.set(nt, new Map());
    }
    
    for (const prod of this.grammar.productions) {
      const firstOfRhs = this.firstFollow.computeFirstOfString(prod.rhs);
      
      for (const terminal of firstOfRhs) {
        if (terminal !== 'ε') {
          this.setEntry(prod.lhs, terminal, prod);
        }
      }
      
      if (firstOfRhs.has('ε')) {
        const followOfLhs = this.firstFollow.followSets.get(prod.lhs);
        for (const terminal of followOfLhs) {
          this.setEntry(prod.lhs, terminal, prod);
        }
      }
    }
    
    return this.conflicts.length === 0;
  }

  setEntry(nonTerminal, terminal, production) {
    const row = this.table.get(nonTerminal);
    if (row.has(terminal)) {
      const existing = row.get(terminal);
      if (existing.original !== production.original) {
        this.conflicts.push(`Conflict at [${nonTerminal}, ${terminal}]: ${existing.original} vs ${production.original}`);
      }
    } else {
      row.set(terminal, production);
    }
  }

  getEntry(nonTerminal, terminal) {
    const row = this.table.get(nonTerminal);
    return row ? row.get(terminal) : null;
  }

  isGrammarLL1() {
    return this.conflicts.length === 0;
  }
}

export class DynamicParser {
  constructor(grammar, parserTable) {
    this.grammar = grammar;
    this.parserTable = parserTable;
    this.stack = [];
    this.input = [];
    this.inputPosition = 0;
    this.parseTree = null;
    this.parsingSteps = [];
    this.errors = [];
    this.success = false;
  }

  parse(inputString) {
    this.reset();
    this.input = this.tokenizeInput(inputString);
    this.inputPosition = 0;
    
    // Initialize stack
    this.stack = ['$'];
    this.stack.push(this.grammar.startSymbol);
    
    // Initialize parse tree
    this.parseTree = {
      symbol: this.grammar.startSymbol,
      isNonTerminal: true,
      children: [],
      ruleNumber: -1
    };
    
    this.recordStep('Initialized parsing');
    
    // Main parsing loop
    while (this.stack.length > 0 && this.inputPosition < this.input.length) {
      if (!this.parseStep()) {
        this.success = false;
        return false;
      }
    }
    
    this.success = this.stack.length === 0 && this.inputPosition === this.input.length;
    
    if (!this.success) {
      if (this.stack.length > 0) {
        this.errors.push('Parsing failed: stack not empty at end');
      }
      if (this.inputPosition < this.input.length) {
        this.errors.push('Parsing failed: input not fully consumed');
      }
    }
    
    return this.success;
  }

  parseStep() {
    const stackTop = this.stack[this.stack.length - 1];
    const currentInput = this.inputPosition < this.input.length ? this.input[this.inputPosition] : '$';
    
    this.recordStep(`Stack top: ${stackTop}, Input: ${currentInput}`);
    
    if (this.grammar.terminals.has(stackTop) || stackTop === '$') {
      if (stackTop === currentInput) {
        this.stack.pop();
        this.inputPosition++;
        this.recordStep(`Matched terminal: ${stackTop}`);
        return true;
      } else {
        this.errors.push(`Mismatch: expected '${stackTop}', got '${currentInput}'`);
        return false;
      }
    }
    
    if (this.grammar.nonTerminals.has(stackTop)) {
      const production = this.parserTable.getEntry(stackTop, currentInput);
      
      if (!production) {
        this.errors.push(`No entry in parsing table for [${stackTop}, ${currentInput}]`);
        return false;
      }
      
      this.stack.pop();
      
      // Push RHS symbols in reverse order
      for (let i = production.rhs.length - 1; i >= 0; i--) {
        if (production.rhs[i] !== 'ε' && production.rhs[i] !== 'epsilon') {
          this.stack.push(production.rhs[i]);
        }
      }
      
      this.recordStep(`Applied rule: ${production.original}`);
      this.buildParseTree(stackTop, production);
      
      return true;
    }
    
    this.errors.push(`Unknown symbol on stack: ${stackTop}`);
    return false;
  }

  tokenizeInput(inputString) {
    const tokens = inputString.trim().split(/\s+/).filter(t => t);
    tokens.push('$');
    return tokens;
  }

  recordStep(step) {
    this.parsingSteps.push({
      step: this.parsingSteps.length + 1,
      stack: [...this.stack],
      input: this.input.slice(this.inputPosition),
      action: step
    });
  }

  buildParseTree(symbol, production) {
    const findAndExpand = (node) => {
      if (node.symbol === symbol && node.children.length === 0) {
        for (const rhsSymbol of production.rhs) {
          if (rhsSymbol !== 'ε' && rhsSymbol !== 'epsilon') {
            node.children.push({
              symbol: rhsSymbol,
              isNonTerminal: this.grammar.nonTerminals.has(rhsSymbol),
              children: [],
              ruleNumber: this.grammar.productions.indexOf(production)
            });
          }
        }
        return true;
      }
      
      for (const child of node.children) {
        if (findAndExpand(child)) return true;
      }
      return false;
    };
    
    findAndExpand(this.parseTree);
  }

  reset() {
    this.stack = [];
    this.input = [];
    this.inputPosition = 0;
    this.parseTree = null;
    this.parsingSteps = [];
    this.errors = [];
    this.success = false;
  }
}
