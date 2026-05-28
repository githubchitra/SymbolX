import React, { useState } from 'react';
import { Play, Download, Upload, FileText, Settings, ChevronRight } from 'lucide-react';
import { CFGGrammar, FirstFollowSets, LL1ParserTable, DynamicParser } from '../utils/cfgParser';

const CFGParser = () => {
  const [grammar, setGrammar] = useState('');
  const [inputString, setInputString] = useState('');
  const [parsedGrammar, setParsedGrammar] = useState(null);
  const [firstFollow, setFirstFollow] = useState(null);
  const [parserTable, setParserTable] = useState(null);
  const [parser, setParser] = useState(null);
  const [parseResult, setParseResult] = useState(null);
  const [isParsing, setIsParsing] = useState(false);
  const [activeSubTab, setActiveSubTab] = useState('grammar');

  const handleGrammarChange = (value) => {
    setGrammar(value);
  };

  const handleParseGrammar = () => {
    const newGrammar = new CFGGrammar();
    const success = newGrammar.parseGrammar(grammar);
    
    if (success) {
      setParsedGrammar(newGrammar);
      
      const newFirstFollow = new FirstFollowSets(newGrammar);
      newFirstFollow.computeAll();
      setFirstFollow(newFirstFollow);
      
      const newParserTable = new LL1ParserTable(newGrammar, newFirstFollow);
      newParserTable.build();
      setParserTable(newParserTable);
      
      const newParser = new DynamicParser(newGrammar, newParserTable);
      setParser(newParser);
      
      setParseResult(null);
    } else {
      alert('Grammar parsing failed. Please check for errors.');
    }
  };

  const handleParseInput = () => {
    if (!parser) {
      alert('Please parse a grammar first.');
      return;
    }

    setIsParsing(true);
    
    setTimeout(() => {
      const success = parser.parse(inputString);
      setParseResult({
        success,
        parseTree: parser.parseTree,
        parsingSteps: parser.parsingSteps,
        errors: parser.errors
      });
      setIsParsing(false);
    }, 100);
  };

  const handleLoadGrammar = (event) => {
    const file = event.target.files[0];
    if (file) {
      const reader = new FileReader();
      reader.onload = (e) => {
        setGrammar(e.target.result);
      };
      reader.readAsText(file);
    }
  };

  const handleSaveGrammar = () => {
    const blob = new Blob([grammar], { type: 'text/plain' });
    const url = URL.createObjectURL(blob);
    const a = document.createElement('a');
    a.href = url;
    a.download = 'grammar.txt';
    a.click();
    URL.revokeObjectURL(url);
  };

  const exampleGrammar = `E -> E + T | E - T | T
T -> T * F | T / F | F
F -> ( E ) | id`;

  const exampleInput = 'id + id * id';

  const subTabs = [
    { id: 'grammar', label: 'Grammar', icon: FileText },
    { id: 'first-follow', label: 'FIRST/FOLLOW', icon: Settings },
    { id: 'parsing-table', label: 'Parsing Table', icon: Settings },
    { id: 'parsing-steps', label: 'Parsing Steps', icon: Settings },
  ];

  // ASCII parse tree generator
  const generateASCIITree = (node, prefix = '', isLast = true) => {
    if (!node) return '';
    
    let result = prefix + (isLast ? '└── ' : '├── ') + node.symbol + '\n';
    
    if (node.children && node.children.length > 0) {
      const childCount = node.children.length;
      node.children.forEach((child, index) => {
        const isLastChild = index === childCount - 1;
        const newPrefix = prefix + (isLast ? '    ' : '│   ');
        result += generateASCIITree(child, newPrefix, isLastChild);
      });
    }
    
    return result;
  };

  // Generate derivation steps
  const generateDerivation = () => {
    if (!parseResult) return [];
    
    const derivation = [];
    const steps = parseResult.parsingSteps;
    
    if (parseResult.parseTree) {
      derivation.push(parseResult.parseTree.symbol);
      
      steps.forEach((step, index) => {
        if (step.action.includes('Applied rule')) {
          const match = step.action.match(/Applied rule: (.+)/);
          if (match) {
            derivation.push(match[1]);
          }
        }
      });
    }
    
    return derivation;
  };

  return (
    <div className="space-y-6">
      {/* Grammar Input Section */}
      <div className="bg-white dark:bg-gray-800 rounded-lg shadow-lg p-6 border border-gray-200 dark:border-gray-700">
        <div className="flex items-center justify-between mb-4">
          <h2 className="text-lg font-semibold text-gray-900 dark:text-white">
            Grammar Input
          </h2>
          <div className="flex gap-2">
            <button
              onClick={() => setGrammar(exampleGrammar)}
              className="px-3 py-1 text-sm bg-gray-200 dark:bg-gray-700 text-gray-700 dark:text-gray-300 rounded-lg hover:bg-gray-300 dark:hover:bg-gray-600 transition-colors"
            >
              Load Example
            </button>
            <button
              onClick={handleParseGrammar}
              className="px-4 py-2 bg-primary-600 text-white rounded-lg hover:bg-primary-700 transition-colors font-medium flex items-center gap-2"
            >
              <Play className="w-4 h-4" />
              Parse Grammar
            </button>
          </div>
        </div>
        <textarea
          value={grammar}
          onChange={(e) => handleGrammarChange(e.target.value)}
          placeholder="Enter CFG grammar productions (e.g., E -> E + T | T)"
          className="w-full px-4 py-2 border border-gray-300 dark:border-gray-600 rounded-lg focus:ring-2 focus:ring-primary-500 focus:border-transparent dark:bg-gray-700 dark:text-white transition-colors duration-200 font-mono text-sm h-32 resize-none"
        />
        {parsedGrammar && parsedGrammar.errors.length > 0 && (
          <div className="mt-4 p-3 bg-red-50 dark:bg-red-900/20 border border-red-200 dark:border-red-800 rounded-lg">
            <h3 className="font-semibold text-red-800 dark:text-red-400 mb-2">Errors:</h3>
            <ul className="list-disc list-inside text-sm text-red-700 dark:text-red-300">
              {parsedGrammar.errors.map((error, index) => (
                <li key={index}>{error}</li>
              ))}
            </ul>
          </div>
        )}
        {parsedGrammar && parsedGrammar.warnings.length > 0 && (
          <div className="mt-4 p-3 bg-yellow-50 dark:bg-yellow-900/20 border border-yellow-200 dark:border-yellow-800 rounded-lg">
            <h3 className="font-semibold text-yellow-800 dark:text-yellow-400 mb-2">Warnings:</h3>
            <ul className="list-disc list-inside text-sm text-yellow-700 dark:text-yellow-300">
              {parsedGrammar.warnings.map((warning, index) => (
                <li key={index}>{warning}</li>
              ))}
            </ul>
          </div>
        )}
      </div>

      {/* Input String Section */}
      {parsedGrammar && (
        <div className="bg-white dark:bg-gray-800 rounded-lg shadow-lg p-6 border border-gray-200 dark:border-gray-700">
          <div className="flex items-center justify-between mb-4">
            <h2 className="text-lg font-semibold text-gray-900 dark:text-white">
              Input String
            </h2>
            <div className="flex gap-2">
              <button
                onClick={() => setInputString(exampleInput)}
                className="px-3 py-1 text-sm bg-gray-200 dark:bg-gray-700 text-gray-700 dark:text-gray-300 rounded-lg hover:bg-gray-300 dark:hover:bg-gray-600 transition-colors"
              >
                Load Example
              </button>
              <button
                onClick={handleParseInput}
                disabled={isParsing}
                className="px-4 py-2 bg-secondary-600 text-white rounded-lg hover:bg-secondary-700 transition-colors font-medium flex items-center gap-2 disabled:opacity-50"
              >
                <Play className="w-4 h-4" />
                {isParsing ? 'Parsing...' : 'Parse Input'}
              </button>
            </div>
          </div>
          <input
            type="text"
            value={inputString}
            onChange={(e) => setInputString(e.target.value)}
            placeholder="Enter input string to parse (e.g., id + id * id)"
            className="w-full px-4 py-2 border border-gray-300 dark:border-gray-600 rounded-lg focus:ring-2 focus:ring-primary-500 focus:border-transparent dark:bg-gray-700 dark:text-white transition-colors duration-200 font-mono"
          />
        </div>
      )}

      {/* Sub-tabs */}
      {parsedGrammar && (
        <div className="bg-white dark:bg-gray-800 rounded-lg shadow-lg border border-gray-200 dark:border-gray-700">
          <div className="flex border-b border-gray-200 dark:border-gray-700 mb-6">
            {subTabs.map((tab) => {
              const Icon = tab.icon;
              return (
                <button
                  key={tab.id}
                  onClick={() => setActiveSubTab(tab.id)}
                  className={`flex items-center gap-2 px-4 py-3 font-medium transition-colors ${
                    activeSubTab === tab.id 
                      ? 'border-b-2 border-primary-600 text-primary-600 dark:text-primary-400' 
                      : 'border-b-2 border-transparent text-gray-500 hover:text-gray-700 dark:text-gray-400 dark:hover:text-gray-200'
                  }`}
                >
                  <Icon className="w-4 h-4" />
                  {tab.label}
                </button>
              );
            })}
          </div>

          <div className="p-6 min-h-[400px]">
            {activeSubTab === 'grammar' && (
              <GrammarTabContent grammar={parsedGrammar} />
            )}
            {activeSubTab === 'first-follow' && (
              <FirstFollowTabContent firstFollow={firstFollow} />
            )}
            {activeSubTab === 'parsing-table' && (
              <ParsingTableTabContent parserTable={parserTable} />
            )}
            {activeSubTab === 'parsing-steps' && (
              <ParsingStepsTabContent 
                parseResult={parseResult} 
                generateASCIITree={generateASCIITree}
                generateDerivation={generateDerivation}
              />
            )}
          </div>
        </div>
      )}
    </div>
  );
};

