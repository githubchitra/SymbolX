import React, { useState } from 'react';
import { Play, AlertCircle, FileText, Binary, RefreshCw, Layout, Code } from 'lucide-react';
import { CFGGrammar, EarleyParser } from '../utils/cfgParser';

const CFGParser = () => {
  const [grammar, setGrammar] = useState(defaultGrammar);
  const [inputString, setInputString] = useState('id + id * id');
  const [parseResult, setParseResult] = useState(null);
  const [errorMsg, setErrorMsg] = useState(null);
  const [isParsing, setIsParsing] = useState(false);
  const [viewMode, setViewMode] = useState('visual'); // 'visual' or 'ascii'

  const handleParseAll = () => {
    setIsParsing(true);
    setErrorMsg(null);
    setParseResult(null);

    setTimeout(() => {
      try {
        const newGrammar = new CFGGrammar();
        // Support both U+2192 (→) and standard "->" arrows
        const normalizedGrammar = grammar.replace(/→/g, '->');
        const success = newGrammar.parseGrammar(normalizedGrammar);

        if (!success) {
          setErrorMsg({
            title: 'Grammar Syntax Error',
            description: 'Failed to parse grammar rules. Please ensure every rule is in the format "A -> alpha | beta".',
            details: newGrammar.errors
          });
          setIsParsing(false);
          return;
        }

        const parser = new EarleyParser(newGrammar);
        const tokens = parser.tokenizeInput(inputString);
        const parseTree = parser.parse(tokens);

        if (!parseTree) {
          setErrorMsg({
            title: 'Expression Parsing Failed',
            description: 'The parser could not generate a parse tree for the input string using the given grammar.',
            details: parser.errors
          });
          setIsParsing(false);
          return;
        }

        setParseResult({
          success: true,
          parseTree: parseTree,
          grammar: newGrammar
        });
      } catch (e) {
        console.error(e);
        setErrorMsg({
          title: 'Unexpected Error',
          description: 'An unexpected error occurred during grammar analysis.',
          details: [e.message]
        });
      } finally {
        setIsParsing(false);
      }
    }, 150);
  };

  const handleReset = () => {
    setGrammar(defaultGrammar);
    setInputString('id + id * id');
    setParseResult(null);
    setErrorMsg(null);
  };

  // ASCII prefix tree generator
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

  return (
    <div className="space-y-6 max-w-7xl mx-auto animate-fade-in">
      {/* Title Header banner */}
      <div className="bg-gradient-to-r from-primary-600 to-indigo-600 dark:from-primary-700 dark:to-indigo-800 rounded-2xl shadow-xl p-6 text-white flex flex-col md:flex-row items-center justify-between gap-4">
        <div className="flex items-center gap-4 text-center md:text-left">
          <div className="p-3 bg-white/10 rounded-xl backdrop-blur-md">
            <Binary className="h-8 w-8 text-white" />
          </div>
          <div>
            <h2 className="text-2xl font-bold tracking-tight">CFG Parse Tree Visualizer</h2>
            <p className="text-white/80 text-sm mt-1">
              Enter any context-free grammar and expression, then instantly generate its syntax parse tree.
            </p>
          </div>
        </div>
        <div className="flex items-center gap-3">
          <button
            onClick={() => {
              setGrammar(defaultGrammar);
              setInputString('id + id * id');
            }}
            className="px-4 py-2 bg-white hover:bg-gray-50 text-primary-700 hover:text-primary-800 rounded-xl text-xs font-bold tracking-wider uppercase shadow-md transition-all border border-transparent active:scale-95"
            title="Load default arithmetic expression grammar"
          >
            Load Default Grammar
          </button>
        </div>
      </div>

      <div className="grid grid-cols-1 lg:grid-cols-12 gap-6">
        {/* Left Column - Input Controls (5/12 cols) */}
        <div className="lg:col-span-5 space-y-6">
          <div className="bg-white dark:bg-gray-800 rounded-2xl shadow-lg border border-gray-100 dark:border-gray-700 p-6 flex flex-col">
            <h3 className="text-sm font-bold text-gray-400 dark:text-gray-500 uppercase tracking-wider mb-4 flex items-center gap-2">
              <Layout className="w-4 h-4 text-primary-500" />
              Parsing Configuration
            </h3>

            {/* Grammar Input */}
            <div className="space-y-2 mb-4">
              <label className="text-xs font-bold text-gray-700 dark:text-gray-300 uppercase tracking-wider flex justify-between">
                <span>Grammar Productions (CFG)</span>
                <span className="text-[10px] text-gray-400 lowercase font-normal">Supports "→" or "-&gt;" arrows</span>
              </label>
              <textarea
                value={grammar}
                onChange={(e) => setGrammar(e.target.value)}
                placeholder="E -> E + T | T..."
                className="w-full px-4 py-3 border border-gray-200 dark:border-gray-700 rounded-xl focus:ring-2 focus:ring-primary-500 focus:border-transparent dark:bg-gray-900 dark:text-white font-mono text-sm h-48 resize-none shadow-inner transition-all duration-200"
              />
            </div>

            {/* Input Expression String */}
            <div className="space-y-2 mb-6">
              <label className="text-xs font-bold text-gray-700 dark:text-gray-300 uppercase tracking-wider">
                Expression Input String
              </label>
              <input
                type="text"
                value={inputString}
                onChange={(e) => setInputString(e.target.value)}
                placeholder="e.g., id + id * id"
                className="w-full px-4 py-3 border border-gray-200 dark:border-gray-700 rounded-xl focus:ring-2 focus:ring-primary-500 focus:border-transparent dark:bg-gray-900 dark:text-white font-mono text-sm shadow-inner transition-all duration-200"
              />
            </div>

            {/* Control Actions */}
            <div className="flex gap-3 mt-auto">
              <button
                onClick={handleReset}
                className="px-4 py-3 border border-gray-200 dark:border-gray-700 text-gray-500 hover:text-gray-700 dark:text-gray-400 dark:hover:text-gray-200 hover:bg-gray-50 dark:hover:bg-gray-700/50 rounded-xl font-bold text-sm tracking-wide transition-all active:scale-95 flex items-center justify-center gap-2"
              >
                <RefreshCw className="w-4 h-4" />
                Reset
              </button>
              <button
                onClick={handleParseAll}
                disabled={isParsing}
                className="flex-1 px-6 py-3 bg-gradient-to-r from-primary-600 to-indigo-600 hover:from-primary-700 hover:to-indigo-700 text-white rounded-xl font-bold text-sm tracking-wide shadow-md hover:shadow-lg transition-all active:scale-95 disabled:opacity-50 flex items-center justify-center gap-2"
              >
                <Play className="w-4 h-4 fill-current" />
                {isParsing ? 'Processing...' : 'Generate Parse Tree'}
              </button>
            </div>
          </div>
        </div>

        {/* Right Column - Parse Tree Visualizer (7/12 cols) */}
        <div className="lg:col-span-7 flex flex-col">
          <div className="bg-white dark:bg-gray-800 rounded-2xl shadow-lg border border-gray-100 dark:border-gray-700 p-6 flex flex-col flex-1 min-h-[500px]">
            {/* Output Header with Tab Mode Switches */}
            <div className="flex items-center justify-between border-b border-gray-100 dark:border-gray-700 pb-4 mb-6">
              <h3 className="text-sm font-bold text-gray-400 dark:text-gray-500 uppercase tracking-wider flex items-center gap-2">
                <Code className="w-4 h-4 text-primary-500" />
                Parse Tree Output
              </h3>

              {parseResult && (
                <div className="flex bg-gray-100 dark:bg-gray-900 rounded-lg p-0.5 border border-gray-200/50 dark:border-gray-800 shadow-inner">
                  <button
                    onClick={() => setViewMode('visual')}
                    className={`px-3 py-1.5 rounded-md font-bold text-xs transition-all ${
                      viewMode === 'visual'
                        ? 'bg-white dark:bg-gray-700 text-gray-900 dark:text-white shadow-sm'
                        : 'text-gray-500 hover:text-gray-700 dark:text-gray-400 dark:hover:text-gray-200'
                    }`}
                  >
                    Visual Diagram
                  </button>
                  <button
                    onClick={() => setViewMode('ascii')}
                    className={`px-3 py-1.5 rounded-md font-bold text-xs transition-all ${
                      viewMode === 'ascii'
                        ? 'bg-white dark:bg-gray-700 text-gray-900 dark:text-white shadow-sm'
                        : 'text-gray-500 hover:text-gray-700 dark:text-gray-400 dark:hover:text-gray-200'
                    }`}
                  >
                    ASCII Tree
                  </button>
                </div>
              )}
            </div>

            {/* Error alerts */}
            {errorMsg && (
              <div className="bg-red-50 dark:bg-red-950/20 border border-red-200 dark:border-red-900/50 rounded-2xl p-5 shadow-sm animate-shake">
                <div className="flex items-start gap-3">
                  <AlertCircle className="w-6 h-6 text-red-600 dark:text-red-500 shrink-0 mt-0.5" />
                  <div className="space-y-1">
                    <h4 className="font-bold text-red-800 dark:text-red-400 text-sm">{errorMsg.title}</h4>
                    <p className="text-xs text-red-700 dark:text-red-300 leading-relaxed">
                      {errorMsg.description}
                    </p>
                  </div>
                </div>
                {errorMsg.details && errorMsg.details.length > 0 && (
                  <div className="mt-4 bg-white/60 dark:bg-black/20 rounded-xl p-3 border border-red-200/50 max-h-48 overflow-y-auto">
                    <ul className="list-disc list-inside text-xs text-red-900 dark:text-red-400 font-mono space-y-1">
                      {errorMsg.details.map((detail, idx) => (
                        <li key={idx}>{detail}</li>
                      ))}
                    </ul>
                  </div>
                )}
              </div>
            )}

            {/* Parse Tree output */}
            {parseResult ? (
              <div className="flex-1 flex flex-col justify-center animate-fade-in">
                {viewMode === 'visual' ? (
                  /* Visual node tree block */
                  <div className="flex-1 flex items-center justify-center p-6 bg-gray-50/50 dark:bg-gray-900/30 rounded-2xl border border-gray-100 dark:border-gray-700/50 overflow-auto max-h-[500px] shadow-inner custom-scrollbar">
                    <div className="p-4 flex items-center justify-center">
                      <VisualTreeNode node={parseResult.parseTree} />
                    </div>
                  </div>
                ) : (
                  /* Standard ASCII prefix block */
                  <div className="flex-1 bg-gray-950 dark:bg-black rounded-2xl border border-gray-800 p-6 overflow-auto max-h-[500px] shadow-inner font-mono text-sm text-green-400/90 leading-relaxed custom-scrollbar">
                    <pre className="whitespace-pre">
                      {generateASCIITree(parseResult.parseTree)}
                    </pre>
                  </div>
                )}
              </div>
            ) : (
              /* Empty state / Welcome card */
              !errorMsg && (
                <div className="flex-1 flex flex-col items-center justify-center text-center p-8 border-2 border-dashed border-gray-200 dark:border-gray-700/50 rounded-2xl animate-fade-in">
                  <div className="w-16 h-16 bg-primary-50 dark:bg-primary-950/30 text-primary-500 dark:text-primary-400 rounded-2xl flex items-center justify-center mb-4 shadow-md shadow-primary-500/5">
                    <FileText className="w-8 h-8" />
                  </div>
                  <h4 className="font-bold text-gray-800 dark:text-gray-200 text-base mb-1">
                    Waiting for Parse Input
                  </h4>
                  <p className="text-xs text-gray-400 dark:text-gray-500 max-w-sm leading-relaxed">
                    Set up your grammar rules and input expression string on the left, then click <strong>Generate Parse Tree</strong> to begin the visual synthesis.
                  </p>
                </div>
              )
            )}
          </div>
        </div>
      </div>
    </div>
  );
};

