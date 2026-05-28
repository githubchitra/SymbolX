import React, { useMemo, useState } from 'react';
import { Play, RotateCcw } from 'lucide-react';

const DEFAULT_GRAMMAR = `E -> E + T | T
T -> T * F | F
F -> ( E ) | id`;

const DEFAULT_INPUT = 'id + id * id';
const EPSILON_SYMBOLS = new Set(['epsilon', 'eps', 'e', '\u03b5', '\u03bb']);

const splitWordByNonTerminals = (word, nonTerminals) => {
  const orderedNonTerminals = [...nonTerminals].sort((a, b) => b.length - a.length);
  const symbols = [];
  let cursor = 0;

  while (cursor < word.length) {
    let nonTerminal = '';
    for (const item of orderedNonTerminals) {
      if (word.startsWith(item, cursor)) {
        nonTerminal = item;
        break;
      }
    }

    if (nonTerminal) {
      symbols.push(nonTerminal);
      cursor += nonTerminal.length;
      continue;
    }

    let nextNonTerminalIndex = -1;
    for (let index = cursor + 1; index < word.length; index += 1) {
      if (orderedNonTerminals.some((item) => word.startsWith(item, index))) {
        nextNonTerminalIndex = index;
        break;
      }
    }

    if (nextNonTerminalIndex === -1) {
      symbols.push(word.slice(cursor));
      break;
    }

    symbols.push(word.slice(cursor, nextNonTerminalIndex));
    cursor = nextNonTerminalIndex;
  }

  return symbols.filter(Boolean);
};

const tokenizeProductionAlternative = (alternative, nonTerminals) => {
  const trimmed = alternative.trim();

  if (!trimmed || EPSILON_SYMBOLS.has(trimmed.toLowerCase())) {
    return [];
  }

  if (/\s/.test(trimmed)) {
    const symbols = trimmed.split(/\s+/).filter(Boolean);
    return symbols.length === 1 && EPSILON_SYMBOLS.has(symbols[0].toLowerCase()) ? [] : symbols;
  }

  const symbols = [];
  let cursor = 0;

  while (cursor < trimmed.length) {
    const char = trimmed[cursor];

    if (char === '"' || char === "'") {
      const end = trimmed.indexOf(char, cursor + 1);
      if (end === -1) {
        throw new Error(`Unclosed quoted terminal in: ${alternative}`);
      }
      symbols.push(trimmed.slice(cursor + 1, end));
      cursor = end + 1;
      continue;
    }

    if (/[A-Za-z0-9_]/.test(char)) {
      let end = cursor + 1;
      while (end < trimmed.length && /[A-Za-z0-9_]/.test(trimmed[end])) {
        end += 1;
      }
      symbols.push(...splitWordByNonTerminals(trimmed.slice(cursor, end), nonTerminals));
      cursor = end;
      continue;
    }

    symbols.push(char);
    cursor += 1;
  }

  return symbols;
};

const parseGrammar = (source) => {
  const productions = [];
  const nonTerminals = new Set();
  const lines = source
    .split(/\r?\n/)
    .map((line) => line.trim())
    .filter(Boolean);

  lines.forEach((line) => {
    const match = line.match(/^(.+?)\s*(?:->|\u2192)\s*(.+)$/);
    if (!match) {
      throw new Error(`Invalid production: ${line}`);
    }

    nonTerminals.add(match[1].trim());
  });

  lines.forEach((line) => {
    const [, lhsSource, rhsSource] = line.match(/^(.+?)\s*(?:->|\u2192)\s*(.+)$/);
    const lhs = lhsSource.trim();

    rhsSource.split('|').forEach((alternative) => {
      productions.push({ lhs, rhs: tokenizeProductionAlternative(alternative, nonTerminals) });
    });
  });

  if (productions.length === 0) {
    throw new Error('Add at least one grammar production.');
  }

  const terminals = new Set();
  productions.forEach((production) => {
    production.rhs.forEach((symbol) => {
      if (!nonTerminals.has(symbol)) {
        terminals.add(symbol);
      }
    });
  });

  return {
    startSymbol: productions[0].lhs,
    productions,
    nonTerminals,
    terminals,
  };
};