// Sub-tab components
const GrammarTabContent = ({ grammar }) => {
  if (!grammar) {
    return (
      <div className="flex items-center justify-center h-96 text-gray-500 dark:text-gray-400">
        <div className="text-center">
          <p className="text-lg mb-2">No grammar loaded</p>
          <p className="text-sm">Enter a grammar and click "Parse Grammar" to begin</p>
        </div>
      </div>
    );
  }

  return (
    <div className="space-y-6">
      <div className="grid grid-cols-1 md:grid-cols-3 gap-4">
        <div className="bg-gray-50 dark:bg-gray-700/50 p-4 rounded-lg">
          <h3 className="font-semibold text-gray-900 dark:text-white mb-2">Start Symbol</h3>
          <p className="text-2xl font-mono text-primary-600 dark:text-primary-400">
            {grammar.startSymbol}
          </p>
        </div>
        <div className="bg-gray-50 dark:bg-gray-700/50 p-4 rounded-lg">
          <h3 className="font-semibold text-gray-900 dark:text-white mb-2">Non-terminals</h3>
          <p className="text-2xl font-mono text-secondary-600 dark:text-secondary-400">
            {grammar.nonTerminals.size}
          </p>
        </div>
        <div className="bg-gray-50 dark:bg-gray-700/50 p-4 rounded-lg">
          <h3 className="font-semibold text-gray-900 dark:text-white mb-2">Terminals</h3>
          <p className="text-2xl font-mono text-green-600 dark:text-green-400">
            {grammar.terminals.size}
          </p>
        </div>
      </div>

      <div>
        <h3 className="font-semibold text-gray-900 dark:text-white mb-3">Production Rules</h3>
        <div className="bg-gray-50 dark:bg-gray-800 rounded-lg p-4 max-h-64 overflow-y-auto">
          <table className="w-full text-sm">
            <thead>
              <tr className="border-b border-gray-200 dark:border-gray-700">
                <th className="text-left py-2 px-3 text-gray-700 dark:text-gray-300 font-medium">#</th>
                <th className="text-left py-2 px-3 text-gray-700 dark:text-gray-300 font-medium">Production</th>
              </tr>
            </thead>
            <tbody>
              {grammar.productions.map((prod, index) => (
                <tr key={index} className="border-b border-gray-100 dark:border-gray-800 last:border-0">
                  <td className="py-2 px-3 text-gray-600 dark:text-gray-400 font-mono text-sm">
                    {index + 1}
                  </td>
                  <td className="py-2 px-3 font-mono text-sm">
                    <span className="text-secondary-600 dark:text-secondary-400 font-semibold">
                      {prod.lhs}
                    </span>
                    <span className="text-gray-500 dark:text-gray-400 mx-2">→</span>
                    {prod.rhs.join(' ')}
                  </td>
                </tr>
              ))}
            </tbody>
          </table>
        </div>
      </div>
    </div>
  );
};

