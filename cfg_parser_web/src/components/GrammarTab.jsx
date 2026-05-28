import React from 'react';

const GrammarTab = ({ grammar }) => {
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
      {/* Grammar Info */}
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

      {/* Non-terminals */}
      <div>
        <h3 className="font-semibold text-gray-900 dark:text-white mb-3">Non-terminals</h3>
        <div className="flex flex-wrap gap-2">
          {[...grammar.nonTerminals].sort().map((nt) => (
            <span
              key={nt}
              className="px-3 py-1 bg-secondary-100 dark:bg-secondary-900/30 text-secondary-800 dark:text-secondary-300 rounded-full text-sm font-medium"
            >
              {nt}
            </span>
          ))}
        </div>
      </div>

      {/* Terminals */}
      <div>
        <h3 className="font-semibold text-gray-900 dark:text-white mb-3">Terminals</h3>
        <div className="flex flex-wrap gap-2">
          {[...grammar.terminals].sort().map((t) => (
            <span
              key={t}
              className="px-3 py-1 bg-green-100 dark:bg-green-900/30 text-green-800 dark:text-green-300 rounded-full text-sm font-medium"
            >
              {t}
            </span>
          ))}
        </div>
      </div>

      {/* Productions */}
      <div>
        <h3 className="font-semibold text-gray-900 dark:text-white mb-3">Production Rules</h3>
        <div className="bg-gray-50 dark:bg-gray-800 rounded-lg p-4 max-h-64 overflow-y-auto scrollbar-thin">
          <table className="w-full">
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

export default GrammarTab;