const tokenizeInput = (source, terminals) => {
  const trimmed = source.trim();
  if (!trimmed) {
    return [];
  }

  if (/\s/.test(trimmed)) {
    return trimmed.split(/\s+/).filter(Boolean);
  }

  const orderedTerminals = [...terminals].sort((a, b) => b.length - a.length);
  const tokens = [];
  let cursor = 0;

  while (cursor < trimmed.length) {
    let terminal = '';
    for (const item of orderedTerminals) {
      if (trimmed.startsWith(item, cursor)) {
        terminal = item;
        break;
      }
    }

    if (!terminal) {
      throw new Error(`Could not tokenize input near "${trimmed.slice(cursor)}". Add spaces between tokens if needed.`);
    }

    tokens.push(terminal);
    cursor += terminal.length;
  }

  return tokens;
};

const stateKey = (state) => `${state.lhs}\u0001${state.rhs.join('\u0002')}\u0001${state.dot}\u0001${state.origin}`;

const parseWithEarley = (grammar, tokens) => {
  const augmentedStart = '__START__';
  const allProductions = [{ lhs: augmentedStart, rhs: [grammar.startSymbol] }, ...grammar.productions];
  const byLhs = new Map();

  allProductions.forEach((production) => {
    if (!byLhs.has(production.lhs)) {
      byLhs.set(production.lhs, []);
    }
    byLhs.get(production.lhs).push(production);
  });

  const chart = Array.from({ length: tokens.length + 1 }, () => ({ states: [], keys: new Set() }));
  const addState = (position, state) => {
    const key = stateKey(state);
    if (chart[position].keys.has(key)) {
      return false;
    }

    chart[position].keys.add(key);
    chart[position].states.push(state);
    return true;
  };

  addState(0, {
    lhs: augmentedStart,
    rhs: [grammar.startSymbol],
    dot: 0,
    origin: 0,
    children: [],
  });

  for (let position = 0; position < chart.length; position += 1) {
    for (let cursor = 0; cursor < chart[position].states.length; cursor += 1) {
      const state = chart[position].states[cursor];
      const nextSymbol = state.rhs[state.dot];

      if (nextSymbol) {
        if (grammar.nonTerminals.has(nextSymbol)) {
          (byLhs.get(nextSymbol) || []).forEach((production) => {
            addState(position, {
              lhs: production.lhs,
              rhs: production.rhs,
              dot: 0,
              origin: position,
              children: [],
            });
          });
        } else if (tokens[position] === nextSymbol) {
          addState(position + 1, {
            ...state,
            dot: state.dot + 1,
            children: [...state.children, { label: nextSymbol }],
          });
        }
      } else {
        const completedTree = {
          label: state.lhs,
          children: state.rhs.length === 0 ? [{ label: 'epsilon' }] : state.children,
        };

        chart[state.origin].states.forEach((previousState) => {
          if (previousState.rhs[previousState.dot] === state.lhs) {
            addState(position, {
              ...previousState,
              dot: previousState.dot + 1,
              children: [...previousState.children, completedTree],
            });
          }
        });
      }
    }
  }

  const finalState = chart[tokens.length].states.find(
    (state) => state.lhs === augmentedStart && state.dot === 1 && state.origin === 0
  );

  if (!finalState) {
    throw new Error(`Input was not accepted by this grammar. Tokens read: ${tokens.join(' ') || '(empty)'}`);
  }

  return finalState.children[0];
};

const generateDerivation = (tree, nonTerminals) => {
  const frontier = [{ label: tree.label, node: tree }];
  const steps = [frontier.map((item) => item.label).join(' ')];

  while (frontier.some((item) => nonTerminals.has(item.label) && item.node.children)) {
    const targetIndex = frontier.findIndex((item) => nonTerminals.has(item.label) && item.node.children);
    const target = frontier[targetIndex];
    const replacement = target.node.children.map((child) => ({ label: child.label, node: child }));
    frontier.splice(targetIndex, 1, ...replacement);
    steps.push(frontier.map((item) => item.label).join(' '));
  }

  return steps;
};

