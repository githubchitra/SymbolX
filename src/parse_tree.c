#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/parse_tree.h"

// Create a new parse tree node
ParseTreeNode* create_parse_node(NodeType type, const char* value, int line) {
    ParseTreeNode* node = (ParseTreeNode*)malloc(sizeof(ParseTreeNode));
    if (!node) {
        fprintf(stderr, "Error: Memory allocation failed for parse tree node\n");
        return NULL;
    }
    
    node->type = type;
    node->line = line;
    node->value = value ? strdup(value) : NULL;
    node->data_type = NULL;
    node->scope = NULL;
    node->children = NULL;
    node->child_count = 0;
    node->child_capacity = 0;
    node->parent = NULL;
    node->error_count = 0;
    
    return node;
}

// Add a child node to a parent node
int add_child_node(ParseTreeNode* parent, ParseTreeNode* child) {
    if (!parent || !child) {
        return 0;
    }
    
    child->parent = parent;
    
    // Resize children array if needed
    if (parent->child_count >= parent->child_capacity) {
        int new_capacity = parent->child_capacity == 0 ? 4 : parent->child_capacity * 2;
        ParseTreeNode** new_children = (ParseTreeNode**)realloc(
            parent->children, new_capacity * sizeof(ParseTreeNode*)
        );
        
        if (!new_children) {
            fprintf(stderr, "Error: Memory allocation failed for children array\n");
            return 0;
        }
        
        parent->children = new_children;
        parent->child_capacity = new_capacity;
    }
    
    parent->children[parent->child_count] = child;
    parent->child_count++;
    
    return 1;
}

// Set node data type
void set_node_data_type(ParseTreeNode* node, const char* data_type) {
    if (!node) return;
    
    if (node->data_type) {
        free(node->data_type);
    }
    node->data_type = data_type ? strdup(data_type) : NULL;
}

// Set node scope
void set_node_scope(ParseTreeNode* node, const char* scope) {
    if (!node) return;
    
    if (node->scope) {
        free(node->scope);
    }
    node->scope = scope ? strdup(scope) : NULL;
}

// Add error to node
int add_node_error(ParseTreeNode* node, const char* error_message) {
    if (!node || !error_message) {
        return 0;
    }
    
    // Resize errors array if needed
    if (node->error_count >= MAX_NODE_ERRORS) {
        fprintf(stderr, "Warning: Maximum errors per node reached\n");
        return 0;
    }
    
    node->errors[node->error_count] = strdup(error_message);
    if (!node->errors[node->error_count]) {
        fprintf(stderr, "Error: Memory allocation failed for error message\n");
        return 0;
    }
    
    node->error_count++;
    return 1;
}

// Find node by line number
ParseTreeNode* find_node_by_line(ParseTreeNode* root, int line) {
    if (!root) {
        return NULL;
    }
    
    if (root->line == line) {
        return root;
    }
    
    for (int i = 0; i < root->child_count; i++) {
        ParseTreeNode* result = find_node_by_line(root->children[i], line);
        if (result) {
            return result;
        }
    }
    
    return NULL;
}

// Find nodes by type
int find_nodes_by_type(ParseTreeNode* root, NodeType type, ParseTreeNode** results, int max_results) {
    if (!root || !results || max_results <= 0) {
        return 0;
    }
    
    int count = 0;
    
    if (root->type == type) {
        results[count] = root;
        count++;
        if (count >= max_results) {
            return count;
        }
    }
    
    for (int i = 0; i < root->child_count && count < max_results; i++) {
        count += find_nodes_by_type(root->children[i], type, results + count, max_results - count);
    }
    
    return count;
}

// Get node type as string
const char* node_type_to_string(NodeType type) {
    switch (type) {
        case NODE_PROGRAM: return "program";
        case NODE_FUNCTION: return "function";
        case NODE_DECLARATION: return "declaration";
        case NODE_STATEMENT: return "statement";
        case NODE_EXPRESSION: return "expression";
        case NODE_BLOCK: return "block";
        case NODE_PARAMETER: return "parameter";
        case NODE_IDENTIFIER: return "identifier";
        case NODE_LITERAL: return "literal";
        case NODE_OPERATOR: return "operator";
        default: return "unknown";
    }
}

// Print parse tree (for debugging)
void print_parse_tree(ParseTreeNode* node, int depth) {
    if (!node) return;
    
    // Print indentation
    for (int i = 0; i < depth; i++) {
        printf("  ");
    }
    
    // Print node information
    printf("%s", node_type_to_string(node->type));
    if (node->value) {
        printf(" (%s)", node->value);
    }
    if (node->data_type) {
        printf(" [%s]", node->data_type);
    }
    if (node->line > 0) {
        printf(" line:%d", node->line);
    }
    if (node->scope) {
        printf(" scope:%s", node->scope);
    }
    
    // Print errors
    if (node->error_count > 0) {
        printf(" ERRORS:");
        for (int i = 0; i < node->error_count; i++) {
            printf(" %s", node->errors[i]);
        }
    }
    
    printf("\n");
    
    // Print children
    for (int i = 0; i < node->child_count; i++) {
        print_parse_tree(node->children[i], depth + 1);
    }
}

