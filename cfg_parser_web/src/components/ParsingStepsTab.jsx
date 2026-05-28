import React, { useState } from 'react';
import { Download } from 'lucide-react';
import { jsPDF } from 'jspdf';

const ParsingStepsTab = ({ parseResult }) => {
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

  // Generate ASCII parse tree
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
    const derivation = [];
    let current = parseResult.parseTree;
    
    if (!current) return derivation;
    
    // Simple derivation based on parsing steps
    derivation.push(current.symbol);
    
    steps.forEach((step, index) => {
      if (step.action.includes('Applied rule')) {
        // Extract the production from the action
        const match = step.action.match(/Applied rule: (.+)/);
        if (match) {
          derivation.push(match[1]);
        }
      }
    });
    
    return derivation;
  };

  const derivation = generateDerivation();
  const asciiTree = parseResult.parseTree ? generateASCIITree(parseResult.parseTree) : '';

  const handleExportPDF = () => {
    const doc = new jsPDF();
    
    doc.setFontSize(16);
    doc.text('Parsing Results', 20, 20);
    
    doc.setFontSize(14);
    doc.text('Parse Tree', 20, 35);
    
    doc.setFontSize(10);
    let yPosition = 45;
    
    const treeLines = asciiTree.split('\n');
    treeLines.forEach(line => {
      if (yPosition > 270) {
        doc.addPage();
        yPosition = 20;
      }
      doc.text(line, 25, yPosition);
      yPosition += 6;
    });
    
    yPosition += 10;
    doc.setFontSize(14);
    doc.text('Step-by-step Derivation', 20, yPosition);
    yPosition += 10;
    
    doc.setFontSize(10);
    derivation.forEach((step, index) => {
      if (yPosition > 270) {
        doc.addPage();
        yPosition = 20;
      }
      
      if (index === 0) {
        doc.text(step, 25, yPosition);
      } else {
        doc.text('→ ' + step, 25, yPosition);
      }
      yPosition += 8;
    });
    
    doc.save('parsing-results.pdf');
  };

  return (
    <div className="space-y-6">
      {/* Parse Result Status */}
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

      {/* Export Button */}
      <div className="flex justify-end">
        <button
          onClick={handleExportPDF}
          className="btn-outline flex items-center gap-2"
        >
          <Download className="w-4 h-4" />
          Export PDF
        </button>
      </div>

      {/* Parse Tree */}
      <div className="card">
        <h3 className="font-semibold text-gray-900 dark:text-white mb-4">Parse Tree</h3>
        <div className="bg-gray-50 dark:bg-gray-800 rounded-lg p-6 overflow-x-auto">
          <pre className="font-mono text-sm text-gray-700 dark:text-gray-300 whitespace-pre">
            {asciiTree}
          </pre>
        </div>
      </div>

      {/* Step-by-step Derivation */}
      <div className="card">
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

      {/* Parsing Steps Table */}
      <div className="card">
        <h3 className="font-semibold text-gray-900 dark:text-white mb-4">Parsing Steps</h3>
        <div className="bg-gray-50 dark:bg-gray-800 rounded-lg p-4 max-h-64 overflow-y-auto scrollbar-thin">
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

export default ParsingStepsTab;
