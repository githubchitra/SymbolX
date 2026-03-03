import React, { useState, useEffect, useMemo } from 'react';
import { Search, Download, Edit2, Trash2, Eye, AlertCircle, AlertTriangle } from 'lucide-react';
import { symbolTableAPI, exportAPI } from '../services/api';

const SymbolTableGrid = ({ symbols: propSymbols, onSymbolSelect, onSymbolEdit, onDelete, problems = [] }) => {
  const [symbols, setSymbols] = useState(propSymbols || []);
  const [searchTerm, setSearchTerm] = useState('');
  const [filterType, setFilterType] = useState('all');
  const [filterKind, setFilterKind] = useState('all');
  const [sortConfig, setSortConfig] = useState({ key: 'name', direction: 'ascending' });

  // Update symbols when prop changes
  useEffect(() => {
    setSymbols(propSymbols || []);
  }, [propSymbols]);

  const getSymbolProblems = (symbolName, scopeName) => {
    return problems.filter(p => p.symbol === symbolName && p.scope === scopeName);
  };

  // Filter and sort symbols
  const filteredAndSortedSymbols = useMemo(() => {
    let filtered = symbols.filter(symbol => {
      const matchesSearch = symbol.name.toLowerCase().includes(searchTerm.toLowerCase()) ||
        symbol.scope_name.toLowerCase().includes(searchTerm.toLowerCase());
      const matchesType = filterType === 'all' || symbol.data_type.toLowerCase() === filterType.toLowerCase();
      const matchesKind = filterKind === 'all' || symbol.kind.toLowerCase() === filterKind.toLowerCase();

      return matchesSearch && matchesType && matchesKind;
    });

    // Sort symbols
    filtered.sort((a, b) => {
      const aValue = a[sortConfig.key];
      const bValue = b[sortConfig.key];

      if (aValue < bValue) {
        return sortConfig.direction === 'ascending' ? -1 : 1;
      }
      if (aValue > bValue) {
        return sortConfig.direction === 'ascending' ? 1 : -1;
      }
      return 0;
    });

    return filtered;
  }, [symbols, searchTerm, filterType, filterKind, sortConfig]);

  const handleSort = (key) => {
    let direction = 'ascending';
    if (sortConfig.key === key && sortConfig.direction === 'ascending') {
      direction = 'descending';
    }
    setSortConfig({ key, direction });
  };

  const handleExport = async (format) => {
    try {
      const data = await exportAPI.exportSymbolTable(format);

      if (format === 'json') {
        const blob = new Blob([JSON.stringify(data, null, 2)], { type: 'application/json' });
        const url = URL.createObjectURL(blob);
        const a = document.createElement('a');
        a.href = url;
        a.download = 'symbol-table.json';
        a.click();
        URL.revokeObjectURL(url);
      } else if (format === 'csv') {
        const blob = new Blob([data.csv], { type: 'text/csv' });
        const url = URL.createObjectURL(blob);
        const a = document.createElement('a');
        a.href = url;
        a.download = 'symbol-table.csv';
        a.click();
        URL.revokeObjectURL(url);
      }
    } catch (error) {
      console.error('Error exporting symbols:', error);
    }
  };

  const getSymbolColor = (kind) => {
    const colors = {
      variable: 'text-green-600 bg-green-50',
      function: 'text-amber-600 bg-amber-50',
      array: 'text-purple-600 bg-purple-50',
      parameter: 'text-cyan-600 bg-cyan-50',
      constant: 'text-red-600 bg-red-50',
      struct: 'text-indigo-600 bg-indigo-50',
      typedef: 'text-lime-600 bg-lime-50',
    };
    return colors[kind.toLowerCase()] || 'text-gray-600 bg-gray-50';
  };

  const getDataTypeColor = (type) => {
    const colors = {
      int: 'text-blue-600',
      float: 'text-green-600',
      char: 'text-orange-600',
      double: 'text-purple-600',
      void: 'text-gray-600',
      bool: 'text-red-600',
      string: 'text-pink-600',
      array: 'text-indigo-600',
      pointer: 'text-cyan-600',
      struct: 'text-yellow-600',
      function: 'text-amber-600',
    };
    return colors[type.toLowerCase()] || 'text-gray-600';
  };

  const getStatusColor = (status) => {
    const colors = {
      initialized: 'text-green-800 bg-green-100',
      declared: 'text-gray-800 bg-gray-100',
      defined: 'text-blue-800 bg-blue-100',
      parameter: 'text-amber-800 bg-amber-100',
    };
    return colors[status?.toLowerCase()] || 'text-gray-800 bg-gray-100';
  };

  return (
    <div className="bg-white rounded-lg shadow-lg overflow-hidden">
      {/* Header */}
      <div className="bg-gradient-to-r from-primary-600 to-primary-700 text-white p-6">
        <h2 className="text-2xl font-bold mb-4">Symbol Table</h2>

        {/* Search and Filters */}
        <div className="flex flex-wrap gap-4">
          <div className="flex-1 min-w-64">
            <div className="relative">
              <Search className="absolute left-3 top-1/2 transform -translate-y-1/2 text-gray-400 h-4 w-4" />
              <input
                type="text"
                placeholder="Search symbols..."
                value={searchTerm}
                onChange={(e) => setSearchTerm(e.target.value)}
                className="w-full pl-10 pr-4 py-2 border border-gray-300 rounded-lg focus:ring-2 focus:ring-primary-500 focus:border-transparent text-gray-900"
              />
            </div>
          </div>

          <select
            value={filterType}
            onChange={(e) => setFilterType(e.target.value)}
            className="px-4 py-2 border border-gray-300 rounded-lg focus:ring-2 focus:ring-primary-500 focus:border-transparent text-gray-900"
          >
            <option value="all">All Types</option>
            <option value="int">int</option>
            <option value="float">float</option>
            <option value="char">char</option>
            <option value="double">double</option>
            <option value="void">void</option>
            <option value="bool">bool</option>
            <option value="array">array</option>
            <option value="pointer">pointer</option>
            <option value="struct">struct</option>
            <option value="function">function</option>
          </select>

          <select
            value={filterKind}
            onChange={(e) => setFilterKind(e.target.value)}
            className="px-4 py-2 border border-gray-300 rounded-lg focus:ring-2 focus:ring-primary-500 focus:border-transparent text-gray-900"
          >
            <option value="all">All Kinds</option>
            <option value="variable">Variable</option>
            <option value="function">Function</option>
            <option value="parameter">Parameter</option>
            <option value="constant">Constant</option>
            <option value="array">Array</option>
            <option value="struct">Struct</option>
            <option value="typedef">Typedef</option>
          </select>

          <div className="flex gap-2">
            <button
              onClick={() => handleExport('json')}
              className="px-4 py-2 bg-green-600 text-white rounded-lg hover:bg-green-700 transition-colors flex items-center gap-2"
            >
              <Download className="h-4 w-4" />
              JSON
            </button>
            <button
              onClick={() => handleExport('csv')}
              className="px-4 py-2 bg-blue-600 text-white rounded-lg hover:bg-blue-700 transition-colors flex items-center gap-2"
            >
              <Download className="h-4 w-4" />
              CSV
            </button>
          </div>
        </div>
      </div>

      {/* Table */}
      <div className="overflow-x-auto">
        <table className="w-full">
          <thead className="bg-gray-50 border-b border-gray-200">
            <tr>
              <th
                onClick={() => handleSort('name')}
                className="px-6 py-3 text-left text-xs font-medium text-gray-500 uppercase tracking-wider cursor-pointer hover:bg-gray-100"
              >
                Name
                {sortConfig.key === 'name' && (
                  <span className="ml-1">
                    {sortConfig.direction === 'ascending' ? '↑' : '↓'}
                  </span>
                )}
              </th>
              <th
                onClick={() => handleSort('data_type')}
                className="px-6 py-3 text-left text-xs font-medium text-gray-500 uppercase tracking-wider cursor-pointer hover:bg-gray-100"
              >
                Type
                {sortConfig.key === 'data_type' && (
                  <span className="ml-1">
                    {sortConfig.direction === 'ascending' ? '↑' : '↓'}
                  </span>
                )}
              </th>
              <th
                onClick={() => handleSort('kind')}
                className="px-6 py-3 text-left text-xs font-medium text-gray-500 uppercase tracking-wider cursor-pointer hover:bg-gray-100"
              >
                Kind
                {sortConfig.key === 'kind' && (
                  <span className="ml-1">
                    {sortConfig.direction === 'ascending' ? '↑' : '↓'}
                  </span>
                )}
              </th>
              <th
                onClick={() => handleSort('scope_name')}
                className="px-6 py-3 text-left text-xs font-medium text-gray-500 uppercase tracking-wider cursor-pointer hover:bg-gray-100"
              >
                Scope
                {sortConfig.key === 'scope_name' && (
                  <span className="ml-1">
                    {sortConfig.direction === 'ascending' ? '↑' : '↓'}
                  </span>
                )}
              </th>
              <th
                onClick={() => handleSort('line_number')}
                className="px-6 py-3 text-left text-xs font-medium text-gray-500 uppercase tracking-wider cursor-pointer hover:bg-gray-100"
              >
                Line
                {sortConfig.key === 'line_number' && (
                  <span className="ml-1">
                    {sortConfig.direction === 'ascending' ? '↑' : '↓'}
                  </span>
                )}
              </th>
              <th className="px-6 py-3 text-left text-xs font-medium text-gray-500 uppercase tracking-wider">
                Status
              </th>
              <th className="px-6 py-3 text-left text-xs font-medium text-gray-500 uppercase tracking-wider">
                Actions
              </th>
            </tr>
          </thead>
          <tbody className="bg-white divide-y divide-gray-200">
            {filteredAndSortedSymbols.map((symbol, index) => {
              const symbolProblems = getSymbolProblems(symbol.name, symbol.scope_name);
              const hasError = symbolProblems.some(p => p.severity === 'error');
              const hasWarning = symbolProblems.some(p => p.severity === 'warning');

              return (
                <tr
                  key={`${symbol.name}-${symbol.scope_name}-${index}`}
                  className={`transition-colors cursor-pointer ${hasError ? 'bg-red-50 hover:bg-red-100' :
                    hasWarning ? 'bg-amber-50 hover:bg-amber-100' :
                      'hover:bg-gray-50'
                    }`}
                  onClick={() => {
                    onSymbolSelect && onSymbolSelect(symbol);
                  }}
                >
                  <td className="px-6 py-4 whitespace-nowrap">
                    <div className="flex items-center gap-2">
                      <div className="text-sm font-medium text-gray-900">{symbol.name}</div>
                      {hasError && <AlertCircle className="h-3 w-3 text-red-500" />}
                      {hasWarning && !hasError && <AlertTriangle className="h-3 w-3 text-amber-500" />}
                    </div>
                  </td>
                  <td className="px-6 py-4 whitespace-nowrap">
                    <span className={`text-sm font-medium ${getDataTypeColor(symbol.data_type)}`}>
                      {symbol.data_type}
                    </span>
                  </td>
                  <td className="px-6 py-4 whitespace-nowrap">
                    <span className={`inline-flex px-2 py-1 text-xs font-semibold rounded-full ${getSymbolColor(symbol.kind)}`}>
                      {symbol.kind}
                    </span>
                  </td>
                  <td className="px-6 py-4 whitespace-nowrap">
                    <div className="text-sm text-gray-900">{symbol.scope_name}</div>
                    <div className="text-xs text-gray-500">Level {symbol.scope_level}</div>
                  </td>
                  <td className="px-6 py-4 whitespace-nowrap text-sm text-gray-900">
                    {symbol.line_number}
                  </td>
                  <td className="px-6 py-4 whitespace-nowrap">
                    <div className="flex flex-wrap items-center gap-2">
                      <span className={`inline-flex items-center px-2.5 py-0.5 rounded-full text-xs font-medium capitalize ${getStatusColor(symbol.status)}`}>
                        {symbol.status || (symbol.is_initialized ? 'initialized' : 'declared')}
                      </span>
                      {symbolProblems.length > 0 && (
                        <span className={`inline-flex items-center px-2 py-0.5 rounded-full text-[10px] font-bold ${hasError ? 'bg-red-100 text-red-700' : 'bg-amber-100 text-amber-700'}`}>
                          {symbolProblems.length} issue{symbolProblems.length !== 1 ? 's' : ''}
                        </span>
                      )}
                      {symbol.storage_class && symbol.storage_class !== 'auto' && (
                        <span className="inline-flex items-center px-2 py-1 text-xs font-medium text-blue-800 bg-blue-100 rounded-full">
                          {symbol.storage_class}
                        </span>
                      )}
                    </div>
                  </td>
                  <td className="px-6 py-4 whitespace-nowrap text-sm font-medium">
                    <div className="flex items-center gap-2">
                      <button
                        onClick={(e) => {
                          e.stopPropagation();
                          onSymbolSelect && onSymbolSelect(symbol);
                        }}
                        className="text-primary-600 hover:text-primary-900"
                        title="View Details"
                      >
                        <Eye className="h-4 w-4" />
                      </button>
                      <button
                        onClick={(e) => {
                          e.stopPropagation();
                          onSymbolEdit && onSymbolEdit(symbol);
                        }}
                        className="text-amber-600 hover:text-amber-900"
                        title="Edit Symbol"
                      >
                        <Edit2 className="h-4 w-4" />
                      </button>
                      <button
                        onClick={(e) => {
                          e.stopPropagation();
                          onDelete && onDelete(symbol.name);
                        }}
                        className="text-red-600 hover:text-red-900"
                        title="Delete Symbol"
                      >
                        <Trash2 className="h-4 w-4" />
                      </button>
                    </div>
                  </td>
                </tr>
              );
            })}
          </tbody>
        </table>

        {filteredAndSortedSymbols.length === 0 && (
          <div className="text-center py-8 text-gray-500">
            No symbols found matching your criteria.
          </div>
        )}
      </div>
    </div>
  );
};

export default SymbolTableGrid;
