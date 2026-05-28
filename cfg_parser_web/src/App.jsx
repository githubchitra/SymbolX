import React, { useState, useEffect } from 'react';
import { Moon, Sun, Play, Download, Upload, FileText, Settings } from 'lucide-react';
import { CFGGrammar, FirstFollowSets, LL1ParserTable, DynamicParser } from './utils/cfgParser';
import GrammarTab from './components/GrammarTab';
import FirstFollowTab from './components/FirstFollowTab';

function App() {
  const [darkMode, setDarkMode] = useState(false);
  const [activeTab, setActiveTab] = useState('grammar');
  const [grammar, setGrammar] = useState('');
  const [inputString, setInputString] = useState('');
  const [parsedGrammar, setParsedGrammar] = useState(null);
  const [firstFollow, setFirstFollow] = useState(null);
  const [parser, setParser] = useState(null);
  const [isParsing, setIsParsing] = useState(false);

  useEffect(() => {
    // Check for saved theme preference
    const savedTheme = localStorage.getItem('theme');
    if (savedTheme === 'dark') {
      setDarkMode(true);
      document.documentElement.classList.add('dark');
    }
  }, []);

  const toggleTheme = () => {
    setDarkMode(!darkMode);
    document.documentElement.classList.toggle('dark');
    localStorage.setItem('theme', darkMode ? 'light' : 'dark');
  };

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
      
      const newParser = new DynamicParser(newGrammar, newParserTable);
      setParser(newParser);
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
      parser.parse(inputString);
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

  const tabs = [
    { id: 'grammar', label: 'Grammar', icon: FileText },
    { id: 'first-follow', label: 'FIRST/FOLLOW', icon: Settings },
  ];

  const exampleGrammar = `E -> E + T | E - T | T
T -> T * F | T / F | F
F -> ( E ) | id`;

  const exampleInput = 'id + id * id';

  return (
    <div className={`min-h-screen transition-colors duration-200 ${darkMode ? 'dark' : ''}`}>
      <div className="bg-gray-50 dark:bg-gray-900 min-h-screen">
        {/* Header */}
        <header className="bg-white dark:bg-gray-800 shadow-lg border-b border-gray-200 dark:border-gray-700">
          <div className="max-w-7xl mx-auto px-4 py-4">
            <div className="flex items-center justify-between">
              <div>
                <h1 className="text-2xl font-bold text-gray-900 dark:text-white">
                  CFG Grammar Parser
                </h1>
                <p className="text-sm text-gray-600 dark:text-gray-400">
                  Interactive Context-Free Grammar Parser
                </p>
              </div>
              <div className="flex items-center gap-4">
                <button
                  onClick={handleLoadGrammar}
                  className="p-2 rounded-lg hover:bg-gray-100 dark:hover:bg-gray-700 transition-colors"
                  title="Load Grammar"
                >
                  <Upload className="w-5 h-5 text-gray-600 dark:text-gray-400" />
                </button>
                <button
                  onClick={handleSaveGrammar}
                  className="p-2 rounded-lg hover:bg-gray-100 dark:hover:bg-gray-700 transition-colors"
                  title="Save Grammar"
                >
                  <Download className="w-5 h-5 text-gray-600 dark:text-gray-400" />
                </button>
                <button
                  onClick={toggleTheme}
                  className="p-2 rounded-lg hover:bg-gray-100 dark:hover:bg-gray-700 transition-colors"
                  title="Toggle Theme"
                >
                  {darkMode ? (
                    <Sun className="w-5 h-5 text-gray-600 dark:text-gray-400" />
                  ) : (
                    <Moon className="w-5 h-5 text-gray-600 dark:text-gray-400" />
                  )}
                </button>
              </div>
            </div>
          </div>
        </header>

        {/* Main Content */}
        <main className="max-w-7xl mx-auto px-4 py-6">
          {/* Grammar Input Section */}
          <div className="card mb-6">
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
                  className="btn-primary flex items-center gap-2"
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
              className="input-field font-mono text-sm h-32 resize-none"
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
            <div className="card mb-6">
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
                    className="btn-secondary flex items-center gap-2 disabled:opacity-50"
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
                className="input-field font-mono"
              />
            </div>
          )}

          {/* Tabs */}
          <div className="card">
            <div className="flex border-b border-gray-200 dark:border-gray-700 mb-6">
              {tabs.map((tab) => {
                const Icon = tab.icon;
                return (
                  <button
                    key={tab.id}
                    onClick={() => setActiveTab(tab.id)}
                    className={`flex items-center gap-2 px-4 py-3 font-medium transition-colors ${
                      activeTab === tab.id ? 'tab-active' : 'tab-inactive'
                    }`}
                  >
                    <Icon className="w-4 h-4" />
                    {tab.label}
                  </button>
                );
              })}
            </div>

            {/* Tab Content */}
            <div className="min-h-[400px]">
              {activeTab === 'grammar' && (
                <GrammarTab grammar={parsedGrammar} />
              )}
              {activeTab === 'first-follow' && (
                <FirstFollowTab firstFollow={firstFollow} />
              )}
            </div>
          </div>
        </main>

        {/* Footer */}
        <footer className="bg-white dark:bg-gray-800 border-t border-gray-200 dark:border-gray-700 mt-8">
          <div className="max-w-7xl mx-auto px-4 py-4 text-center text-sm text-gray-600 dark:text-gray-400">
            CFG Grammar Parser - Built with React and Tailwind CSS
          </div>
        </footer>
      </div>
    </div>
  );
}

export default App;
