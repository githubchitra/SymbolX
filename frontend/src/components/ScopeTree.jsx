import React, { useState, useEffect } from 'react';
import { ChevronRight, ChevronDown, Folder, FolderOpen, File, Minus, AlertCircle, AlertTriangle } from 'lucide-react';
import { scopeAPI } from '../services/api';

const ScopeTree = ({ onScopeSelect, currentScope, problems = [] }) => {
  const [scopeHierarchy, setScopeHierarchy] = useState(null);
  const [expandedNodes, setExpandedNodes] = useState(new Set(['global']));
  const [loading, setLoading] = useState(true);

  const getScopeProblems = (scopeName) => {
    return problems.filter(p => p.scope === scopeName);
  };

  useEffect(() => {
    loadScopeHierarchy();
  }, []);

  const loadScopeHierarchy = async () => {
    try {
      setLoading(true);
      const data = await scopeAPI.getScopeHierarchy();
      setScopeHierarchy(data);
    } catch (error) {
      console.error('Error loading scope hierarchy:', error);
      // Set default structure if API fails
      setScopeHierarchy({
        scopes: [
          {
            name: 'global',
            level: 0,
            children: [],
            symbol_count: 0
          }
        ],
        current_scope: 'global'
      });
    } finally {
      setLoading(false);
    }
  };

  const toggleNode = (nodeName) => {
    const newExpanded = new Set(expandedNodes);
    if (newExpanded.has(nodeName)) {
      newExpanded.delete(nodeName);
    } else {
      newExpanded.add(nodeName);
    }
    setExpandedNodes(newExpanded);
  };

  const handleScopeEnter = async (scopeName) => {
    try {
      await scopeAPI.enterScope(scopeName);
      await loadScopeHierarchy(); // Refresh hierarchy
      onScopeSelect && onScopeSelect(scopeName);
    } catch (error) {
      console.error('Error entering scope:', error);
    }
  };

  const handleScopeExit = async () => {
    try {
      await scopeAPI.exitScope();
      await loadScopeHierarchy(); // Refresh hierarchy
      onScopeSelect && onScopeSelect(scopeHierarchy?.current_scope || 'global');
    } catch (error) {
      console.error('Error exiting scope:', error);
    }
  };

  const renderScopeNode = (scope, level = 0) => {
    const isExpanded = expandedNodes.has(scope.name);
    const hasChildren = scope.children && scope.children.length > 0;
    const isCurrentScope = scope.name === currentScope;

    const scopeProblems = getScopeProblems(scope.name);
    const hasError = scopeProblems.some(p => p.severity === 'error');
    const hasWarning = scopeProblems.some(p => p.severity === 'warning');

    return (
      <div key={scope.name} className="select-none">
        <div
          className={`flex items-center py-2 px-3 hover:bg-gray-100 rounded-lg cursor-pointer transition-colors ${isCurrentScope ? 'bg-primary-100 border-l-4 border-primary-600' : ''
            }`}
          style={{ paddingLeft: `${level * 20 + 12}px` }}
          onClick={() => {
            if (hasChildren) {
              toggleNode(scope.name);
            }
            handleScopeEnter(scope.name);
          }}
        >
          {hasChildren ? (
            isExpanded ? (
              <ChevronDown className="h-4 w-4 mr-2 text-gray-500" />
            ) : (
              <ChevronRight className="h-4 w-4 mr-2 text-gray-500" />
            )
          ) : (
            <div className="w-4 mr-2" />
          )}

          {hasChildren ? (
            isExpanded ? (
              <FolderOpen className={`h-4 w-4 mr-2 ${hasError ? 'text-red-500' : 'text-amber-600'}`} />
            ) : (
              <Folder className={`h-4 w-4 mr-2 ${hasError ? 'text-red-500' : 'text-amber-600'}`} />
            )
          ) : (
            <File className={`h-4 w-4 mr-2 ${hasError ? 'text-red-400' : 'text-gray-400'}`} />
          )}

          <span className={`flex-1 text-sm font-medium ${hasError ? 'text-red-700' : isCurrentScope ? 'text-primary-900' : 'text-gray-700'
            }`}>
            {scope.name}
          </span>

          <div className="flex items-center gap-2">
            {scopeProblems.length > 0 && (
              <div className="flex items-center gap-1">
                {hasError ? <AlertCircle className="h-3 w-3 text-red-500" /> : <AlertTriangle className="h-3 w-3 text-amber-500" />}
                <span className={`text-[10px] font-bold ${hasError ? 'text-red-600' : 'text-amber-600'}`}>
                  {scopeProblems.length}
                </span>
              </div>
            )}
            {scope.symbol_count !== undefined && (
              <span className="text-xs text-gray-500 bg-gray-200 px-2 py-0.5 rounded-full">
                {scope.symbol_count}
              </span>
            )}
            {scope.level !== undefined && (
              <span className="text-xs text-gray-400">
                L{scope.level}
              </span>
            )}
          </div>
        </div>

        {hasChildren && isExpanded && (
          <div className="ml-2">
            {scope.children.map(child => renderScopeNode(child, level + 1))}
          </div>
        )}
      </div>
    );
  };

  const renderBreadcrumbs = () => {
    if (!currentScope || currentScope === 'global') {
      return (
        <div className="flex items-center text-sm text-gray-600">
          <span className="font-medium">Global Scope</span>
        </div>
      );
    }

    return (
      <div className="flex items-center text-sm text-gray-600">
        <span
          className="hover:text-primary-600 cursor-pointer"
          onClick={() => handleScopeEnter('global')}
        >
          Global
        </span>
        <ChevronRight className="h-4 w-4 mx-1" />
        <span className="font-medium text-primary-600">{currentScope}</span>
      </div>
    );
  };

  if (loading) {
    return (
      <div className="bg-white rounded-lg shadow-lg p-6">
        <div className="flex items-center justify-center h-32">
          <div className="animate-spin rounded-full h-6 w-6 border-b-2 border-primary-600"></div>
        </div>
      </div>
    );
  }

  return (
    <div className="bg-white rounded-lg shadow-lg overflow-hidden">
      {/* Header */}
      <div className="bg-gradient-to-r from-amber-600 to-amber-700 text-white p-6">
        <h2 className="text-xl font-bold mb-2">Scope Hierarchy</h2>
        <div className="text-amber-100 text-sm">
          {renderBreadcrumbs()}
        </div>
      </div>

      {/* Controls */}
      <div className="border-b border-gray-200 p-4">
        <div className="flex items-center justify-between">
          <div className="flex items-center gap-2">
            <button
              onClick={() => handleScopeEnter('global')}
              className="px-3 py-1 text-sm bg-gray-100 text-gray-700 rounded hover:bg-gray-200 transition-colors flex items-center gap-1"
            >
              <Folder className="h-3 w-3" />
              Global
            </button>
            {currentScope && currentScope !== 'global' && (
              <button
                onClick={handleScopeExit}
                className="px-3 py-1 text-sm bg-red-100 text-red-700 rounded hover:bg-red-200 transition-colors flex items-center gap-1"
              >
                <Minus className="h-3 w-3" />
                Exit Scope
              </button>
            )}
          </div>

          <button
            onClick={loadScopeHierarchy}
            className="px-3 py-1 text-sm bg-primary-100 text-primary-700 rounded hover:bg-primary-200 transition-colors"
          >
            Refresh
          </button>
        </div>
      </div>

      {/* Tree */}
      <div className="p-4 max-h-96 overflow-y-auto">
        {scopeHierarchy?.scopes ? (
          scopeHierarchy.scopes.map(scope => renderScopeNode(scope))
        ) : (
          <div className="text-center text-gray-500 py-8">
            <Folder className="h-12 w-12 mx-auto mb-4 text-gray-300" />
            <p>No scope hierarchy available</p>
            <p className="text-sm mt-2">Start parsing code to see scopes</p>
          </div>
        )}
      </div>

      {/* Legend */}
      <div className="border-t border-gray-200 p-4 bg-gray-50">
        <div className="text-xs text-gray-600">
          <div className="flex items-center gap-4">
            <div className="flex items-center gap-1">
              <Folder className="h-3 w-3 text-amber-600" />
              <span>Scope with children</span>
            </div>
            <div className="flex items-center gap-1">
              <File className="h-3 w-3 text-gray-400" />
              <span>Leaf scope</span>
            </div>
            <div className="flex items-center gap-1">
              <div className="w-3 h-3 bg-primary-100 border-l-2 border-primary-600"></div>
              <span>Current scope</span>
            </div>
          </div>
        </div>
      </div>
    </div>
  );
};

export default ScopeTree;