// Free parse tree node and all its children
void free_parse_tree(ParseTreeNode* node) {
    if (!node) return;
    
    // Free all children first
    for (int i = 0; i < node->child_count; i++) {
        free_parse_tree(node->children[i]);
    }
    
    // Free node resources
    if (node->value) free(node->value);
    if (node->data_type) free(node->data_type);
    if (node->scope) free(node->scope);
    if (node->children) free(node->children);
    
    // Free error messages
    for (int i = 0; i < node->error_count; i++) {
        if (node->errors[i]) {
            free(node->errors[i]);
        }
    }
    
    free(node);
}

// Calculate tree statistics
TreeStats calculate_tree_stats(ParseTreeNode* root) {
    TreeStats stats = {0};
    
    if (!root) {
        return stats;
    }
    
    stats.total_nodes = 1;
    stats.max_depth = 1;
    stats.error_nodes = root->error_count > 0 ? 1 : 0;
    
    // Count by type
    switch (root->type) {
        case NODE_PROGRAM: stats.program_nodes++; break;
        case NODE_FUNCTION: stats.function_nodes++; break;
        case NODE_DECLARATION: stats.declaration_nodes++; break;
        case NODE_STATEMENT: stats.statement_nodes++; break;
        case NODE_EXPRESSION: stats.expression_nodes++; break;
        case NODE_BLOCK: stats.block_nodes++; break;
        case NODE_PARAMETER: stats.parameter_nodes++; break;
        case NODE_IDENTIFIER: stats.identifier_nodes++; break;
        case NODE_LITERAL: stats.literal_nodes++; break;
        case NODE_OPERATOR: stats.operator_nodes++; break;
        default: break;
    }
    
    // Process children
    int max_child_depth = 0;
    for (int i = 0; i < root->child_count; i++) {
        TreeStats child_stats = calculate_tree_stats(root->children[i]);
        
        stats.total_nodes += child_stats.total_nodes;
        stats.program_nodes += child_stats.program_nodes;
        stats.function_nodes += child_stats.function_nodes;
        stats.declaration_nodes += child_stats.declaration_nodes;
        stats.statement_nodes += child_stats.statement_nodes;
        stats.expression_nodes += child_stats.expression_nodes;
        stats.block_nodes += child_stats.block_nodes;
        stats.parameter_nodes += child_stats.parameter_nodes;
        stats.identifier_nodes += child_stats.identifier_nodes;
        stats.literal_nodes += child_stats.literal_nodes;
        stats.operator_nodes += child_stats.operator_nodes;
        stats.error_nodes += child_stats.error_nodes;
        
        if (child_stats.max_depth > max_child_depth) {
            max_child_depth = child_stats.max_depth;
        }
    }
    
    stats.max_depth += max_child_depth;
    
    return stats;
}

// Validate parse tree structure
int validate_parse_tree(ParseTreeNode* root) {
    if (!root) {
        fprintf(stderr, "Error: Parse tree root is NULL\n");
        return 0;
    }
    
    // Check for circular references
    ParseTreeNode* slow = root;
    ParseTreeNode* fast = root;
    
    while (fast && fast->parent) {
        slow = slow->parent;
        fast = fast->parent->parent ? fast->parent->parent : fast->parent;
        
        if (slow == fast) {
            fprintf(stderr, "Error: Circular reference detected in parse tree\n");
            return 0;
        }
    }
    
    // Validate all children
    for (int i = 0; i < root->child_count; i++) {
        ParseTreeNode* child = root->children[i];
        
        if (!child) {
            fprintf(stderr, "Error: NULL child found in parse tree\n");
            return 0;
        }
        
        if (child->parent != root) {
            fprintf(stderr, "Error: Child parent reference is inconsistent\n");
            return 0;
        }
        
        if (!validate_parse_tree(child)) {
            return 0;
        }
    }
    
    return 1;
}

// Create a simple program tree for testing
ParseTreeNode* create_test_program_tree() {
    ParseTreeNode* program = create_parse_node(NODE_PROGRAM, "program", 0);
    set_node_scope(program, "global");
    
    // Add declaration: int x = 10;
    ParseTreeNode* decl1 = create_parse_node(NODE_DECLARATION, "x", 1);
    set_node_data_type(decl1, "int");
    set_node_scope(decl1, "global");
    
    ParseTreeNode* expr1 = create_parse_node(NODE_EXPRESSION, "10", 1);
    set_node_data_type(expr1, "int");
    
    add_child_node(decl1, expr1);
    add_child_node(program, decl1);
    
    // Add function: int main() { return 0; }
    ParseTreeNode* func1 = create_parse_node(NODE_FUNCTION, "main", 3);
    set_node_data_type(func1, "int");
    set_node_scope(func1, "global");
    
    ParseTreeNode* block1 = create_parse_node(NODE_BLOCK, "{", 3);
    set_node_scope(block1, "main");
    
    ParseTreeNode* stmt1 = create_parse_node(NODE_STATEMENT, "return", 4);
    set_node_data_type(stmt1, "int");
    set_node_scope(stmt1, "main");
    
    ParseTreeNode* expr2 = create_parse_node(NODE_EXPRESSION, "0", 4);
    set_node_data_type(expr2, "int");
    
    add_child_node(stmt1, expr2);
    add_child_node(block1, stmt1);
    add_child_node(func1, block1);
    add_child_node(program, func1);
    
    return program;
}
