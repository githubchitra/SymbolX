import React, { useState, useEffect } from 'react';
import { AlertTriangle, AlertCircle, X, CheckCircle, Info, Bug, FileText, Code } from 'lucide-react';

const AdvancedErrorDetection = ({ code, parseResults, symbols, onErrorsUpdate }) => {
  const [errors, setErrors] = useState([]);
  const [warnings, setWarnings] = useState([]);
  const [suggestions, setSuggestions] = useState([]);
  const [selectedError, setSelectedError] = useState(null);
  const [analysisComplete, setAnalysisComplete] = useState(false);

  useEffect(() => {
    if (code) {
      analyzeCode();
    }
  }, [code, parseResults, symbols]);

  useEffect(() => {
    if (onErrorsUpdate) {
      onErrorsUpdate({ errors, warnings, suggestions });
    }
  }, [errors, warnings, suggestions]);

  const analyzeCode = () => {
    const detectedErrors = [];
    const detectedWarnings = [];
    const detectedSuggestions = [];

    // 1. Syntax and Basic Errors
    detectSyntaxErrors(code, detectedErrors);
    
    // 2. Semantic Errors
    detectSemanticErrors(code, symbols, detectedErrors, detectedWarnings);
    
    // 3. Logic Errors
    detectLogicErrors(code, parseResults, detectedErrors, detectedWarnings);
    
    // 4. Style and Best Practice Issues
    detectStyleIssues(code, detectedWarnings, detectedSuggestions);
    
    // 5. Performance Issues
    detectPerformanceIssues(code, detectedWarnings, detectedSuggestions);
    
    // 6. Security Vulnerabilities
    detectSecurityIssues(code, detectedErrors, detectedWarnings);

    setErrors(detectedErrors);
    setWarnings(detectedWarnings);
    setSuggestions(detectedSuggestions);
    setAnalysisComplete(true);
  };

  const detectSyntaxErrors = (code, errorList) => {
    const lines = code.split('\n');
    
    lines.forEach((line, index) => {
      const lineNum = index + 1;
      const trimmed = line.trim();
      
      // Missing semicolons
      if (trimmed && !trimmed.startsWith('//') && !trimmed.includes('{') && 
          !trimmed.includes('}') && !trimmed.endsWith(';') && 
          !trimmed.includes('#') && !trimmed.includes('if') && 
          !trimmed.includes('for') && !trimmed.includes('while') &&
          !trimmed.includes('switch') && !trimmed.includes('case') &&
          !trimmed.includes('default') && !trimmed.includes('return') &&
          !trimmed.includes('break') && !trimmed.includes('continue') &&
          !trimmed.includes('goto') && !trimmed.includes('do') &&
          !trimmed.includes('else')) {
        
        errorList.push({
          type: 'error',
          severity: 'high',
          category: 'syntax',
          line: lineNum,
          column: line.length,
          message: 'Missing semicolon',
          description: 'Statements in C should end with a semicolon',
          suggestion: 'Add a semicolon at the end of this statement',
          code: line.trim()
        });
      }

      // Unmatched brackets
      const openBraces = (line.match(/{/g) || []).length;
      const closeBraces = (line.match(/}/g) || []).length;
      const openParens = (line.match(/\(/g) || []).length;
      const closeParens = (line.match(/\)/g) || []).length;
      const openBrackets = (line.match(/\[/g) || []).length;
      const closeBrackets = (line.match(/\]/g) || []).length;

      if (openBraces > closeBraces) {
        errorList.push({
          type: 'error',
          severity: 'high',
          category: 'syntax',
          line: lineNum,
          message: 'Unmatched opening brace',
          description: 'Missing closing brace',
          suggestion: 'Add a closing brace to match the opening brace',
          code: line.trim()
        });
      }

      if (openParens > closeParens) {
        errorList.push({
          type: 'error',
          severity: 'high',
          category: 'syntax',
          line: lineNum,
          message: 'Unmatched opening parenthesis',
          description: 'Missing closing parenthesis',
          suggestion: 'Add a closing parenthesis to match the opening parenthesis',
          code: line.trim()
        });
      }
    });
  };

  const detectSemanticErrors = (code, symbols, errorList, warningList) => {
    const lines = code.split('\n');
    const symbolNames = new Set(symbols.map(s => s.name));
    
    lines.forEach((line, index) => {
      const lineNum = index + 1;
      
      // Check for undefined variables
      const variablePattern = /\b([a-zA-Z_][a-zA-Z0-9_]*)\b/g;
      let match;
      while ((match = variablePattern.exec(line)) !== null) {
        const varName = match[1];
        
        // Skip keywords and types
        const keywords = ['int', 'float', 'double', 'char', 'void', 'if', 'else', 'for', 'while', 
                         'do', 'switch', 'case', 'default', 'break', 'continue', 'return', 
                         'goto', 'sizeof', 'struct', 'union', 'enum', 'typedef', 'const', 
                         'static', 'extern', 'auto', 'register', 'signed', 'unsigned'];
        
        if (!keywords.includes(varName) && !symbolNames.has(varName) && 
            !line.includes(varName + ' ') && !line.includes(' ' + varName) &&
            !line.includes('*' + varName) && !line.includes(varName + '[') &&
            !line.includes(varName + '(')) {
          
          errorList.push({
            type: 'error',
            severity: 'high',
            category: 'semantic',
            line: lineNum,
            message: `Undefined variable: ${varName}`,
            description: `Variable '${varName}' is used but not declared`,
            suggestion: `Declare variable '${varName}' before using it`,
            code: line.trim()
          });
        }
      }

      // Check for type mismatches (simplified)
      if (line.includes('=') && line.includes('+')) {
        const parts = line.split('=');
        if (parts.length >= 2) {
          const leftSide = parts[0].trim();
          const rightSide = parts[1].trim();
          
          if (leftSide.includes('int') && rightSide.includes('"')) {
            errorList.push({
              type: 'error',
              severity: 'high',
              category: 'semantic',
              line: lineNum,
              message: 'Type mismatch',
              description: 'Cannot assign string value to integer variable',
              suggestion: 'Use string type or convert string to integer',
              code: line.trim()
            });
          }
        }
      }
    });
  };

  const detectLogicErrors = (code, parseResults, errorList, warningList) => {
    const lines = code.split('\n');
    
    lines.forEach((line, index) => {
      const lineNum = index + 1;
      const trimmed = line.trim();
      
      // Infinite loops
      if (trimmed.includes('for') && trimmed.includes(';') && trimmed.split(';').length >= 3) {
        const parts = trimmed.split(';');
        if (parts[1].trim() === '' && parts[2].trim() === '') {
          errorList.push({
            type: 'error',
            severity: 'high',
            category: 'logic',
            line: lineNum,
            message: 'Potential infinite loop',
            description: 'For loop with empty condition may cause infinite loop',
            suggestion: 'Add a proper loop condition or break statement',
            code: line.trim()
          });
        }
      }

      // Unreachable code
      if (trimmed.includes('return') && index < lines.length - 1) {
        const nextLine = lines[index + 1].trim();
        if (nextLine && !nextLine.startsWith('}') && !nextLine.startsWith('//')) {
          warningList.push({
            type: 'warning',
            severity: 'medium',
            category: 'logic',
            line: lineNum + 1,
            message: 'Unreachable code',
            description: 'Code after return statement will never execute',
            suggestion: 'Move or remove the unreachable code',
            code: nextLine
          });
        }
      }

      // Division by zero risk
      if (trimmed.includes('/') && !trimmed.includes('//')) {
        const parts = trimmed.split('/');
        if (parts.length > 1) {
          const divisor = parts[1].trim().split(/[+\-*/]/)[0].trim();
          if (divisor === '0') {
            errorList.push({
              type: 'error',
              severity: 'high',
              category: 'logic',
              line: lineNum,
              message: 'Division by zero',
              description: 'Division by zero will cause runtime error',
              suggestion: 'Check divisor before performing division',
              code: line.trim()
            });
          }
        }
      }
    });
  };

  const detectStyleIssues = (code, warningList, suggestionList) => {
    const lines = code.split('\n');
    
    lines.forEach((line, index) => {
      const lineNum = index + 1;
      const trimmed = line.trim();
      
      // Long lines
      if (line.length > 80) {
        warningList.push({
          type: 'warning',
          severity: 'low',
          category: 'style',
          line: lineNum,
          message: 'Line too long',
          description: 'Line exceeds 80 characters',
          suggestion: 'Break long lines into multiple lines for better readability',
          code: line.trim()
        });
      }

      // Magic numbers
      const numberPattern = /\b\d{2,}\b/g;
      let match;
      while ((match = numberPattern.exec(line)) !== null) {
        const number = match[1] || match[0];
        if (number !== '0' && number !== '1') {
          suggestionList.push({
            type: 'suggestion',
            severity: 'low',
            category: 'style',
            line: lineNum,
            message: 'Magic number detected',
            description: `Consider replacing magic number ${number} with a named constant`,
            suggestion: 'Define a constant for this number',
            code: line.trim()
          });
        }
      }

      // Missing comments for functions
      if (trimmed.includes('int') || trimmed.includes('void') || trimmed.includes('float')) {
        const prevLine = index > 0 ? lines[index - 1].trim() : '';
        if (!prevLine.startsWith('//') && !prevLine.startsWith('/*')) {
          suggestionList.push({
            type: 'suggestion',
            severity: 'low',
            category: 'style',
            line: lineNum,
            message: 'Missing function comment',
            description: 'Functions should have comments explaining their purpose',
            suggestion: 'Add a comment above the function describing its purpose and parameters',
            code: line.trim()
          });
        }
      }
    });
  };

  const detectPerformanceIssues = (code, warningList, suggestionList) => {
    const lines = code.split('\n');
    
    lines.forEach((line, index) => {
      const lineNum = index + 1;
      const trimmed = line.trim();
      
      // Inefficient loops
      if (trimmed.includes('for') && trimmed.includes('strlen')) {
        warningList.push({
          type: 'warning',
          severity: 'medium',
          category: 'performance',
          line: lineNum,
          message: 'Inefficient loop condition',
          description: 'Calling strlen() in loop condition is inefficient',
          suggestion: 'Calculate string length once before the loop',
          code: line.trim()
        });
      }

      // Memory leaks risk
      if (trimmed.includes('malloc') || trimmed.includes('calloc')) {
        const remainingLines = lines.slice(index + 1);
        const hasFree = remainingLines.some(l => l.includes('free'));
        if (!hasFree) {
          warningList.push({
            type: 'warning',
            severity: 'medium',
            category: 'performance',
            line: lineNum,
            message: 'Potential memory leak',
            description: 'Allocated memory may not be freed',
            suggestion: 'Ensure allocated memory is freed when no longer needed',
            code: line.trim()
          });
        }
      }
    });
  };

  const detectSecurityIssues = (code, errorList, warningList) => {
    const lines = code.split('\n');
    
    lines.forEach((line, index) => {
      const lineNum = index + 1;
      const trimmed = line.trim();
      
      // Buffer overflow risks
      if (trimmed.includes('gets(')) {
        errorList.push({
          type: 'error',
          severity: 'critical',
          category: 'security',
          line: lineNum,
          message: 'Dangerous function usage',
          description: 'gets() is dangerous and can cause buffer overflow',
          suggestion: 'Use fgets() instead of gets()',
          code: line.trim()
        });
      }

      if (trimmed.includes('strcpy(') || trimmed.includes('strcat(')) {
        warningList.push({
          type: 'warning',
          severity: 'high',
          category: 'security',
          line: lineNum,
          message: 'Potential buffer overflow',
          description: 'String functions without size limits can cause buffer overflow',
          suggestion: 'Use strncpy() or strncat() with size limits',
          code: line.trim()
        });
      }

      // Format string vulnerabilities
      if (trimmed.includes('printf') && !trimmed.includes('%')) {
        const parts = trimmed.split('printf');
        if (parts.length > 1 && parts[1].includes('(') && parts[1].includes(')')) {
          const args = parts[1].substring(parts[1].indexOf('(') + 1, parts[1].indexOf(')'));
          if (!args.includes('"') && args.includes(',')) {
            errorList.push({
              type: 'error',
              severity: 'high',
              category: 'security',
              line: lineNum,
              message: 'Format string vulnerability',
              description: 'User input in format string can lead to security vulnerabilities',
              suggestion: 'Use format string with proper specifiers',
              code: line.trim()
            });
          }
        }
      }
    });
  };

  const getIssueIcon = (type, severity) => {
    switch (type) {
      case 'error':
        return severity === 'critical' ? 
          <AlertTriangle className="h-4 w-4 text-red-600" /> : 
          <AlertCircle className="h-4 w-4 text-red-500" />;
      case 'warning':
        return <AlertTriangle className="h-4 w-4 text-yellow-500" />;
      case 'suggestion':
        return <Info className="h-4 w-4 text-blue-500" />;
      default:
        return <Bug className="h-4 w-4 text-gray-500" />;
    }
  };

  const getSeverityColor = (severity) => {
    switch (severity) {
      case 'critical': return 'bg-red-100 border-red-300 text-red-800';
      case 'high': return 'bg-red-50 border-red-200 text-red-700';
      case 'medium': return 'bg-yellow-50 border-yellow-200 text-yellow-700';
      case 'low': return 'bg-blue-50 border-blue-200 text-blue-700';
      default: return 'bg-gray-50 border-gray-200 text-gray-700';
    }
  };

  const IssueItem = ({ issue, index }) => (
    <div
      className={`p-3 border rounded-lg cursor-pointer transition-all duration-200 ${
        selectedError === index ? 'ring-2 ring-blue-500' : ''
      } ${getSeverityColor(issue.severity)}`}
      onClick={() => setSelectedError(selectedError === index ? null : index)}
    >
      <div className="flex items-start gap-3">
        {getIssueIcon(issue.type, issue.severity)}
        <div className="flex-1 min-w-0">
          <div className="flex items-center gap-2 mb-1">
            <span className="font-medium text-sm">{issue.message}</span>
            <span className="text-xs bg-gray-200 px-2 py-1 rounded">
              {issue.category}
            </span>
          </div>
          <div className="text-xs text-gray-600 mb-1">
            Line {issue.line}: {issue.description}
          </div>
          {issue.suggestion && (
            <div className="text-xs bg-white p-2 rounded border">
              <strong>Suggestion:</strong> {issue.suggestion}
            </div>
          )}
          {issue.code && (
            <div className="mt-2 text-xs font-mono bg-gray-100 p-2 rounded">
              {issue.code}
            </div>
          )}
        </div>
        <div className="text-xs text-gray-500">
          {issue.severity.toUpperCase()}
        </div>
      </div>
    </div>
  );

  return (
    <div className="bg-white rounded-lg shadow-lg overflow-hidden">
      {/* Header */}
      <div className="bg-gradient-to-r from-red-600 to-orange-600 text-white p-6">
        <h2 className="text-xl font-bold mb-2">Advanced Error Detection</h2>
        <p className="text-red-100 text-sm">
          Comprehensive code analysis with error detection and suggestions
        </p>
      </div>

      {/* Summary */}
      <div className="border-b border-gray-200 p-4">
        <div className="grid grid-cols-3 gap-4">
          <div className="text-center">
            <div className="text-2xl font-bold text-red-600">{errors.length}</div>
            <div className="text-sm text-gray-600">Errors</div>
          </div>
          <div className="text-center">
            <div className="text-2xl font-bold text-yellow-600">{warnings.length}</div>
            <div className="text-sm text-gray-600">Warnings</div>
          </div>
          <div className="text-center">
            <div className="text-2xl font-bold text-blue-600">{suggestions.length}</div>
            <div className="text-sm text-gray-600">Suggestions</div>
          </div>
        </div>
      </div>

      {/* Issues List */}
      <div className="p-4 max-h-96 overflow-y-auto">
        {analysisComplete ? (
          <div className="space-y-3">
            {errors.length > 0 && (
              <div>
                <h3 className="font-medium text-red-800 mb-2 flex items-center gap-2">
                  <AlertCircle className="h-4 w-4" />
                  Errors ({errors.length})
                </h3>
                <div className="space-y-2">
                  {errors.map((error, index) => (
                    <IssueItem key={`error-${index}`} issue={error} index={index} />
                  ))}
                </div>
              </div>
            )}

            {warnings.length > 0 && (
              <div>
                <h3 className="font-medium text-yellow-800 mb-2 flex items-center gap-2">
                  <AlertTriangle className="h-4 w-4" />
                  Warnings ({warnings.length})
                </h3>
                <div className="space-y-2">
                  {warnings.map((warning, index) => (
                    <IssueItem 
                      key={`warning-${index}`} 
                      issue={warning} 
                      index={errors.length + index} 
                    />
                  ))}
                </div>
              </div>
            )}

            {suggestions.length > 0 && (
              <div>
                <h3 className="font-medium text-blue-800 mb-2 flex items-center gap-2">
                  <Info className="h-4 w-4" />
                  Suggestions ({suggestions.length})
                </h3>
                <div className="space-y-2">
                  {suggestions.map((suggestion, index) => (
                    <IssueItem 
                      key={`suggestion-${index}`} 
                      issue={suggestion} 
                      index={errors.length + warnings.length + index} 
                    />
                  ))}
                </div>
              </div>
            )}

            {errors.length === 0 && warnings.length === 0 && suggestions.length === 0 && (
              <div className="text-center py-8 text-gray-500">
                <CheckCircle className="h-12 w-12 mx-auto mb-4 text-green-500" />
                <p className="font-medium text-green-600">No issues detected!</p>
                <p className="text-sm">Your code looks clean and well-structured.</p>
              </div>
            )}
          </div>
        ) : (
          <div className="text-center py-8 text-gray-500">
            <Bug className="h-12 w-12 mx-auto mb-4 text-gray-300" />
            <p>Analyzing code...</p>
          </div>
        )}
      </div>
    </div>
  );
};

export default AdvancedErrorDetection;
