import React from 'react';

const ParsingTableTab = ({ parserTable }) => {
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
      {/* Table Status */}
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

      {/* Parsing Table */}
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

      {/* Table Statistics */}
      <div className="grid grid-cols-2 md:grid-cols-4 gap-4">
        <div className="bg-gray-50 dark:bg-gray-700/50 p-4 rounded-lg text-center">
          <p className="text-2xl font-bold text-primary-600 dark:text-primary-400">
            {nonTerminals.length}
          </p>
          <p className="text-sm text-gray-600 dark:text-gray-400">Non-terminals</p>
        </div>
        <div className="bg-gray-50 dark:bg-gray-700/50 p-4 rounded-lg text-center">
          <p className="text-2xl font-bold text-secondary-600 dark:text-secondary-400">
            {terminals.length}
          </p>
          <p className="text-sm text-gray-600 dark:text-gray-400">Terminals</p>
        </div>
        <div className="bg-gray-50 dark:bg-gray-700/50 p-4 rounded-lg text-center">
          <p className="text-2xl font-bold text-green-600 dark:text-green-400">
            {parserTable.table.size}
          </p>
          <p className="text-sm text-gray-600 dark:text-gray-400">Table Size</p>
        </div>
        <div className="bg-gray-50 dark:bg-gray-700/50 p-4 rounded-lg text-center">
          <p className="text-2xl font-bold text-orange-600 dark:text-orange-400">
            {parserTable.conflicts.length}
          </p>
          <p className="text-sm text-gray-600 dark:text-gray-400">Conflicts</p>
        </div>
      </div>
    </div>
  );
};

export default ParsingTableTab;
