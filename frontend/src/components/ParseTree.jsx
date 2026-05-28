import React, { useState, useEffect, useRef } from 'react';
import { ChevronRight, ChevronDown, File, Folder, Code, Braces, Variable, Cpu, AlertCircle, CheckCircle } from 'lucide-react';

const ParseTree = ({ parseData, errors = [], onNodeClick, selectedNode }) => {
  const [expandedNodes, setExpandedNodes] = useState(new Set(['root']));
  const [hoveredNode, setHoveredNode] = useState(null);

  const toggleNode = (nodeId) => {
    const newExpanded = new Set(expandedNodes);
    if (newExpanded.has(nodeId)) {
      newExpanded.delete(nodeId);
    } else {
      newExpanded.add(nodeId);
    }
    setExpandedNodes(newExpanded);
  };

  const getNodeIcon = (nodeType, hasError = false) => {
    if (hasError) {
      return <AlertCircle className="h-4 w-4 text-red-500" />;
    }
    
    switch (nodeType) {
      case 'program':
        return <File className="h-4 w-4 text-blue-500" />;
      case 'function':
        return <Cpu className="h-4 w-4 text-green-500" />;
      case 'declaration':
        return <Variable className="h-4 w-4 text-purple-500" />;
      case 'statement':
        return <Code className="h-4 w-4 text-orange-500" />;
      case 'expression':
        return <Braces className="h-4 w-4 text-indigo-500" />;
      case 'block':
        return <Folder className="h-4 w-4 text-gray-500" />;
      default:
        return <Code className="h-4 w-4 text-gray-400" />;
    }
  };

  const getNodeTypeColor = (nodeType, hasError = false) => {
    if (hasError) return 'text-red-600 border-red-200 bg-red-50';
    
    switch (nodeType) {
      case 'program':
        return 'text-blue-600 border-blue-200 bg-blue-50';
      case 'function':
        return 'text-green-600 border-green-200 bg-green-50';
      case 'declaration':
        return 'text-purple-600 border-purple-200 bg-purple-50';
      case 'statement':
        return 'text-orange-600 border-orange-200 bg-orange-50';
      case 'expression':
        return 'text-indigo-600 border-indigo-200 bg-indigo-50';
      case 'block':
        return 'text-gray-600 border-gray-200 bg-gray-50';
      default:
        return 'text-gray-600 border-gray-200 bg-gray-50';
    }
  };

  const TreeNode = ({ node, level = 0, parentPath = '' }) => {
    const nodeId = `${parentPath}/${node.type}_${node.id || level}`;
    const isExpanded = expandedNodes.has(nodeId);
    const hasChildren = node.children && node.children.length > 0;
    const hasError = errors.some(error => error.nodeId === nodeId || error.line === node.line);
    const isSelected = selectedNode === nodeId;
    const isHovered = hoveredNode === nodeId;

    if (!node) return null;

    return (
      <div className="select-none">
        <div
          className={`flex items-center gap-2 py-1 px-2 rounded cursor-pointer transition-all duration-200 ${
            isSelected ? 'bg-blue-100 border-l-4 border-blue-500' : ''
          } ${isHovered ? 'bg-gray-100' : ''} ${hasError ? 'bg-red-50' : ''}`}
          style={{ paddingLeft: `${level * 20 + 8}px` }}
          onClick={() => {
            if (hasChildren) {
              toggleNode(nodeId);
            }
            onNodeClick && onNodeClick(node, nodeId);
          }}
          onMouseEnter={() => setHoveredNode(nodeId)}
          onMouseLeave={() => setHoveredNode(null)}
        >
          {hasChildren && (
            <div className="transition-transform duration-200">
              {isExpanded ? (
                <ChevronDown className="h-3 w-3 text-gray-500" />
              ) : (
                <ChevronRight className="h-3 w-3 text-gray-500" />
              )}
            </div>
          )}
          {!hasChildren && <div className="w-3" />}
          
          {getNodeIcon(node.type, hasError)}
          
          <div className="flex-1 min-w-0">
            <div className="flex items-center gap-2">
              <span className={`text-sm font-medium ${getNodeTypeColor(node.type, hasError).split(' ')[0]}`}>
                {node.type}
              </span>
              {node.value && (
                <span className="text-xs text-gray-600 font-mono bg-gray-100 px-1 rounded">
                  {node.value}
                </span>
              )}
              {node.dataType && (
                <span className="text-xs text-blue-600 bg-blue-100 px-1 rounded">
                  {node.dataType}
                </span>
              )}
            </div>
            
            {(node.line !== undefined || node.scope) && (
              <div className="flex items-center gap-2 text-xs text-gray-500">
                {node.line !== undefined && (
                  <span>Line {node.line}</span>
                )}
                {node.scope && (
                  <span className="text-purple-600">
                    {node.scope}
                  </span>
                )}
              </div>
            )}
          </div>

          {hasError && (
            <div className="flex items-center gap-1">
              <AlertCircle className="h-3 w-3 text-red-500" />
              <span className="text-xs text-red-600">
                {errors.filter(e => e.nodeId === nodeId || e.line === node.line).length}
              </span>
            </div>
          )}
        </div>

        {hasChildren && isExpanded && (
          <div className="ml-2">
            {node.children.map((child, index) => (
              <TreeNode
                key={`${nodeId}_${child.type}_${index}`}
                node={child}
                level={level + 1}
                parentPath={nodeId}
              />
            ))}
          </div>
        )}
      </div>
    );
  };

  const generateMockParseTree = () => {
    return {
      type: 'program',
      id: 'root',
      children: [
        {
          type: 'declaration',
          id: 'decl1',
          value: 'x',
          dataType: 'int',
          line: 2,
          scope: 'global',
          children: [
            {
              type: 'expression',
              id: 'expr1',
              value: '10',
              dataType: 'int',
              line: 2,
              children: []
            }
          ]
        },
        {
          type: 'function',
          id: 'func1',
          value: 'main',
          dataType: 'int',
          line: 4,
          scope: 'global',
          children: [
            {
              type: 'block',
              id: 'block1',
              line: 4,
              scope: 'main',
              children: [
                {
                  type: 'declaration',
                  id: 'decl2',
                  value: 'local_var',
                  dataType: 'int',
                  line: 5,
                  scope: 'main',
                  children: [
                    {
                      type: 'expression',
                      id: 'expr2',
                      value: '5',
                      dataType: 'int',
                      line: 5,
                      children: []
                    }
                  ]
                },
                {
                  type: 'statement',
                  id: 'stmt1',
                  value: 'return',
                  dataType: 'int',
                  line: 6,
                  scope: 'main',
                  children: [
                    {
                      type: 'expression',
                      id: 'expr3',
                      value: '0',
                      dataType: 'int',
                      line: 6,
                      children: []
                    }
                  ]
                }
              ]
            }
          ]
        }
      ]
    };
  };

  const parseTreeData = parseData || generateMockParseTree();

  return (
    <div className="bg-white rounded-lg shadow-lg overflow-hidden">
      {/* Header */}
      <div className="bg-gradient-to-r from-green-600 to-green-700 text-white p-6">
        <h2 className="text-xl font-bold mb-2">Parse Tree</h2>
        <p className="text-green-100 text-sm">
          Hierarchical representation of the parsed code structure
        </p>
      </div>

      {/* Controls */}
      <div className="border-b border-gray-200 p-4">
        <div className="flex items-center justify-between">
          <div className="flex items-center gap-2">
            <button
              onClick={() => setExpandedNodes(new Set(['root']))}
              className="px-3 py-1 text-sm bg-gray-600 text-white rounded hover:bg-gray-700 transition-colors"
            >
              Collapse All
            </button>
            <button
              onClick={() => {
                const allNodes = new Set();
                const collectNodes = (node, path = 'root') => {
                  allNodes.add(path);
                  if (node.children) {
                    node.children.forEach((child, index) => {
                      collectNodes(child, `${path}/${child.type}_${child.id || index}`);
                    });
                  }
                };
                collectNodes(parseTreeData);
                setExpandedNodes(allNodes);
              }}
              className="px-3 py-1 text-sm bg-green-600 text-white rounded hover:bg-green-700 transition-colors"
            >
              Expand All
            </button>
          </div>
          
          <div className="flex items-center gap-4 text-sm text-gray-600">
            <div className="flex items-center gap-2">
              <div className="w-3 h-3 bg-blue-500 rounded"></div>
              <span>Program</span>
            </div>
            <div className="flex items-center gap-2">
              <div className="w-3 h-3 bg-green-500 rounded"></div>
              <span>Function</span>
            </div>
            <div className="flex items-center gap-2">
              <div className="w-3 h-3 bg-purple-500 rounded"></div>
              <span>Declaration</span>
            </div>
            <div className="flex items-center gap-2">
              <div className="w-3 h-3 bg-red-500 rounded"></div>
              <span>Error</span>
            </div>
          </div>
        </div>
      </div>

      {/* Parse Tree Content */}
      <div className="p-4 max-h-96 overflow-y-auto">
        {parseTreeData ? (
          <div className="font-mono text-sm">
            <TreeNode node={parseTreeData} />
          </div>
        ) : (
          <div className="text-center py-8 text-gray-500">
            <Code className="h-12 w-12 mx-auto mb-4 text-gray-300" />
            <p>No parse tree available</p>
            <p className="text-sm">Parse some code to see the tree structure</p>
          </div>
        )}
      </div>

      {/* Error Summary */}
      {errors.length > 0 && (
        <div className="border-t border-gray-200 p-4 bg-red-50">
          <div className="flex items-center gap-2 mb-2">
            <AlertCircle className="h-4 w-4 text-red-600" />
            <span className="font-medium text-red-800">Parse Errors ({errors.length})</span>
          </div>
          <div className="space-y-1">
            {errors.slice(0, 3).map((error, index) => (
              <div key={index} className="text-sm text-red-700">
                Line {error.line}: {error.message}
              </div>
            ))}
            {errors.length > 3 && (
              <div className="text-sm text-red-600">
                ... and {errors.length - 3} more errors
              </div>
            )}
          </div>
        </div>
      )}
    </div>
  );
};

export default ParseTree;
