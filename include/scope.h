#ifndef SCOPE_H
#define SCOPE_H

#include "data_types.h"

#define MAX_SCOPE_DEPTH 100

// Scope structure
typedef struct Scope {
    int level;                  // Nesting level (0 = global, 1+ = local)
    char scope_name[64];        // Name of the scope (function name, block name)
    struct Scope* parent;       // Parent scope
    struct Scope* next;         // Next sibling scope
    struct Scope* children;     // Child scopes
} Scope;

// Scope manager
typedef struct {
    Scope* global_scope;        // Global scope
    Scope* current_scope;       // Current active scope
    int scope_counter;          // Counter for generating unique scope names
} ScopeManager;

// Create a new scope manager
ScopeManager* create_scope_manager();

// Enter a new scope
void enter_scope(ScopeManager* manager, const char* scope_name);

// Exit current scope
void exit_scope(ScopeManager* manager);

// Get current scope level
int get_current_scope_level(ScopeManager* manager);

// Get current scope name
const char* get_current_scope_name(ScopeManager* manager);

// Free scope manager
void free_scope_manager(ScopeManager* manager);

// Print scope hierarchy
void print_scope_hierarchy(Scope* scope, int indent);

#endif
