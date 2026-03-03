#include "../include/symbol_table.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Global symbol table instance for the bridge
static SymbolTable *global_symbol_table = NULL;

// Initialize the symbol table
void *create_symbol_table_bridge() {
  if (global_symbol_table == NULL) {
    global_symbol_table = create_symbol_table();
  }
  return global_symbol_table;
}

// Insert symbol wrapper
int symbol_table_insert_bridge(void *st, const char *name, int data_type,
                               int kind, int line_number) {
  if (st == NULL)
    return 0;
  return symbol_table_insert((SymbolTable *)st, name, (DataType)data_type,
                             (SymbolKind)kind, line_number);
}

// Lookup symbol wrapper
void *symbol_table_lookup_bridge(void *st, const char *name) {
  if (st == NULL)
    return NULL;
  return symbol_table_lookup((SymbolTable *)st, name);
}

// Enter scope wrapper
void symbol_table_enter_scope_bridge(void *st, const char *scope_name) {
  if (st == NULL)
    return;
  symbol_table_enter_scope((SymbolTable *)st, scope_name);
}

// Exit scope wrapper
void symbol_table_exit_scope_bridge(void *st) {
  if (st == NULL)
    return;
  symbol_table_exit_scope((SymbolTable *)st);
}

// Set storage class wrapper
int symbol_table_set_storage_class_bridge(void *st, const char *name,
                                          int storage_class) {
  if (st == NULL)
    return 0;
  return symbol_table_set_storage_class((SymbolTable *)st, name,
                                        (StorageClass)storage_class);
}

// Set array dimensions wrapper
int symbol_table_set_array_dims_bridge(void *st, const char *name,
                                       int dimensions, int *sizes) {
  if (st == NULL)
    return 0;
  return symbol_table_set_array_dims((SymbolTable *)st, name, dimensions,
                                     sizes);
}

// Set function parameters wrapper
int symbol_table_set_function_params_bridge(void *st, const char *name,
                                            int num_params, int *param_types) {
  if (st == NULL)
    return 0;
  return symbol_table_set_function_params((SymbolTable *)st, name, num_params,
                                          (DataType *)param_types);
}

// Mark initialized wrapper
int symbol_table_mark_initialized_bridge(void *st, const char *name) {
  if (st == NULL)
    return 0;
  return symbol_table_mark_initialized((SymbolTable *)st, name);
}

// Check if declared wrapper
int symbol_table_is_declared_bridge(void *st, const char *name) {
  if (st == NULL)
    return 0;
  return symbol_table_is_declared((SymbolTable *)st, name);
}

// Set initial value wrapper (New)
int symbol_table_set_initial_value_bridge(void *st, const char *name,
                                          const char *value) {
  if (st == NULL)
    return 0;
  return symbol_table_set_initial_value((SymbolTable *)st, name, value);
}

// Delete symbol wrapper (New)
int symbol_table_delete_bridge(void *st, const char *name) {
  if (st == NULL)
    return 0;
  return symbol_table_delete((SymbolTable *)st, name);
}

// Get error count wrapper
int symbol_table_get_errors_bridge(void *st) {
  if (st == NULL)
    return 0;
  return symbol_table_get_errors((SymbolTable *)st);
}

// Free symbol table wrapper
void free_symbol_table_bridge(void *st) {
  if (st == NULL)
    return;
  free_symbol_table((SymbolTable *)st);
  if (st == global_symbol_table) {
    global_symbol_table = NULL;
  }
}

// Get all symbols (new function for web interface)
// Symbols with full details
typedef struct {
  char name[256];
  int data_type;
  int kind;
  int storage_class;
  int scope_level;
  char scope_name[64];
  int size;
  int offset;
  int line_number;
  int is_initialized;
  char initial_value[64];
  int num_parameters;
  int array_dimensions;
  int dimension_sizes[10];
} SymbolDetails;

typedef struct {
  SymbolDetails *symbols;
  int count;
  int capacity;
} SymbolDetailsList;

SymbolDetailsList *create_symbol_details_list() {
  SymbolDetailsList *list = malloc(sizeof(SymbolDetailsList));
  list->capacity = 100;
  list->symbols = malloc(sizeof(SymbolDetails) * list->capacity);
  list->count = 0;
  return list;
}

