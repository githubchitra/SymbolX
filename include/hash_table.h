#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include "data_types.h"
#include "scope.h"

#define HASH_TABLE_SIZE 211 // Prime number for better distribution

// Symbol entry in the hash table
typedef struct SymbolEntry {
  char *name;                 // Symbol name
  DataType data_type;         // Data type
  SymbolKind kind;            // Symbol kind
  StorageClass storage_class; // Storage class
  int scope_level;            // Scope level
  char scope_name[64];        // Scope name

  // Additional attributes
  int size;               // Size (for arrays)
  int offset;             // Memory offset
  int line_number;        // Line where declared
  int is_initialized;     // Initialization flag
  char initial_value[64]; // Initial value as string

  // Function-specific attributes
  int num_parameters;    // Number of parameters (for functions)
  DataType *param_types; // Parameter types (for functions)

  // Array-specific attributes
  int array_dimensions; // Number of dimensions
  int *dimension_sizes; // Size of each dimension

  struct SymbolEntry *next; // For collision handling (chaining)
} SymbolEntry;

// Hash table structure
typedef struct {
  SymbolEntry *table[HASH_TABLE_SIZE];
  int count; // Number of symbols
} HashTable;

// Create a new hash table
HashTable *create_hash_table();

// Hash function
unsigned int hash(const char *key);

// Insert a symbol into hash table
int insert_symbol(HashTable *ht, const char *name, DataType type,
                  SymbolKind kind, int scope_level, const char *scope_name,
                  int line_number);

// Lookup a symbol in hash table
SymbolEntry *lookup_symbol(HashTable *ht, const char *name, int scope_level,
                           const char *scope_name);

// Lookup symbol in current or outer scopes
SymbolEntry *lookup_symbol_recursive(HashTable *ht, const char *name,
                                     ScopeManager *sm);

// Delete a specific symbol by name
int delete_symbol(HashTable *ht, const char *name);

// Delete symbols from a specific scope
void delete_scope_symbols(HashTable *ht, int scope_level,
                          const char *scope_name);

// Free hash table
void free_hash_table(HashTable *ht);

// Display hash table contents
void display_hash_table(HashTable *ht);

// Display symbols in a specific scope
void display_scope_symbols(HashTable *ht, int scope_level,
                           const char *scope_name);

#endif
