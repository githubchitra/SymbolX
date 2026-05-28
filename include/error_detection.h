#ifndef ERROR_DETECTION_H
#define ERROR_DETECTION_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symbol_table.h"
#include "parse_tree.h"

// Forward declarations
typedef struct ErrorContext ErrorContext;

// Error types
typedef enum {
    ERROR_SYNTAX,
    ERROR_SEMANTIC,
    ERROR_LOGIC,
    ERROR_RUNTIME,
    ERROR_TYPE,
    ERROR_SCOPE,
    ERROR_MEMORY,
    ERROR_SECURITY,
    ERROR_PERFORMANCE,
    ERROR_STYLE
} ErrorType;

// Error severity levels
typedef enum {
    SEVERITY_CRITICAL,
    SEVERITY_HIGH,
    SEVERITY_MEDIUM,
    SEVERITY_LOW,
    SEVERITY_INFO
} ErrorSeverity;

// Error information structure
typedef struct {
    ErrorType type;
    ErrorSeverity severity;
    int line;
    int column;
    char* message;
    char* suggestion;
    char* scope;
    char* node_id;
} ErrorInfo;

// Error statistics
typedef struct {
    int total_errors;
    int total_warnings;
    int critical_errors;
    int high_errors;
    int medium_errors;
    int low_errors;
} ErrorStats;

// Error report structure
typedef struct {
    ErrorInfo* errors;
    int error_count;
    ErrorStats stats;
    SymbolTable* symbol_table;
    ParseTreeNode* parse_tree;
} ErrorReport;

// Constants
#define MAX_ERRORS_PER_LINE 10
#define MAX_ERROR_MESSAGE_LENGTH 256

// Function declarations

// Main error detection function
ErrorReport* detect_errors(const char* code, SymbolTable* symbol_table, ParseTreeNode* parse_tree);

// Error report management
void free_error_report(ErrorReport* report);
void print_error_report(ErrorReport* report);

// Utility functions
const char* error_type_to_string(ErrorType type);
const char* error_severity_to_string(ErrorSeverity severity);

// Individual error detection functions
void detect_syntax_errors(ErrorContext* ctx, const char* code);
void detect_semantic_errors(ErrorContext* ctx, const char* code);
void detect_logic_errors(ErrorContext* ctx, const char* code);
void detect_type_errors(ErrorContext* ctx, const char* code);
void detect_security_issues(ErrorContext* ctx, const char* code);
void detect_performance_issues(ErrorContext* ctx, const char* code);

// Context management
ErrorContext* create_error_context(SymbolTable* symbol_table, ParseTreeNode* parse_tree);
void free_error_context(ErrorContext* ctx);
int add_error(ErrorContext* ctx, ErrorType type, ErrorSeverity severity, 
               int line, int column, const char* message, const char* suggestion);

#endif // ERROR_DETECTION_H