const FirstFollowTabContent = ({ firstFollow }) => {
  if (!firstFollow) {
    return (
      <div className="flex items-center justify-center h-96 text-gray-500 dark:text-gray-400">
        <div className="text-center">
          <p className="text-lg mb-2">No FIRST/FOLLOW sets computed</p>
          <p className="text-sm">Parse a grammar first to compute FIRST and FOLLOW sets</p>
        </div>
      </div>
    );
  }

  const formatSet = (set) => {
    return [...set].sort().join(', ');
  };

  return (
    <div className="space-y-6">
      <div className={`p-4 rounded-lg ${
        firstFollow.isLL1() 
          ? 'bg-green-50 dark:bg-green-900/20 border border-green-200 dark:border-green-800' 
          : 'bg-red-50 dark:bg-red-900/20 border border-red-200 dark:border-red-800'
      }`}>
        <h3 className="font-semibold mb-2">
          Grammar Status: {firstFollow.isLL1() ? 'LL(1)' : 'NOT LL(1)'}
        </h3>
        {!firstFollow.isLL1() && (
          <div className="mt-2">
            <p className="text-sm font-medium mb-1">Conflicts:</p>
            <ul className="list-disc list-inside text-sm space-y-1">
              {firstFollow.getLL1Conflicts().map((conflict, index) => (
                <li key={index}>{conflict}</li>
              ))}
            </ul>
          </div>
        )}
      </div>

      <div>
        <h3 className="font-semibold text-gray-900 dark:text-white mb-3">FIRST Sets</h3>
        <div className="bg-gray-50 dark:bg-gray-800 rounded-lg p-4 max-h-64 overflow-y-auto">
          <table className="w-full text-sm">
            <thead>
              <tr className="border-b border-gray-200 dark:border-gray-700">
                <th className="text-left py-2 px-3 text-gray-700 dark:text-gray-300 font-medium">Symbol</th>
                <th className="text-left py-2 px-3 text-gray-700 dark:text-gray-300 font-medium">FIRST Set</th>
              </tr>
            </thead>
            <tbody>
              {[...firstFollow.firstSets.entries()].sort().map(([symbol, set]) => (
                <tr key={symbol} className="border-b border-gray-100 dark:border-gray-800 last:border-0">
                  <td className="py-2 px-3 font-mono text-sm font-semibold text-secondary-600 dark:text-secondary-400">
                    {symbol}
                  </td>
                  <td className="py-2 px-3 font-mono text-sm text-gray-700 dark:text-gray-300">
                    {formatSet(set)}
                  </td>
                </tr>
              ))}
            </tbody>
          </table>
        </div>
      </div>

      <div>
        <h3 className="font-semibold text-gray-900 dark:text-white mb-3">FOLLOW Sets</h3>
        <div className="bg-gray-50 dark:bg-gray-800 rounded-lg p-4 max-h-64 overflow-y-auto">
          <table className="w-full text-sm">
            <thead>
              <tr className="border-b border-gray-200 dark:border-gray-700">
                <th className="text-left py-2 px-3 text-gray-700 dark:text-gray-300 font-medium">Symbol</th>
                <th className="text-left py-2 px-3 text-gray-700 dark:text-gray-300 font-medium">FOLLOW Set</th>
              </tr>
            </thead>
            <tbody>
              {[...firstFollow.followSets.entries()].sort().map(([symbol, set]) => (
                <tr key={symbol} className="border-b border-gray-100 dark:border-gray-800 last:border-0">
                  <td className="py-2 px-3 font-mono text-sm font-semibold text-secondary-600 dark:text-secondary-400">
                    {symbol}
                  </td>
                  <td className="py-2 px-3 font-mono text-sm text-gray-700 dark:text-gray-300">
                    {formatSet(set)}
                  </td>
                </tr>
              ))}
            </tbody>
          </table>
        </div>
      </div>
    </div>
  );
};

