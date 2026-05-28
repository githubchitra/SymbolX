import React from 'react';

const FirstFollowTab = ({ firstFollow }) => {
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
      {/* LL(1) Status */}
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

      {/* FIRST Sets */}
      <div>
        <h3 className="font-semibold text-gray-900 dark:text-white mb-3">FIRST Sets</h3>
        <div className="bg-gray-50 dark:bg-gray-800 rounded-lg p-4 max-h-64 overflow-y-auto scrollbar-thin">
          <table className="w-full">
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

      {/* FOLLOW Sets */}
      <div>
        <h3 className="font-semibold text-gray-900 dark:text-white mb-3">FOLLOW Sets</h3>
        <div className="bg-gray-50 dark:bg-gray-800 rounded-lg p-4 max-h-64 overflow-y-auto scrollbar-thin">
          <table className="w-full">
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

export default FirstFollowTab;