void free_symbol_details_list(SymbolDetailsList *list) {
  if (list == NULL)
    return;
  free(list->symbols);
  free(list);
}

void fill_symbol_details(SymbolDetails *details, SymbolEntry *entry) {
  if (!details || !entry)
    return;

  strncpy(details->name, entry->name, 255);
  details->name[255] = '\0';
  details->data_type = entry->data_type;
  details->kind = entry->kind;
  details->storage_class = entry->storage_class;
  details->scope_level = entry->scope_level;
  strncpy(details->scope_name, entry->scope_name, 63);
  details->scope_name[63] = '\0';
  details->size = entry->size;
  details->offset = entry->offset;
  details->line_number = entry->line_number;
  details->is_initialized = entry->is_initialized;
  strncpy(details->initial_value, entry->initial_value, 63);
  details->initial_value[63] = '\0';
  details->num_parameters = entry->num_parameters;
  details->array_dimensions = entry->array_dimensions;
  for (int i = 0; i < 10; i++) {
    details->dimension_sizes[i] =
        (i < entry->array_dimensions) ? entry->dimension_sizes[i] : 0;
  }
}

// Get all symbols with full details
SymbolDetailsList *get_all_symbol_details_bridge(void *st) {
  if (st == NULL)
    return NULL;

  SymbolTable *symbol_table = (SymbolTable *)st;
  SymbolDetailsList *list = create_symbol_details_list();
  HashTable *ht = symbol_table->hash_table;

  for (int i = 0; i < HASH_TABLE_SIZE; i++) {
    SymbolEntry *entry = ht->table[i];
    while (entry != NULL) {
      if (list->count >= list->capacity) {
        list->capacity *= 2;
        list->symbols =
            realloc(list->symbols, sizeof(SymbolDetails) * list->capacity);
      }
      fill_symbol_details(&list->symbols[list->count++], entry);
      entry = entry->next;
    }
  }
  return list;
}

SymbolDetails *get_symbol_details_bridge(void *st, const char *name) {
  if (st == NULL || name == NULL)
    return NULL;

  SymbolEntry *entry = symbol_table_lookup((SymbolTable *)st, name);
  if (entry == NULL)
    return NULL;

  SymbolDetails *details = malloc(sizeof(SymbolDetails));
  if (details == NULL)
    return NULL;

  fill_symbol_details(details, entry);
  return details;
}

// Free symbol details
void free_symbol_details_bridge(SymbolDetails *details) {
  if (details != NULL) {
    free(details);
  }
}

// Scope Hierarchy Support
typedef struct {
  char name[64];
  int level;
  char parent_name[64];
} ScopeInfo;

typedef struct {
  ScopeInfo *scopes;
  int count;
  int capacity;
} ScopeList;

ScopeList *create_scope_list() {
  ScopeList *list = malloc(sizeof(ScopeList));
  list->capacity = 100;
  list->scopes = malloc(sizeof(ScopeInfo) * list->capacity);
  list->count = 0;
  return list;
}

void collect_scopes_recursive(Scope *scope, ScopeList *list) {
  if (!scope || !list)
    return;

  if (list->count < list->capacity) {
    ScopeInfo *info = &list->scopes[list->count++];
    strncpy(info->name, scope->scope_name, 63);
    info->name[63] = '\0';
    info->level = scope->level;
    if (scope->parent) {
      strncpy(info->parent_name, scope->parent->scope_name, 63);
      info->parent_name[63] = '\0';
    } else {
      info->parent_name[0] = '\0';
    }
  }

  // Traverse children
  Scope *child = scope->children;
  while (child) {
    collect_scopes_recursive(child, list);
    child = child->next;
  }
}

ScopeList *get_scope_hierarchy_bridge(void *st) {
  if (!st)
    return NULL;
  SymbolTable *sym_table = (SymbolTable *)st;

  ScopeList *list = create_scope_list();
  if (sym_table->scope_manager && sym_table->scope_manager->global_scope) {
    collect_scopes_recursive(sym_table->scope_manager->global_scope, list);
  }
  return list;
}

void free_scope_list_bridge(ScopeList *list) {
  if (list) {
    free(list->scopes);
    free(list);
  }
}