const ParsingTableTabContent = ({ parserTable }) => {
  if (!parserTable) {
    return (
      <div className="flex items-center justify-center h-96 text-gray-500 dark:text-gray-400">
        <div className="text-center">
          <p className="text-lg mb-2">No parsing table built</p>
          <p className="text-sm">Parse a grammar first to build the LL(1) parsing table</p>
        </div>
      </div>
    );
  }

  const terminals = [...parserTable.grammar.terminals, '$'].sort();
  const nonTerminals = [...parserTable.grammar.nonTerminals].sort();

  const getProductionDisplay = (production) => {
    if (!production) return '-';
    return `${production.lhs} → ${production.rhs.join(' ')}`;
  };

  return (
    <div className="space-y-6">
      <div className={`p-4 rounded-lg ${
        parserTable.isGrammarLL1() 
          ? 'bg-green-50 dark:bg-green-900/20 border border-green-200 dark:border-green-800' 
          : 'bg-red-50 dark:bg-red-900/20 border border-red-200 dark:border-red-800'
      }`}>
        <h3 className="font-semibold mb-2">
          Parsing Table Status: {parserTable.isGrammarLL1() ? 'Valid LL(1)' : 'Invalid (Has Conflicts)'}
        </h3>
        {!parserTable.isGrammarLL1() && parserTable.conflicts.length > 0 && (
          <div className="mt-2">
            <p className="text-sm font-medium mb-1">Conflicts:</p>
            <ul className="list-disc list-inside text-sm space-y-1">
              {parserTable.conflicts.map((conflict, index) => (
                <li key={index}>{conflict}</li>
              ))}
            </ul>
          </div>
        )}
      </div>

      <div className="overflow-x-auto">
        <div className="bg-gray-50 dark:bg-gray-800 rounded-lg p-4">
          <table className="w-full text-sm">
            <thead>
              <tr className="border-b border-gray-200 dark:border-gray-700">
                <th className="py-2 px-3 text-left text-gray-700 dark:text-gray-300 font-medium bg-gray-100 dark:bg-gray-700">
                  NT \ T
                </th>
                {terminals.map((terminal) => (
                  <th key={terminal} className="py-2 px-3 text-center text-gray-700 dark:text-gray-300 font-medium bg-gray-100 dark:bg-gray-700">
                    {terminal}
                  </th>
                ))}
              </tr>
            </thead>
            <tbody>
              {nonTerminals.map((nt) => (
                <tr key={nt} className="border-b border-gray-100 dark:border-gray-800 last:border-0">
                  <td className="py-2 px-3 font-mono font-semibold text-secondary-600 dark:text-secondary-400 bg-gray-50 dark:bg-gray-700/50">
                    {nt}
                  </td>
                  {terminals.map((terminal) => {
                    const production = parserTable.getEntry(nt, terminal);
                    return (
                      <td key={terminal} className="py-2 px-3 text-center">
                        <span className="font-mono text-xs text-gray-700 dark:text-gray-300">
                          {getProductionDisplay(production)}
                        </span>
                      </td>
                    );
                  })}
                </tr>
              ))}
            </tbody>
          </table>
        </div>
      </div>
    </div>
  );
};