const calculateWidths = (node) => {
  if (!node.children || node.children.length === 0) {
    return { ...node, width: Math.max(node.label.length, 2) };
  }

  const children = node.children.map(calculateWidths);
  const width = Math.max(
    node.label.length,
    children.reduce((sum, child) => sum + child.width, 0) + Math.max(children.length - 1, 0) * 3
  );

  return { ...node, children, width };
};

const renderTreeLines = (tree) => {
  const measuredTree = calculateWidths(tree);
  const levels = [];
  const connectors = [];

  const visit = (node, left, depth) => {
    const center = left + Math.floor(node.width / 2);
    levels[depth] = levels[depth] || [];
    levels[depth].push({ label: node.label, center });

    if (!node.children || node.children.length === 0) {
      return;
    }

    let childLeft = left;
    node.children.forEach((child) => {
      const childCenter = childLeft + Math.floor(child.width / 2);
      connectors[depth] = connectors[depth] || [];
      connectors[depth].push({
        center: childCenter,
        char: childCenter < center ? '/' : childCenter > center ? '\\' : '|',
      });
      visit(child, childLeft, depth + 1);
      childLeft += child.width + 3;
    });
  };

  visit(measuredTree, 0, 0);

  return levels.flatMap((nodes, depth) => {
    let line = '';
    nodes.forEach(({ label, center }) => {
      const start = Math.max(center - Math.floor(label.length / 2), line.length);
      line += ' '.repeat(start - line.length) + label;
    });
    const lines = [line.trimEnd()];

    if (connectors[depth]) {
      let connectorLine = '';
      connectors[depth].forEach(({ char, center }) => {
        connectorLine += ' '.repeat(Math.max(center - connectorLine.length, 0)) + char;
      });
      lines.push(connectorLine.trimEnd());
    }

    return lines;
  });
};

