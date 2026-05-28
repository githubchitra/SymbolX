#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../include/error_detection.h"
#include "../include/parse_tree.h"
#include "../include/symbol_table.h"

// Error detection context
typedef struct ErrorContext {
    SymbolTable* symbol_table;
    ParseTreeNode* parse_tree;
    ErrorInfo* errors;
    int error_count;
    int error_capacity;
    int current_line;
    int current_scope_level;
    char* current_scope;
} ErrorContext;

// Create error detection context
ErrorContext* create_error_context(SymbolTable* symbol_table, ParseTreeNode* parse_tree) {
    ErrorContext* ctx = (ErrorContext*)malloc(sizeof(ErrorContext));
    if (!ctx) return NULL;
    
    ctx->symbol_table = symbol_table;
    ctx->parse_tree = parse_tree;
    ctx->errors = NULL;
    ctx->error_count = 0;
    ctx->error_capacity = 0;
    ctx->current_line = 0;
    ctx->current_scope_level = 0;
    ctx->current_scope = strdup("global");
    
    return ctx;
}

// Add error to context
int add_error(ErrorContext* ctx, ErrorType type, ErrorSeverity severity, 
              int line, int column, const char* message, const char* suggestion) {
    if (!ctx || !message) return 0;
    
    // Resize error array if needed
    if (ctx->error_count >= ctx->error_capacity) {
        int new_capacity = ctx->error_capacity == 0 ? 16 : ctx->error_capacity * 2;
        ErrorInfo* new_errors = (ErrorInfo*)realloc(ctx->errors, new_capacity * sizeof(ErrorInfo));
        if (!new_errors) return 0;
        
        ctx->errors = new_errors;
        ctx->error_capacity = new_capacity;
    }
    
    ErrorInfo* error = &ctx->errors[ctx->error_count];
    error->type = type;
    error->severity = severity;
    error->line = line;
    error->column = column;
    error->message = strdup(message);
    error->suggestion = suggestion ? strdup(suggestion) : NULL;
    error->scope = ctx->current_scope ? strdup(ctx->current_scope) : NULL;
    error->node_id = NULL;
    
    ctx->error_count++;
    return 1;
}

// Free error context
void free_error_context(ErrorContext* ctx) {
    if (!ctx) return;
    
    if (ctx->errors) {
        for (int i = 0; i < ctx->error_count; i++) {
            if (ctx->errors[i].message) free(ctx->errors[i].message);
            if (ctx->errors[i].suggestion) free(ctx->errors[i].suggestion);
            if (ctx->errors[i].scope) free(ctx->errors[i].scope);
            if (ctx->errors[i].node_id) free(ctx->errors[i].node_id);
        }
        free(ctx->errors);
    }
    
    if (ctx->current_scope) free(ctx->current_scope);
    free(ctx);
}

// Get error type as string
const char* error_type_to_string(ErrorType type) {
    switch (type) {
        case ERROR_SYNTAX: return "syntax";
        case ERROR_SEMANTIC: return "semantic";
        case ERROR_LOGIC: return "logic";
        case ERROR_RUNTIME: return "runtime";
        case ERROR_TYPE: return "type";
        case ERROR_SCOPE: return "scope";
        case ERROR_MEMORY: return "memory";
        case ERROR_SECURITY: return "security";
        case ERROR_PERFORMANCE: return "performance";
        case ERROR_STYLE: return "style";
        default: return "unknown";
    }
}

// Get error severity as string
const char* error_severity_to_string(ErrorSeverity severity) {
    switch (severity) {
        case SEVERITY_CRITICAL: return "critical";
        case SEVERITY_HIGH: return "high";
        case SEVERITY_MEDIUM: return "medium";
        case SEVERITY_LOW: return "low";
        case SEVERITY_INFO: return "info";
        default: return "unknown";
    }
}

