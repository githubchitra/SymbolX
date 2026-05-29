import React, { useState, useEffect, useRef } from 'react';
import { BarChart3, Code, Moon, Sun, Table, AlertCircle, Hash, Binary } from 'lucide-react';
import SymbolTableGrid from './components/SymbolTableGrid';
import ScopeTree from './components/ScopeTree';
import CodeParser from './components/CodeParser';
import ProblemsPanel from './components/ProblemsPanel';
import CFGParser from './components/CFGParser';
import { wsManager, symbolTableAPI, parseAPI } from './services/api';

function App() {
  const [activeTab, setActiveTab] = useState('symbols');
  const [symbols, setSymbols] = useState([]);
  const [selectedSymbol, setSelectedSymbol] = useState(null);
  const [currentScope, setCurrentScope] = useState('global');
  const [darkMode, setDarkMode] = useState(false);
  const [wsConnected, setWsConnected] = useState(false);
  const [problems, setProblems] = useState([]);
  const parserRef = useRef(null);

  const [code, setCode] = useState('');
  const [savedCode, setSavedCode] = useState('');

  const fetchSymbols = async () => {
    try {
      const data = await symbolTableAPI.getAllSymbols();
      setSymbols(data.symbols || []);
    } catch (error) {
      console.error('Failed to fetch symbols:', error);
    }
  };

  useEffect(() => {
    fetchSymbols();

    wsManager.on('connected', () => {
      setWsConnected(true);
      console.log('WebSocket connected');
    });

    wsManager.on('disconnected', () => {
      setWsConnected(false);
      console.log('WebSocket disconnected');
    });

    wsManager.on('message', (data) => {
      console.log('WebSocket message:', data);
      if (data.type === 'symbol_added') {
        console.log('Symbol added:', data.symbol);
        setSymbols(prev => [...prev, data.symbol]);
      } else if (data.type === 'symbol_updated') {
        console.log('Symbol updated:', data.symbol);
        setSymbols(prev => prev.map(s => s.name === data.symbol.name ? data.symbol : s));
      } else if (data.type === 'scope_entered') {
        setCurrentScope(data.scope_name);
      } else if (data.type === 'scope_exited') {
        setCurrentScope('global');
      }
    });

    wsManager.connect();

    return () => {
      wsManager.disconnect();
    };
  }, []);

  // Real-time analysis effect
  useEffect(() => {
    if (!code.trim()) {
      setProblems([]);
      return;
    }

    const timer = setTimeout(async () => {
      try {
        const data = await parseAPI.analyzeCode(code);
        if (data && data.problems) {
          setProblems(data.problems);
        }
      } catch (error) {
        console.error('Real-time analysis failed:', error);
      }
    }, 500);

    return () => clearTimeout(timer);
  }, [code]);

  const handleSymbolSelect = (symbol) => {
    setSelectedSymbol(symbol);
  };

  const handleSymbolEdit = async (symbol) => {
    // For now, let's use a simple prompt to demonstrate it works
    // In a real app, this would be a modal
    const newInitialValue = window.prompt(`Enter new initial value for ${symbol.name}:`, symbol.initial_value || '');
    if (newInitialValue !== null) {
      try {
        await symbolTableAPI.updateSymbol(symbol.name, { initial_value: newInitialValue });
        fetchSymbols(); // Refresh the list
      } catch (error) {
        alert('Failed to update symbol: ' + error.message);
      }
    }
  };

  const handleDelete = async (name) => {
    if (window.confirm(`Are you sure you want to delete symbol "${name}"?`)) {
      try {
        await symbolTableAPI.deleteSymbol(name);
        setSymbols(prev => prev.filter(s => s.name !== name));
      } catch (error) {
        alert('Failed to delete symbol: ' + error.message);
      }
    }
  };

  const handleParseComplete = (results) => {
    console.log('Parse completed:', results);
    if (results.problems) {
      setProblems(results.problems);
    }


    fetchSymbols();
    // Only switch to symbols if there are no errors, otherwise stay on problems if they exist
    if (results.problems?.some(p => p.severity === 'error')) {
      setActiveTab('problems');
    } else {
      setActiveTab('symbols');
    }
  };

  const handleReset = () => {
    setSymbols([]);
    setSelectedSymbol(null);
    setCurrentScope('global');
    setCode('');
    setSavedCode('');
  };

  const handleStepComplete = (step, stepIndex) => {
    console.log('Step completed:', step, stepIndex);
  };

  const toggleDarkMode = () => {
    setDarkMode(!darkMode);
    document.documentElement.classList.toggle('dark');
  };

  const tabs = [
    {
      id: 'symbols',
      name: 'Symbol Table',
      icon: Table,
      component: (
        <SymbolTableGrid
          symbols={symbols}
          onSymbolSelect={handleSymbolSelect}
          onSymbolEdit={handleSymbolEdit}
          onDelete={handleDelete}
          problems={problems}
        />
      )
    },
    {
      id: 'scopes',
      name: 'Scope Hierarchy',
      icon: BarChart3,
      component: (
        <ScopeTree
          onScopeSelect={setCurrentScope}
          currentScope={currentScope}
          problems={problems}
        />
      )
    },
    {
      id: 'parser',
      name: 'Code Parser',
      icon: Code,
      component: (
        <CodeParser
          ref={parserRef}
          code={code}
          setCode={setCode}
          savedCode={savedCode}
          setSavedCode={setSavedCode}
          onParseComplete={handleParseComplete}
          onStepComplete={handleStepComplete}
          onReset={handleReset}
          problems={problems}
        />
      )
    },
    {
      id: 'parsing',
      name: 'Parsing',
      icon: Binary,
      component: (
        <CFGParser />
      )
    },
    {
      id: 'problems',
      name: 'Problems',
      icon: AlertCircle,
      component: (
        <ProblemsPanel
          problems={problems}
          onNavigate={(line) => {
            setActiveTab('parser');
            setTimeout(() => {
              parserRef.current?.jumpToLine(line);
            }, 100);
          }}
        />
      )
    }
  ];

  return (
    /* ✅ yaha flex + flex-col add kiya */
    <div
      className={`min-h-screen flex flex-col ${darkMode ? 'dark bg-gray-900' : 'bg-gray-50'
        }`}
    >
      {/* Header */}
      <header className="bg-white dark:bg-gray-800 shadow-sm border-b border-gray-200 dark:border-gray-700">
        <div className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8">
          <div className="flex items-center justify-between h-16">
            <div className="flex items-center">
              <Table className="h-8 w-8 text-primary-600 dark:text-primary-400 mr-3" />
              <h1 className="text-xl font-bold text-gray-900 dark:text-white">
                Symbol Table Visualizer
              </h1>
            </div>

            <div className="flex items-center gap-4">
              <nav className="flex space-x-2 mr-4" aria-label="Tabs">
                {tabs.map((tab) => {
                  const Icon = tab.icon;
                  const problemCount = tab.id === 'problems' ? problems.length : 0;
                  const hasError = tab.id === 'problems' && problems.some(p => p.severity === 'error');

                  return (
                    <button
                      key={tab.id}
                      onClick={() => setActiveTab(tab.id)}
                      className={`${activeTab === tab.id
                        ? 'bg-primary-50 text-primary-700 ring-1 ring-primary-200 dark:bg-gray-700 dark:text-white'
                        : 'text-gray-500 hover:text-gray-700 hover:bg-gray-50 dark:text-gray-400 dark:hover:text-gray-300 dark:hover:bg-gray-700'
                        } px-3 py-2 rounded-lg font-bold text-xs flex items-center gap-2 transition-all group relative`}
                    >
                      <Icon className={`h-4 w-4 ${activeTab === tab.id ? 'text-primary-600' : 'text-gray-400 group-hover:text-gray-600'}`} />
                      {tab.name}
                      {problemCount > 0 && (
                        <span className={`flex items-center justify-center min-w-[18px] h-[18px] text-[10px] rounded-full text-white font-bold ml-1 ${hasError ? 'bg-red-500' : 'bg-amber-500'}`}>
                          {problemCount}
                        </span>
                      )}
                    </button>
                  );
                })}
              </nav>

              <div className="flex items-center gap-2">
                <div className={`flex items-center gap-2 px-3 py-1.5 rounded-full text-[10px] font-bold tracking-wider uppercase border shadow-sm transition-all ${wsConnected
                  ? 'bg-green-50 text-green-700 border-green-200 shadow-green-100/50'
                  : 'bg-red-50 text-red-700 border-red-200'
                  }`}>
                  <div className={`h-1.5 w-1.5 rounded-full ${wsConnected ? 'bg-green-500 animate-pulse' : 'bg-red-500'}`} />
                  {wsConnected ? 'System Live' : 'Offline'}
                </div>

                <button
                  onClick={toggleDarkMode}
                  className="p-2 rounded-xl text-gray-500 hover:text-primary-600 hover:bg-primary-50 active:scale-90 transition-all border border-transparent hover:border-primary-100"
                  title="Toggle theme"
                >
                  {darkMode ? (
                    <Sun className="h-5 w-5" />
                  ) : (
                    <Moon className="h-5 w-5" />
                  )}
                </button>
              </div>
            </div>
          </div>
        </div>
      </header>



      {/* Main Content */}
      {/* ✅ yaha flex-1 add kiya */}
      <main className="flex-1 max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 py-8">
        <div className="grid grid-cols-1 lg:grid-cols-4 gap-6">
          <div className={activeTab === 'parsing' ? 'lg:col-span-4' : 'lg:col-span-3'}>
            {tabs.find((tab) => tab.id === activeTab)?.component}
          </div>

          {activeTab !== 'parsing' && (
          <div className="lg:col-span-1">
              {selectedSymbol && (
                <div className="bg-white dark:bg-gray-800 rounded-lg shadow-lg p-6 mb-6">
                  <h3 className="text-lg font-semibold mb-4 text-gray-900 dark:text-white">
                    Symbol Details
                  </h3>
                  <div className="space-y-3">
                    <div>
                      <label className="text-sm font-medium text-gray-500 dark:text-gray-400">
                        Name
                      </label>
                      <p className="text-gray-900 dark:text-white font-mono">
                        {selectedSymbol.name}
                      </p>
                    </div>
                    <div>
                      <label className="text-sm font-medium text-gray-500 dark:text-gray-400">
                        Type
                      </label>
                      <p className="text-gray-900 dark:text-white">
                        {selectedSymbol.data_type}
                      </p>
                    </div>
                    <div>
                      <label className="text-sm font-medium text-gray-500 dark:text-gray-400">
                        Kind
                      </label>
                      <p className="text-gray-900 dark:text-white">
                        {selectedSymbol.kind}
                      </p>
                    </div>
                    <div>
                      <label className="text-sm font-medium text-gray-500 dark:text-gray-400">
                        Scope
                      </label>
                      <p className="text-gray-900 dark:text-white">
                        {selectedSymbol.scope_name}
                      </p>
                    </div>
                    <div>
                      <label className="text-sm font-medium text-gray-500 dark:text-gray-400">
                        Line
                      </label>
                      <p className="text-gray-900 dark:text-white">
                        {selectedSymbol.line_number}
                      </p>
                    </div>
                    <div>
                      <label className="text-sm font-medium text-gray-500 dark:text-gray-400">
                        Storage Class
                      </label>
                      <p className="text-gray-900 dark:text-white">
                        {selectedSymbol.storage_class}
                      </p>
                    </div>
                    <div>
                      <label className="text-sm font-medium text-gray-500 dark:text-gray-400">
                        Initialized
                      </label>
                      <p className="text-gray-900 dark:text-white">
                        {selectedSymbol.is_initialized ? 'Yes' : 'No'}
                      </p>
                    </div>
                    {selectedSymbol.initial_value && (
                      <div>
                        <label className="text-sm font-medium text-gray-500 dark:text-gray-400">
                          Initial Value
                        </label>
                        <p className="text-gray-900 dark:text-white font-mono bg-gray-100 dark:bg-gray-700 px-2 py-1 rounded">
                          {selectedSymbol.initial_value}
                        </p>
                      </div>
                    )}
                  </div>
                </div>
              )}

              <div className="bg-white dark:bg-gray-800 rounded-lg shadow-xl border border-gray-100 dark:border-gray-700 p-6">
                <h3 className="text-lg font-bold mb-5 flex items-center gap-2 text-gray-900 dark:text-white border-b border-gray-100 dark:border-gray-700 pb-3">
                  <Hash className="h-5 w-5 text-primary-500" />
                  Quick Stats
                </h3>
                <div className="space-y-4">
                  <div className="flex justify-between items-center">
                    <span className="text-sm text-gray-500 dark:text-gray-400">Analysis State</span>
                    <div className="flex gap-2">
                      <span title="Errors" className="flex items-center gap-1 text-[10px] font-bold px-2 py-0.5 rounded bg-red-100 text-red-700 dark:bg-red-900/30 dark:text-red-400">
                        🔴 {problems.filter(p => p.severity === 'error').length}
                      </span>
                      <span title="Warnings" className="flex items-center gap-1 text-[10px] font-bold px-2 py-0.5 rounded bg-amber-100 text-amber-700 dark:bg-amber-900/30 dark:text-amber-400">
                        🟡 {problems.filter(p => p.severity === 'warning').length}
                      </span>
                      <span title="Info/Tips" className="flex items-center gap-1 text-[10px] font-bold px-2 py-0.5 rounded bg-blue-100 text-blue-700 dark:bg-blue-900/30 dark:text-blue-400">
                        🔵 {problems.filter(p => p.severity !== 'error' && p.severity !== 'warning').length}
                      </span>
                    </div>
                  </div>
                  <div className="flex justify-between">
                    <span className="text-sm text-gray-500 dark:text-gray-400">
                      Current Scope
                    </span>
                    <span className="text-sm font-bold text-gray-900 dark:text-white font-mono bg-gray-50 dark:bg-gray-700 px-2 py-0.5 rounded border border-gray-100 dark:border-gray-600">
                      {currentScope}
                    </span>
                  </div>
                  <div className="flex justify-between">
                    <span className="text-sm text-gray-500 dark:text-gray-400">
                      Socket Status
                    </span>
                    <span
                      className={`text-sm font-bold flex items-center gap-2 ${wsConnected ? 'text-green-600' : 'text-red-600'
                        }`}
                    >
                      <div className={`h-2 w-2 rounded-full ${wsConnected ? 'bg-green-600 animate-pulse' : 'bg-red-600'}`} />
                      {wsConnected ? 'Live' : 'Offline'}
                    </span>
                  </div>
                  <div className="flex justify-between">
                    <span className="text-sm text-gray-500 dark:text-gray-400">
                      UI Mode
                    </span>
                    <span className="text-sm font-bold text-gray-900 dark:text-white">
                      {darkMode ? 'Dark Engine' : 'Light Classic'}
                    </span>
                  </div>
                </div>
              </div>
          </div>
          )}
        </div>
      </main>

      {/* Footer */}
      <footer className="bg-white dark:bg-gray-800 border-t border-gray-200 dark:border-gray-700">
        <div className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 py-2">
          <div className="text-center text-sm text-gray-500 dark:text-gray-400">
            <p>Symbol Table Visualizer - Interactive Compiler Design Tool</p>
            <p className="mt-1">Built with React, FastAPI, and C</p>
          </div>
        </div>
      </footer>
    </div>
  );
}

export default App;