const ParsingTab = () => {
  const [grammar, setGrammar] = useState(DEFAULT_GRAMMAR);
  const [inputString, setInputString] = useState(DEFAULT_INPUT);
  const [result, setResult] = useState(null);

  const grammarSummary = useMemo(() => {
    try {
      const parsedGrammar = parseGrammar(grammar);
      return {
        error: '',
        startSymbol: parsedGrammar.startSymbol,
        terminals: [...parsedGrammar.terminals].join(', ') || '(none)',
      };
    } catch (error) {
      return { error: error.message };
    }
  }, [grammar]);

  const handleBuild = () => {
    try {
      const parsedGrammar = parseGrammar(grammar);
      const tokens = tokenizeInput(inputString, parsedGrammar.terminals);
      const parseTree = parseWithEarley(parsedGrammar, tokens);

      setResult({
        derivation: generateDerivation(parseTree, parsedGrammar.nonTerminals),
        treeLines: renderTreeLines(parseTree),
        tokens,
      });
    } catch (error) {
      setResult({ error: error.message });
    }
  };

  const handleReset = () => {
    setGrammar(DEFAULT_GRAMMAR);
    setInputString(DEFAULT_INPUT);
    setResult(null);
  };

  return (
    <div className="space-y-6">
      <div className="bg-white dark:bg-gray-800 rounded-lg shadow-lg border border-gray-100 dark:border-gray-700 p-6">
        <div className="flex flex-wrap items-center justify-between gap-3 mb-4">
          <h2 className="text-xl font-bold text-gray-900 dark:text-white">Parsing</h2>
          <div className="flex gap-2">
            <button
              onClick={handleReset}
              className="px-3 py-2 rounded-lg border border-gray-200 dark:border-gray-600 text-gray-700 dark:text-gray-200 hover:bg-gray-50 dark:hover:bg-gray-700 transition-colors flex items-center gap-2 text-sm font-bold"
            >
              <RotateCcw className="h-4 w-4" />
              Reset
            </button>
            <button
              onClick={handleBuild}
              className="px-4 py-2 bg-primary-600 text-white rounded-lg hover:bg-primary-700 transition-colors flex items-center gap-2 text-sm font-bold"
            >
              <Play className="h-4 w-4" />
              Build Parse Tree
            </button>
          </div>
        </div>

        <div className="grid grid-cols-1 xl:grid-cols-2 gap-5">
          <label className="block">
            <span className="block text-sm font-bold text-gray-700 dark:text-gray-300 mb-2">Grammar</span>
            <textarea
              value={grammar}
              onChange={(event) => setGrammar(event.target.value)}
              className="w-full h-40 px-4 py-3 rounded-lg border border-gray-300 dark:border-gray-600 bg-white dark:bg-gray-900 text-gray-900 dark:text-white font-mono text-sm focus:ring-2 focus:ring-primary-500 focus:border-transparent resize-none"
              spellCheck="false"
            />
          </label>

          <label className="block">
            <span className="block text-sm font-bold text-gray-700 dark:text-gray-300 mb-2">Input String</span>
            <input
              value={inputString}
              onChange={(event) => setInputString(event.target.value)}
              className="w-full px-4 py-3 rounded-lg border border-gray-300 dark:border-gray-600 bg-white dark:bg-gray-900 text-gray-900 dark:text-white font-mono text-sm focus:ring-2 focus:ring-primary-500 focus:border-transparent"
              spellCheck="false"
            />
          </label>
        </div>

        {grammarSummary.error ? (
          <div className="mt-4 p-3 rounded-lg bg-red-50 dark:bg-red-900/20 border border-red-200 dark:border-red-800 text-sm text-red-800 dark:text-red-200">
            {grammarSummary.error}
          </div>
        ) : (
          <div className="mt-4 flex flex-wrap gap-2 text-xs font-bold text-gray-600 dark:text-gray-300">
            <span className="px-2 py-1 rounded bg-gray-100 dark:bg-gray-700">Start: {grammarSummary.startSymbol}</span>
            <span className="px-2 py-1 rounded bg-gray-100 dark:bg-gray-700">Terminals: {grammarSummary.terminals}</span>
          </div>
        )}
      </div>

      {result?.error && (
        <div className="bg-red-50 dark:bg-red-900/20 border border-red-200 dark:border-red-800 rounded-lg p-4 text-red-800 dark:text-red-200 font-semibold">
          {result.error}
        </div>
      )}

      {result?.derivation && (
        <div className="grid grid-cols-1 xl:grid-cols-2 gap-6">
          <section className="bg-white dark:bg-gray-800 rounded-lg shadow-lg border border-gray-100 dark:border-gray-700 p-6">
            <div className="flex flex-wrap items-center justify-between gap-2 mb-4">
              <h3 className="text-lg font-bold text-gray-900 dark:text-white">Leftmost Derivation</h3>
              <span className="text-xs font-bold text-gray-500 dark:text-gray-400">
                Tokens: {result.tokens.join(' ') || '(empty)'}
              </span>
            </div>
            <div className="bg-gray-50 dark:bg-gray-900 rounded-lg border border-gray-100 dark:border-gray-700 p-5 overflow-x-auto">
              <div className="space-y-2 font-mono text-sm text-gray-800 dark:text-gray-200 whitespace-nowrap">
                {result.derivation.map((step, index) => (
                  <div key={`${step}-${index}`}>{index === 0 ? step : `-> ${step}`}</div>
                ))}
              </div>
            </div>
          </section>

          <section className="bg-white dark:bg-gray-800 rounded-lg shadow-lg border border-gray-100 dark:border-gray-700 p-6">
            <h3 className="text-lg font-bold text-gray-900 dark:text-white mb-4">Parse Tree</h3>
            <div className="bg-gray-50 dark:bg-gray-900 rounded-lg border border-gray-100 dark:border-gray-700 p-5 overflow-x-auto">
              <pre className="font-mono text-sm leading-7 text-gray-800 dark:text-gray-200 whitespace-pre">{result.treeLines.join('\n')}</pre>
            </div>
          </section>
        </div>
      )}
    </div>
  );
};

export default ParsingTab;
