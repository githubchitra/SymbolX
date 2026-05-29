// CFG Grammar Parser and General-Purpose Earley Parser in JavaScript
// Supports any Context-Free Grammar, including left-recursive and ambiguous grammars.

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

    // First collect all LHS symbols as non-terminals
    for (const prod of this.productions) {
      this.nonTerminals.add(prod.lhs);
    }

    // Then collect RHS symbols that are not non-terminals or epsilon as terminals
    for (const prod of this.productions) {
      for (const symbol of prod.rhs) {
        if (symbol === 'ε' || symbol === 'epsilon') {
          continue;
        }
        if (!this.nonTerminals.has(symbol)) {
          this.terminals.add(symbol);
        }
      }
    }
  }

  isNonTerminal(symbol) {
    return this.nonTerminals.has(symbol);
  }

  validateGrammar() {
    if (!this.startSymbol) {
      this.errors.push('No start symbol defined');
    }

    if (this.productions.length === 0) {
      this.errors.push('No productions defined');
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

// Earley Parser: parses any general CFG, handles left recursion seamlessly
export class EarleyParser {
  constructor(grammar) {
    this.grammar = grammar;
    this.errors = [];
  }

  tokenizeInput(inputString) {
    const symbols = [...this.grammar.terminals, ...this.grammar.nonTerminals];
    symbols.sort((a, b) => b.length - a.length);
    const escapedSymbols = symbols.map(s => s.replace(/[-/\\^$*+?.()|[\]{}]/g, '\\$&'));
    const pattern = new RegExp(
      `\\s+|(${escapedSymbols.join('|')}|\\w+|.)`,
      'g'
    );
    
    const tokens = [];
    let match;
    while ((match = pattern.exec(inputString)) !== null) {
      const token = match[1];
      if (token && token.trim()) {
        tokens.push(token);
      }
    }
    return tokens;
  }

  parse(inputTokens) {
    this.errors = [];
    const n = inputTokens.length;
    const S = Array.from({ length: n + 1 }, () => []);
    
    const startSymbol = this.grammar.startSymbol;
    if (!startSymbol) {
      this.errors.push('No start symbol found in grammar.');
      return null;
    }

    // Helper to uniquely add items to a state set
    const addUnique = (setIdx, item) => {
      const set = S[setIdx];
      const exists = set.some(x => 
        x.lhs === item.lhs && 
        x.rhs.join(' ') === item.rhs.join(' ') && 
        x.dot === item.dot && 
        x.start === item.start
      );
      if (!exists) {
        set.push(item);
        return true;
      }
      return false;
    };
    
    // Initial state: add start rule transition
    addUnique(0, { lhs: '$start', rhs: [startSymbol], dot: 0, start: 0, children: [] });
    
    for (let i = 0; i <= n; i++) {
      let j = 0;
      while (j < S[i].length) {
        const item = S[i][j];
        
        if (item.dot < item.rhs.length) {
          // Dot is not at the end of the rule: Predict or Scan
          const nextSymbol = item.rhs[item.dot];
          
          if (this.grammar.nonTerminals.has(nextSymbol)) {
            // Next symbol is Non-Terminal -> PREDICT
            const productions = this.grammar.getProductionsForSymbol(nextSymbol);
            for (const prod of productions) {
              const isEpsilon = prod.rhs.length === 0 || 
                                (prod.rhs.length === 1 && (prod.rhs[0] === 'ε' || prod.rhs[0] === 'epsilon'));
              if (isEpsilon) {
                // Epsilon transitions: immediately complete and record
                addUnique(i, {
                  lhs: nextSymbol,
                  rhs: prod.rhs,
                  dot: prod.rhs.length,
                  start: i,
                  children: [{ symbol: prod.rhs[0] || 'ε', isNonTerminal: false, children: [] }]
                });
              } else {
                addUnique(i, {
                  lhs: nextSymbol,
                  rhs: prod.rhs,
                  dot: 0,
                  start: i,
                  children: []
                });
              }
            }
          } else {
            // Next symbol is Terminal -> SCAN
            if (i < n && nextSymbol === inputTokens[i]) {
              addUnique(i + 1, {
                lhs: item.lhs,
                rhs: item.rhs,
                dot: item.dot + 1,
                start: item.start,
                children: [...item.children, { symbol: nextSymbol, isNonTerminal: false, children: [] }]
              });
            }
          }
        } else {
          // Dot is at the end of the rule: COMPLETE
          const origin = item.start;
          for (let k = 0; k < S[origin].length; k++) {
            const prevItem = S[origin][k];
            if (prevItem.dot < prevItem.rhs.length && prevItem.rhs[prevItem.dot] === item.lhs) {
              const node = {
                symbol: item.lhs,
                isNonTerminal: true,
                children: item.children
              };
              addUnique(i, {
                lhs: prevItem.lhs,
                rhs: prevItem.rhs,
                dot: prevItem.dot + 1,
                start: prevItem.start,
                children: [...prevItem.children, node]
              });
            }
          }
        }
        j++;
      }
    }
    
    // Find accepting item ($start -> startSymbol ., origin=0) in final state set S[n]
    const finalItem = S[n].find(x => x.lhs === '$start' && x.dot === 1 && x.start === 0);
    if (finalItem && finalItem.children && finalItem.children.length > 0) {
      return finalItem.children[0];
    } else {
      this.errors.push(`Parsing failed: "${inputTokens.join(' ')}" does not conform to the grammar syntax rules.`);
      return null;
    }
  }
}
