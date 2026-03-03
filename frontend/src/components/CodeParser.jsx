import React, { useState, useRef, useImperativeHandle, forwardRef } from 'react';
import { Play, Pause, SkipForward, RefreshCw, Code, AlertCircle, CheckCircle, Save, AlertTriangle } from 'lucide-react';
import { parseAPI } from '../services/api';

const DEFAULT_CODE = ``;

const CodeParser = forwardRef(({
  code,
  setCode,
  savedCode,
  setSavedCode,
  onParseComplete,
  onStepComplete,
  onReset,
  problems = []
}, ref) => {
  const [isParsing, setIsParsing] = useState(false);
  const [isStepping, setIsStepping] = useState(false);
  const [currentStep, setCurrentStep] = useState(0);
  const [parseResults, setParseResults] = useState([]);
  const [errors, setErrors] = useState([]);
  const [showSaveFeedback, setShowSaveFeedback] = useState(false);
  const textareaRef = useRef(null);

  useImperativeHandle(ref, () => ({
    jumpToLine: (line) => {
      const lines = code.split('\n');
      let pos = 0;
      for (let i = 0; i < line - 1; i++) {
        pos += lines[i].length + 1; // +1 for \n
      }

      if (textareaRef.current) {
        textareaRef.current.focus();
        textareaRef.current.setSelectionRange(pos, pos + (lines[line - 1]?.length || 0));

        // Scroll to line (approximate)
        const lineHeight = 24; // text-sm leading-6
        textareaRef.current.scrollTop = Math.max(0, (line - 5) * lineHeight);
      }
    }
  }));

  const handleSave = () => {
    setSavedCode(code);
    setShowSaveFeedback(true);
    setTimeout(() => setShowSaveFeedback(false), 2000);
  };

  const handleParse = async (stepByStep = false) => {
    if (!savedCode.trim()) {
      if (code.trim()) {
        setErrors(['Please save your code first (click the Save button).']);
      } else {
        setErrors(['Please enter some code and save it first.']);
      }
      return;
    }

    try {
      setIsParsing(true);
      setErrors([]);

      const data = await parseAPI.parseCode(savedCode, stepByStep);

      if (stepByStep) {
        setParseResults(data.results || []);
        setCurrentStep(0);
        setIsStepping(true);
      } else {
        setParseResults(data.results || []);
        setIsStepping(false);
      }

      if (data.errors > 0) {
        setErrors([`${data.errors} parsing errors detected`]);
      }

      onParseComplete && onParseComplete(data);
    } catch (error) {
      console.error('Error parsing code:', error);
      setErrors([error.message]);
    } finally {
      setIsParsing(false);
    }
  };

  const handleStep = async () => {
    if (currentStep < parseResults.length - 1) {
      const step = parseResults[currentStep];
      setCurrentStep(currentStep + 1);
      onStepComplete && onStepComplete(step, currentStep);
    } else {
      setIsStepping(false);
    }
  };

  const handleReset = () => {
    setCurrentStep(0);
    setParseResults([]);
    setErrors([]);
    setIsStepping(false);
    onReset && onReset();
  };

  const getStepIcon = (step) => {
    switch (step.action) {
      case 'declared_and_initialized':
        return <CheckCircle className="h-4 w-4 text-green-600" />;
      case 'declared':
        return <CheckCircle className="h-4 w-4 text-blue-600" />;
      case 'error':
        return <AlertCircle className="h-4 w-4 text-red-600" />;
      default:
        return <Code className="h-4 w-4 text-blue-600" />;
    }
  };

  return (
    <div className="bg-white rounded-lg shadow-lg overflow-hidden">
      {/* Header */}
      <div className="bg-gradient-to-r from-blue-600 to-blue-700 text-white p-6">
        <h2 className="text-xl font-bold mb-2">Code Parser</h2>
        <p className="text-blue-100 text-sm">
          Parse C code and watch the symbol table build in real-time
        </p>
      </div>

      {/* Controls */}
      <div className="border-b border-gray-200 p-4">
        <div className="flex items-center justify-between mb-4">
          <div className="flex items-center gap-2">
            <button
              onClick={() => handleParse(false)}
              disabled={isParsing || !code.trim()}
              className="px-4 py-2 bg-primary-600 text-white rounded-lg hover:bg-primary-700 disabled:bg-gray-400 disabled:cursor-not-allowed transition-colors flex items-center gap-2"
            >
              <Play className="h-4 w-4" />
              Parse All
            </button>

            <button
              onClick={() => handleParse(true)}
              disabled={isParsing || !code.trim()}
              className="px-4 py-2 bg-amber-600 text-white rounded-lg hover:bg-amber-700 disabled:bg-gray-400 disabled:cursor-not-allowed transition-colors flex items-center gap-2"
            >
              <Play className="h-4 w-4" />
              Step Through
            </button>

            {isStepping && (
              <>
                <button
                  onClick={handleStep}
                  disabled={currentStep >= parseResults.length - 1}
                  className="px-3 py-2 bg-green-600 text-white rounded-lg hover:bg-green-700 disabled:bg-gray-400 disabled:cursor-not-allowed transition-colors flex items-center gap-2"
                >
                  <SkipForward className="h-4 w-4" />
                  Next Step
                </button>

                <button
                  onClick={() => setIsStepping(false)}
                  className="px-3 py-2 bg-red-600 text-white rounded-lg hover:bg-red-700 transition-colors flex items-center gap-2"
                >
                  <Pause className="h-4 w-4" />
                  Stop
                </button>
              </>
            )}

            <button
              onClick={handleReset}
              className="px-3 py-2 bg-gray-600 text-white rounded-lg hover:bg-gray-700 transition-colors flex items-center gap-2"
            >
              <RefreshCw className="h-4 w-4" />
              Reset
            </button>

            <button
              onClick={handleSave}
              className={`px-3 py-2 rounded-lg transition-colors flex items-center gap-2 ${showSaveFeedback
                ? 'bg-green-600 hover:bg-green-700 text-white'
                : 'bg-indigo-600 hover:bg-indigo-700 text-white'
                }`}
            >
              {showSaveFeedback ? <CheckCircle className="h-4 w-4" /> : <Save className="h-4 w-4" />}
              {showSaveFeedback ? 'Saved!' : 'Save'}
            </button>
          </div>

          {isStepping && (
            <div className="text-sm text-gray-600">
              Step {currentStep + 1} of {parseResults.length}
            </div>
          )}
        </div>

        {/* Progress Bar */}
        {isStepping && parseResults.length > 0 && (
          <div className="w-full bg-gray-200 rounded-full h-2">
            <div
              className="bg-primary-600 h-2 rounded-full transition-all duration-300"
              style={{ width: `${((currentStep + 1) / parseResults.length) * 100}%` }}
            ></div>
          </div>
        )}
      </div>

      <div className="grid grid-cols-1 lg:grid-cols-2 gap-6 p-6">
        {/* Code Editor */}
        <div>
          <h3 className="text-lg font-semibold mb-3 text-gray-800">Code Input</h3>
          <div className="relative">
            <textarea
              ref={textareaRef}
              value={code}
              onChange={(e) => setCode(e.target.value)}
              className="w-full h-96 py-4 pr-4 pl-12 font-mono text-sm leading-6 border border-gray-300 rounded-lg focus:ring-2 focus:ring-primary-500 focus:border-transparent resize-none"
              placeholder="Enter C code to parse..."
              spellCheck={false}
            />

            {/* Line numbers overlay */}
            <div className="absolute left-0 top-0 w-12 h-96 py-4 pr-2 font-mono text-sm leading-6 text-gray-400 pointer-events-none overflow-hidden text-right bg-slate-50 border-r border-slate-200">
              {code.split('\n').map((_, index) => {
                const lineNum = index + 1;
                const lineProblems = problems.filter(p => p.line === lineNum);
                const hasError = lineProblems.some(p => p.severity === 'error');
                const hasWarning = lineProblems.some(p => p.severity === 'warning');

                return (
                  <div key={index} className="relative group cursor-help">
                    {index + 1}
                    {hasError && <div className="absolute right-0 top-1 w-1 h-4 bg-red-500 rounded-l" />}
                    {hasWarning && !hasError && <div className="absolute right-0 top-1 w-1 h-4 bg-amber-500 rounded-l" />}

                    {/* Diagnostic Tooltip */}
                    {lineProblems.length > 0 && (
                      <div className="absolute left-full ml-2 top-0 z-50 invisible group-hover:visible w-64 p-2 bg-gray-900 text-white text-[10px] rounded shadow-xl text-left pointer-events-none">
                        <div className="font-bold flex items-center gap-1 mb-1">
                          {hasError ? <AlertCircle className="h-2 w-2 text-red-400" /> : <AlertTriangle className="h-2 w-2 text-amber-400" />}
                          {hasError ? 'Error' : 'Warning'}
                        </div>
                        {lineProblems[0].message}
                        {lineProblems.length > 1 && <div className="mt-1 text-gray-400">(+{lineProblems.length - 1} more)</div>}
                      </div>
                    )}
                  </div>
                );
              })}
            </div>
          </div>
        </div>

        {/* Parse Results */}
        <div>
          <h3 className="text-lg font-semibold mb-3 text-gray-800">Parse Results</h3>

          {errors.length > 0 && (
            <div className="mb-4 p-3 bg-red-50 border border-red-200 rounded-lg">
              <div className="flex items-center gap-2 text-red-800">
                <AlertCircle className="h-4 w-4" />
                <span className="font-medium">Errors:</span>
              </div>
              <ul className="mt-2 text-sm text-red-700">
                {errors.map((error, index) => (
                  <li key={index}>• {error}</li>
                ))}
              </ul>
            </div>
          )}

          <div className="h-96 overflow-y-auto border border-gray-200 rounded-lg">
            {parseResults.length > 0 ? (
              <div className="divide-y divide-gray-200">
                {parseResults.map((result, index) => (
                  <div
                    key={index}
                    className={`p-3 hover:bg-gray-50 transition-colors ${index === currentStep ? 'bg-yellow-50 border-l-4 border-yellow-500' : ''
                      } ${index < currentStep ? 'bg-green-50' : ''}`}
                  >
                    <div className="flex items-start gap-3">
                      <div className="mt-1">
                        {getStepIcon(result)}
                      </div>
                      <div className="flex-1">
                        <div className="flex items-center gap-2 mb-1">
                          <span className="text-sm font-medium text-gray-900">
                            Line {result.line}
                          </span>
                          {result.action === 'declared_and_initialized' && (
                            <span className="inline-flex items-center px-2 py-1 text-xs font-medium text-green-800 bg-green-100 rounded-full">
                              {result.symbol}
                            </span>
                          )}
                          {result.action === 'declared' && (
                            <span className="inline-flex items-center px-2 py-1 text-xs font-medium text-blue-800 bg-blue-100 rounded-full">
                              {result.symbol}
                            </span>
                          )}
                        </div>
                        <div className="text-sm text-gray-600">
                          {result.action === 'declared_and_initialized' && (
                            <>Declared and initialized <span className="font-mono font-medium">{result.symbol}</span> as <span className="font-mono">{result.type}</span></>
                          )}
                          {result.action === 'declared' && (
                            <>Declared <span className="font-mono font-medium">{result.symbol}</span> as <span className="font-mono">{result.type}</span></>
                          )}
                          {result.action === 'error' && (
                            <span className="text-red-600">{result.message}</span>
                          )}
                        </div>
                      </div>
                    </div>
                  </div>
                ))}
              </div>
            ) : (
              <div className="flex items-center justify-center h-full text-gray-500">
                <div className="text-center">
                  <Code className="h-12 w-12 mx-auto mb-4 text-gray-300" />
                  <p>No parsing results yet</p>
                  <p className="text-sm mt-2">Enter code and click "Parse" to see results</p>
                </div>
              </div>
            )}
          </div>
        </div>
      </div>

      {/* Instructions */}
      <div className="border-t border-gray-200 p-4 bg-gray-50">
        <div className="text-sm text-gray-600">
          <h4 className="font-medium mb-2">Instructions:</h4>
          <ul className="space-y-1">
            <li>• Enter C code in the editor on the left</li>
            <li>• Click "Parse All" to parse the entire code at once</li>
            <li>• Click "Step Through" to parse line by line with animations</li>
            <li>• Watch the symbol table update in real-time</li>
            <li>• Errors will be highlighted in red</li>
          </ul>
        </div>
      </div>
    </div>
  );
});

export default CodeParser;