const ParsingStepsTabContent = ({ parseResult, generateASCIITree, generateDerivation }) => {
  if (!parseResult) {
    return (
      <div className="flex items-center justify-center h-96 text-gray-500 dark:text-gray-400">
        <div className="text-center">
          <p className="text-lg mb-2">No parsing steps available</p>
          <p className="text-sm">Parse an input string first to see the parsing steps</p>
        </div>
      </div>
    );
  }

  const steps = parseResult.parsingSteps;
  const derivation = generateDerivation();
  const asciiTree = parseResult.parseTree ? generateASCIITree(parseResult.parseTree) : '';

  return (
    <div className="space-y-6">
      <div className={`p-4 rounded-lg ${
        parseResult.success 
          ? 'bg-green-50 dark:bg-green-900/20 border border-green-200 dark:border-green-800' 
          : 'bg-red-50 dark:bg-red-900/20 border border-red-200 dark:border-red-800'
      }`}>
        <h3 className="font-semibold mb-2">
          Parsing Result: {parseResult.success ? 'ACCEPT' : 'REJECT'}
        </h3>
        {parseResult.errors.length > 0 && (
          <div className="mt-2">
            <p className="text-sm font-medium mb-1">Errors:</p>
            <ul className="list-disc list-inside text-sm space-y-1">
              {parseResult.errors.map((error, index) => (
                <li key={index}>{error}</li>
              ))}
            </ul>
          </div>
        )}
      </div>

      <div className="bg-white dark:bg-gray-800 rounded-lg shadow-lg p-6 border border-gray-200 dark:border-gray-700">
        <h3 className="font-semibold text-gray-900 dark:text-white mb-4">Parse Tree</h3>
        <div className="bg-gray-50 dark:bg-gray-800 rounded-lg p-6 overflow-x-auto">
          <pre className="font-mono text-sm text-gray-700 dark:text-gray-300 whitespace-pre">
            {asciiTree}
          </pre>
        </div>
      </div>

      <div className="bg-white dark:bg-gray-800 rounded-lg shadow-lg p-6 border border-gray-200 dark:border-gray-700">
        <h3 className="font-semibold text-gray-900 dark:text-white mb-4">Step-by-step Derivation</h3>
        <div className="bg-gray-50 dark:bg-gray-800 rounded-lg p-6">
          <div className="space-y-2">
            {derivation.map((step, index) => (
              <div key={index} className="font-mono text-sm text-gray-700 dark:text-gray-300">
                {index === 0 ? step : '→ ' + step}
              </div>
            ))}
          </div>
        </div>
      </div>

      <div className="bg-white dark:bg-gray-800 rounded-lg shadow-lg p-6 border border-gray-200 dark:border-gray-700">
        <h3 className="font-semibold text-gray-900 dark:text-white mb-4">Parsing Steps</h3>
        <div className="bg-gray-50 dark:bg-gray-800 rounded-lg p-4 max-h-64 overflow-y-auto">
          <table className="w-full text-sm">
            <thead>
              <tr className="border-b border-gray-200 dark:border-gray-700">
                <th className="py-2 px-3 text-left text-gray-700 dark:text-gray-300 font-medium bg-gray-100 dark:bg-gray-700">
                  Step
                </th>
                <th className="py-2 px-3 text-left text-gray-700 dark:text-gray-300 font-medium bg-gray-100 dark:bg-gray-700">
                  Stack
                </th>
                <th className="py-2 px-3 text-left text-gray-700 dark:text-gray-300 font-medium bg-gray-100 dark:bg-gray-700">
                  Input
                </th>
                <th className="py-2 px-3 text-left text-gray-700 dark:text-gray-300 font-medium bg-gray-100 dark:bg-gray-700">
                  Action
                </th>
              </tr>
            </thead>
            <tbody>
              {steps.map((step, index) => (
                <tr key={index} className="border-b border-gray-100 dark:border-gray-800 last:border-0">
                  <td className="py-2 px-3 font-mono text-gray-700 dark:text-gray-300">
                    {step.step}
                  </td>
                  <td className="py-2 px-3 font-mono text-gray-700 dark:text-gray-300">
                    [{step.stack.join(', ')}]
                  </td>
                  <td className="py-2 px-3 font-mono text-gray-700 dark:text-gray-300">
                    {step.input.join(' ')}
                  </td>
                  <td className="py-2 px-3 text-gray-700 dark:text-gray-300">
                    {step.action}
                  </td>
                </tr>
              ))}
            </tbody>
          </table>
        </div>
      </div>
    </div>
  );
};

export default CFGParser;