// Check if token is a C keyword
int is_c_keyword(const char* token) {
    const char* keywords[] = {
        "auto", "break", "case", "char", "const", "continue", "default",
        "do", "double", "else", "enum", "extern", "float", "for", "goto",
        "if", "int", "long", "register", "return", "short", "signed",
        "sizeof", "static", "struct", "switch", "typedef", "union",
        "unsigned", "void", "volatile", "while"
    };
    
    int num_keywords = sizeof(keywords) / sizeof(keywords[0]);
    for (int i = 0; i < num_keywords; i++) {
        if (strcmp(token, keywords[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

// Detect syntax errors in code
void detect_syntax_errors(ErrorContext* ctx, const char* code) {
    if (!ctx || !code) return;
    
    int line = 1;
    int column = 1;
    int brace_count = 0;
    int paren_count = 0;
    int bracket_count = 0;
    
    for (int i = 0; code[i] != '\0'; i++) {
        char c = code[i];
        
        switch (c) {
            case '\n':
                line++;
                column = 1;
                break;
            case '{':
                brace_count++;
                column++;
                break;
            case '}':
                brace_count--;
                column++;
                if (brace_count < 0) {
                    add_error(ctx, ERROR_SYNTAX, SEVERITY_HIGH, line, column,
                             "Unmatched closing brace", "Check for missing opening brace");
                    brace_count = 0;
                }
                break;
            case '(':
                paren_count++;
                column++;
                break;
            case ')':
                paren_count--;
                column++;
                if (paren_count < 0) {
                    add_error(ctx, ERROR_SYNTAX, SEVERITY_HIGH, line, column,
                             "Unmatched closing parenthesis", "Check for missing opening parenthesis");
                    paren_count = 0;
                }
                break;
            case '[':
                bracket_count++;
                column++;
                break;
            case ']':
                bracket_count--;
                column++;
                if (bracket_count < 0) {
                    add_error(ctx, ERROR_SYNTAX, SEVERITY_HIGH, line, column,
                             "Unmatched closing bracket", "Check for missing opening bracket");
                    bracket_count = 0;
                }
                break;
            case ';':
                column++;
                break;
            default:
                column++;
                break;
        }
    }
    
    // Check for unclosed braces at end of file
    if (brace_count > 0) {
        add_error(ctx, ERROR_SYNTAX, SEVERITY_HIGH, line, column,
                 "Unclosed brace(s)", "Add missing closing brace(s)");
    }
    
    if (paren_count > 0) {
        add_error(ctx, ERROR_SYNTAX, SEVERITY_HIGH, line, column,
                 "Unclosed parenthesis", "Add missing closing parenthesis");
    }
    
    if (bracket_count > 0) {
        add_error(ctx, ERROR_SYNTAX, SEVERITY_HIGH, line, column,
                 "Unclosed bracket(s)", "Add missing closing bracket(s)");
    }
}

// Detect semantic errors
void detect_semantic_errors(ErrorContext* ctx, const char* code) {
    if (!ctx || !code) return;
    
    // Simple tokenization for basic semantic analysis
    char* code_copy = strdup(code);
    char* line = strtok(code_copy, "\n");
    int line_num = 1;
    
    while (line != NULL) {
        // Skip comments and empty lines
        char* trimmed = line;
        while (isspace(*trimmed)) trimmed++;
        
        if (strlen(trimmed) > 0 && !strstr(trimmed, "//") && !strstr(trimmed, "/*")) {
            // Look for variable usage
            char* token = strtok(trimmed, " \t,;(){}[]");
            while (token != NULL) {
                // Check for undeclared variables (simplified)
                if (isalpha(token[0]) && !is_c_keyword(token)) {
                    // This is a simplified check - in a real parser, we'd have
                    // proper symbol table integration
                    if (strstr(code, token) && !strstr(code, "int") && 
                        !strstr(code, "float") && !strstr(code, "char") &&
                        !strstr(code, "double") && !strstr(code, "void")) {
                        
                        // Check if this looks like a variable assignment or usage
                        char* next_token = strtok(NULL, " \t,;(){}[]");
                        if (next_token && (strcmp(next_token, "=") == 0 || 
                                         strstr(line, "="))) {
                            add_error(ctx, ERROR_SEMANTIC, SEVERITY_MEDIUM, line_num, 0,
                                     "Possible use of undeclared variable", 
                                     "Declare the variable before using it");
                        }
                    }
                }
                token = strtok(NULL, " \t,;(){}[]");
            }
        }
        
        line = strtok(NULL, "\n");
        line_num++;
    }
    
    free(code_copy);
}

// Detect logic errors
void detect_logic_errors(ErrorContext* ctx, const char* code) {
    if (!ctx || !code) return;
    
    char* code_copy = strdup(code);
    char* line = strtok(code_copy, "\n");
    int line_num = 1;
    
    while (line != NULL) {
        // Check for potential infinite loops
        if (strstr(line, "for") && strstr(line, ";")) {
            int semicolon_count = 0;
            for (int i = 0; line[i] != '\0'; i++) {
                if (line[i] == ';') semicolon_count++;
            }
            
            if (semicolon_count >= 3) {
                char* condition = strchr(line, ';');
                if (condition) {
                    condition = strchr(condition + 1, ';');
                    if (condition) {
                        condition++;
                        while (isspace(*condition)) condition++;
                        if (*condition == ';' || *condition == '\0') {
                            add_error(ctx, ERROR_LOGIC, SEVERITY_HIGH, line_num, 0,
                                     "Potential infinite loop", 
                                     "Add a proper loop condition or break statement");
                        }
                    }
                }
            }
        }
        
        // Check for unreachable code after return
        if (strstr(line, "return")) {
            char* next_line = strtok(NULL, "\n");
            if (next_line) {
                char* trimmed = next_line;
                while (isspace(*trimmed)) trimmed++;
                if (strlen(trimmed) > 0 && !strstr(trimmed, "}") && 
                    !strstr(trimmed, "//") && !strstr(trimmed, "/*")) {
                    add_error(ctx, ERROR_LOGIC, SEVERITY_MEDIUM, line_num + 1, 0,
                             "Unreachable code", 
                             "Code after return statement will never execute");
                }
                // Put the line back for processing
                char* rest_of_code = strdup(next_line);
                char* remaining = strtok(NULL, "\n");
                if (remaining) {
                    char* temp = malloc(strlen(rest_of_code) + strlen(remaining) + 2);
                    strcpy(temp, rest_of_code);
                    strcat(temp, "\n");
                    strcat(temp, remaining);
                    // This is a bit hacky but works for this simple implementation
                }
                free(rest_of_code);
            }
        }
        
        line = strtok(NULL, "\n");
        line_num++;
    }
    
    free(code_copy);
}

// Detect type errors
void detect_type_errors(ErrorContext* ctx, const char* code) {
    if (!ctx || !code) return;
    
    char* code_copy = strdup(code);
    char* line = strtok(code_copy, "\n");
    int line_num = 1;
    
    while (line != NULL) {
        // Check for type mismatches in assignments
        if (strstr(line, "=") && !strstr(line, "==")) {
            char* left_side = strtok(line, "=");
            char* right_side = strtok(NULL, "=");
            
            if (left_side && right_side) {
                // Check for assigning string to integer
                if (strstr(left_side, "int") && strstr(right_side, "\"")) {
                    add_error(ctx, ERROR_TYPE, SEVERITY_HIGH, line_num, 0,
                             "Type mismatch: cannot assign string to integer",
                             "Use appropriate type or convert the value");
                }
                
                // Check for assigning integer to string
                if (strstr(left_side, "char") && strstr(right_side, "*") == NULL &&
                    strstr(left_side, "*") && !strstr(right_side, "\"")) {
                    add_error(ctx, ERROR_TYPE, SEVERITY_HIGH, line_num, 0,
                             "Type mismatch: cannot assign integer to string pointer",
                             "Use string literal or proper string assignment");
                }
            }
        }
        
        line = strtok(NULL, "\n");
        line_num++;
    }
    
    free(code_copy);
}

// Detect security issues
void detect_security_issues(ErrorContext* ctx, const char* code) {
    if (!ctx || !code) return;
    
    char* code_copy = strdup(code);
    char* line = strtok(code_copy, "\n");
    int line_num = 1;
    
    while (line != NULL) {
        // Check for dangerous functions
        if (strstr(line, "gets(")) {
            add_error(ctx, ERROR_SECURITY, SEVERITY_CRITICAL, line_num, 0,
                     "Dangerous function: gets() can cause buffer overflow",
                     "Use fgets() instead of gets()");
        }
        
        if (strstr(line, "strcpy(")) {
            add_error(ctx, ERROR_SECURITY, SEVERITY_HIGH, line_num, 0,
                     "Potentially unsafe function: strcpy() can cause buffer overflow",
                     "Use strncpy() with size limit instead");
        }
        
        if (strstr(line, "strcat(")) {
            add_error(ctx, ERROR_SECURITY, SEVERITY_HIGH, line_num, 0,
                     "Potentially unsafe function: strcat() can cause buffer overflow",
                     "Use strncat() with size limit instead");
        }
        
        // Check for format string vulnerabilities
        if (strstr(line, "printf") && !strstr(line, "%")) {
            char* paren_pos = strchr(line, '(');
            if (paren_pos) {
                char* closing_paren = strchr(paren_pos, ')');
                if (closing_paren) {
                    int length = closing_paren - paren_pos - 1;
                    if (length > 0) {
                        char* args = malloc(length + 1);
                        strncpy(args, paren_pos + 1, length);
                        args[length] = '\0';
                        
                        if (strchr(args, ',') && !strchr(args, '"')) {
                            add_error(ctx, ERROR_SECURITY, SEVERITY_HIGH, line_num, 0,
                                     "Potential format string vulnerability",
                                     "Use proper format string with specifiers");
                        }
                        free(args);
                    }
                }
            }
        }
        
        line = strtok(NULL, "\n");
        line_num++;
    }
    
    free(code_copy);
}

// Detect performance issues
void detect_performance_issues(ErrorContext* ctx, const char* code) {
    if (!ctx || !code) return;
    
    char* code_copy = strdup(code);
    char* line = strtok(code_copy, "\n");
    int line_num = 1;
    
    while (line != NULL) {
        // Check for inefficient string operations in loops
        if (strstr(line, "for") && strstr(line, "strlen")) {
            add_error(ctx, ERROR_PERFORMANCE, SEVERITY_MEDIUM, line_num, 0,
                     "Inefficient: strlen() in loop condition",
                     "Calculate string length once before the loop");
        }
        
        // Check for memory allocation without free
        if (strstr(line, "malloc") || strstr(line, "calloc")) {
            // Simple check - in real implementation, we'd track allocations
            add_error(ctx, ERROR_PERFORMANCE, SEVERITY_MEDIUM, line_num, 0,
                     "Potential memory leak: allocated memory may not be freed",
                     "Ensure allocated memory is freed when no longer needed");
        }
        
        line = strtok(NULL, "\n");
        line_num++;
    }
    
    free(code_copy);
}

// Main error detection function
ErrorReport* detect_errors(const char* code, SymbolTable* symbol_table, ParseTreeNode* parse_tree) {
    if (!code) return NULL;
    
    ErrorContext* ctx = create_error_context(symbol_table, parse_tree);
    if (!ctx) return NULL;
    
    // Run all error detection algorithms
    detect_syntax_errors(ctx, code);
    detect_semantic_errors(ctx, code);
    detect_logic_errors(ctx, code);
    detect_type_errors(ctx, code);
    detect_security_issues(ctx, code);
    detect_performance_issues(ctx, code);
    
    // Create error report
    ErrorReport* report = (ErrorReport*)malloc(sizeof(ErrorReport));
    if (!report) {
        free_error_context(ctx);
        return NULL;
    }
    
    report->errors = ctx->errors;
    report->error_count = ctx->error_count;
    report->symbol_table = symbol_table;
    report->parse_tree = parse_tree;
    
    // Calculate error statistics
    report->stats.total_errors = 0;
    report->stats.total_warnings = 0;
    report->stats.critical_errors = 0;
    report->stats.high_errors = 0;
    report->stats.medium_errors = 0;
    report->stats.low_errors = 0;
    
    for (int i = 0; i < ctx->error_count; i++) {
        ErrorInfo* error = &ctx->errors[i];
        if (error->severity <= SEVERITY_MEDIUM) {
            report->stats.total_errors++;
        } else {
            report->stats.total_warnings++;
        }
        
        switch (error->severity) {
            case SEVERITY_CRITICAL: report->stats.critical_errors++; break;
            case SEVERITY_HIGH: report->stats.high_errors++; break;
            case SEVERITY_MEDIUM: report->stats.medium_errors++; break;
            case SEVERITY_LOW: report->stats.low_errors++; break;
            default: break;
        }
    }
    
    // Free context but keep errors (they're now owned by report)
    ctx->errors = NULL;
    ctx->error_count = 0;
    free_error_context(ctx);
    
    return report;
}

// Free error report
void free_error_report(ErrorReport* report) {
    if (!report) return;
    
    if (report->errors) {
        for (int i = 0; i < report->error_count; i++) {
            if (report->errors[i].message) free(report->errors[i].message);
            if (report->errors[i].suggestion) free(report->errors[i].suggestion);
            if (report->errors[i].scope) free(report->errors[i].scope);
            if (report->errors[i].node_id) free(report->errors[i].node_id);
        }
        free(report->errors);
    }
    
    free(report);
}

// Print error report
void print_error_report(ErrorReport* report) {
    if (!report) return;
    
    printf("=== ERROR REPORT ===\n");
    printf("Total Errors: %d\n", report->stats.total_errors);
    printf("Total Warnings: %d\n", report->stats.total_warnings);
    printf("Critical: %d, High: %d, Medium: %d, Low: %d\n\n",
           report->stats.critical_errors, report->stats.high_errors,
           report->stats.medium_errors, report->stats.low_errors);
    
    for (int i = 0; i < report->error_count; i++) {
        ErrorInfo* error = &report->errors[i];
        printf("[%s] %s (Line %d): %s\n",
               error_severity_to_string(error->severity),
               error_type_to_string(error->type),
               error->line,
               error->message);
        
        if (error->suggestion) {
            printf("  Suggestion: %s\n", error->suggestion);
        }
        if (error->scope) {
            printf("  Scope: %s\n", error->scope);
        }
        printf("\n");
    }
}
