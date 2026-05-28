#ifndef PARSE_TREE_H
#define PARSE_TREE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Constants
#define MAX_NODE_ERRORS 10
#define INITIAL_CHILD_CAPACITY 4

// Node types for parse tree
typedef enum {
    NODE_PROGRAM,
    NODE_FUNCTION,
    NODE_DECLARATION,
    NODE_STATEMENT,
    NODE_EXPRESSION,
    NODE_BLOCK,
    NODE_PARAMETER,
    NODE_IDENTIFIER,
    NODE_LITERAL,
    NODE_OPERATOR,
    NODE_UNKNOWN
} NodeType;

// Parse tree node structure
typedef struct ParseTreeNode {
    NodeType type;
    char* value;
    char* data_type;
    char* scope;
    int line;
    int column;
    
    // Tree structure
    struct ParseTreeNode* parent;
    struct ParseTreeNode** children;
    int child_count;
    int child_capacity;
    
    // Error tracking
    char* errors[MAX_NODE_ERRORS];
    int error_count;
} ParseTreeNode;

// Tree statistics structure
typedef struct {
    int total_nodes;
    int max_depth;
    int program_nodes;
    int function_nodes;
    int declaration_nodes;
    int statement_nodes;
    int expression_nodes;
    int block_nodes;
    int parameter_nodes;
    int identifier_nodes;
    int literal_nodes;
    int operator_nodes;
    int error_nodes;
} TreeStats;

// Function declarations

// Node creation and management
ParseTreeNode* create_parse_node(NodeType type, const char* value, int line);
int add_child_node(ParseTreeNode* parent, ParseTreeNode* child);
void set_node_data_type(ParseTreeNode* node, const char* data_type);
void set_node_scope(ParseTreeNode* node, const char* scope);
int add_node_error(ParseTreeNode* node, const char* error_message);

// Tree traversal and search
ParseTreeNode* find_node_by_line(ParseTreeNode* root, int line);
int find_nodes_by_type(ParseTreeNode* root, NodeType type, ParseTreeNode** results, int max_results);

// Utility functions
const char* node_type_to_string(NodeType type);
void print_parse_tree(ParseTreeNode* node, int depth);
TreeStats calculate_tree_stats(ParseTreeNode* root);
int validate_parse_tree(ParseTreeNode* root);

// Memory management
void free_parse_tree(ParseTreeNode* node);

// Test function
ParseTreeNode* create_test_program_tree();

#endif // PARSE_TREE_H