// Beautiful Interactive Recursive HTML Tree Component
const VisualTreeNode = ({ node }) => {
  if (!node) return null;

  const isLeaf = !node.children || node.children.length === 0;

  return (
    <div className="flex flex-col items-center animate-fade-in">
      {/* Node label capsule */}
      <div
        className={`px-3 py-1.5 rounded-lg border font-mono text-xs font-bold shadow-sm transition-all hover:scale-105 active:scale-95 duration-200 ${
          node.isNonTerminal
            ? 'bg-primary-50 hover:bg-primary-100 border-primary-200 text-primary-700 dark:bg-primary-950/40 dark:border-primary-800 dark:text-primary-300'
            : 'bg-green-50 hover:bg-green-100 border-green-200 text-green-700 dark:bg-green-950/40 dark:border-green-800 dark:text-green-300'
        }`}
        title={`${node.isNonTerminal ? 'Non-Terminal' : 'Terminal'}`}
      >
        {node.symbol}
      </div>

      {/* Connection connector lines to children */}
      {!isLeaf && (
        <div className="flex flex-col items-center w-full mt-2">
          {/* Main stem from parent */}
          <div className="w-0.5 h-3 bg-gray-300 dark:bg-gray-600" />

          {/* Horizontal cross-bar bridging all child branches */}
          {node.children.length > 1 && (
            <div className="flex w-full px-6 justify-between h-0.5 bg-gray-300 dark:bg-gray-600 mb-2" />
          )}

          {/* Children row */}
          <div className="flex gap-4 justify-center items-start mt-1">
            {node.children.map((child, index) => (
              <VisualTreeNode key={index} node={child} />
            ))}
          </div>
        </div>
      )}
    </div>
  );
};

// Left recursive grammar example (causes table conflicts)
const defaultGrammar = `E → E + T | T
T → T * F | F
F → ( E ) | id`;

export default CFGParser;
