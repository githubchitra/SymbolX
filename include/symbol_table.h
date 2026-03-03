#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include "data_types.h"
#include "hash_table.h"
#include "scope.h"

// Symbol table structure combining hash table and scope manager
typedef struct {
  HashTable *hash_table;
  ScopeManager *scope_manager;
  int error_count;
} SymbolTable;

// Create a new symbol table
SymbolTable *create_symbol_table();

// Insert a new symbol
int symbol_table_insert(SymbolTable *st, const char *name, DataType type,
                        SymbolKind kind, int line_number);

// Lookup a symbol
SymbolEntry *symbol_table_lookup(SymbolTable *st, const char *name);

// Enter a new scope
void symbol_table_enter_scope(SymbolTable *st, const char *scope_name);

// Exit current scope
void symbol_table_exit_scope(SymbolTable *st);

// Set storage class for a symbol
int symbol_table_set_storage_class(SymbolTable *st, const char *name,
                                   StorageClass sc);

// Set array dimensions
int symbol_table_set_array_dims(SymbolTable *st, const char *name,
                                int dimensions, int *sizes);

// Set function parameters
int symbol_table_set_function_params(SymbolTable *st, const char *name,
                                     int num_params, DataType *param_types);

// Mark symbol as initialized
int symbol_table_mark_initialized(SymbolTable *st, const char *name);

// Check if symbol is declared
int symbol_table_is_declared(SymbolTable *st, const char *name);

// Set initial value string
int symbol_table_set_initial_value(SymbolTable *st, const char *name,
                                   const char *value);

// Delete a symbol
int symbol_table_delete(SymbolTable *st, const char *name);

// Display symbol table
void symbol_table_display(SymbolTable *st);

// Display current scope
void symbol_table_display_current_scope(SymbolTable *st);

// Display all scopes
void symbol_table_display_all_scopes(SymbolTable *st);

// Free symbol table
void free_symbol_table(SymbolTable *st);

// Error reporting
void symbol_table_error(SymbolTable *st, const char *message);

// Get error count
int symbol_table_get_errors(SymbolTable *st);

#endif
