#include "../include/symbol_table.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

SymbolTable *create_symbol_table() {
  SymbolTable *st = (SymbolTable *)malloc(sizeof(SymbolTable));
  if (!st) {
    fprintf(stderr, "Error: Memory allocation failed for symbol table\n");
    return NULL;
  }

  st->hash_table = create_hash_table();
  st->scope_manager = create_scope_manager();
  st->error_count = 0;

  if (!st->hash_table || !st->scope_manager) {
    fprintf(stderr, "Error: Failed to initialize symbol table components\n");
    if (st->hash_table)
      free_hash_table(st->hash_table);
    if (st->scope_manager)
      free_scope_manager(st->scope_manager);
    free(st);
    return NULL;
  }

  return st;
}

int symbol_table_insert(SymbolTable *st, const char *name, DataType type,
                        SymbolKind kind, int line_number) {
  if (!st || !name)
    return 0;

  int scope_level = get_current_scope_level(st->scope_manager);
  const char *scope_name = get_current_scope_name(st->scope_manager);

  int result = insert_symbol(st->hash_table, name, type, kind, scope_level,
                             scope_name, line_number);

  if (!result) {
    st->error_count++;
  }

  return result;
}

SymbolEntry *symbol_table_lookup(SymbolTable *st, const char *name) {
  if (!st || !name)
    return NULL;

  return lookup_symbol_recursive(st->hash_table, name, st->scope_manager);
}

void symbol_table_enter_scope(SymbolTable *st, const char *scope_name) {
  if (!st)
    return;
  enter_scope(st->scope_manager, scope_name);
}

void symbol_table_exit_scope(SymbolTable *st) {
  if (!st)
    return;

  // Commented out to preserve symbols for the UI display
  // int scope_level = get_current_scope_level(st->scope_manager);
  // const char *scope_name = get_current_scope_name(st->scope_manager);
  // delete_scope_symbols(st->hash_table, scope_level, scope_name);
  
  exit_scope(st->scope_manager);
}

int symbol_table_set_storage_class(SymbolTable *st, const char *name,
                                   StorageClass sc) {
  if (!st || !name)
    return 0;

  SymbolEntry *entry = symbol_table_lookup(st, name);
  if (!entry) {
    fprintf(stderr, "Error: Symbol '%s' not found\n", name);
    st->error_count++;
    return 0;
  }

  entry->storage_class = sc;
  return 1;
}

int symbol_table_set_array_dims(SymbolTable *st, const char *name,
                                int dimensions, int *sizes) {
  if (!st || !name || !sizes || dimensions <= 0)
    return 0;

  SymbolEntry *entry = symbol_table_lookup(st, name);
  if (!entry) {
    fprintf(stderr, "Error: Symbol '%s' not found\n", name);
    st->error_count++;
    return 0;
  }

  // Free previous dimension sizes if any
  if (entry->dimension_sizes) {
    free(entry->dimension_sizes);
  }

  entry->array_dimensions = dimensions;
  entry->dimension_sizes = (int *)malloc(dimensions * sizeof(int));
  if (!entry->dimension_sizes) {
    fprintf(stderr, "Error: Memory allocation failed for array dimensions\n");
    st->error_count++;
    return 0;
  }

  int total_size = 1;
  for (int i = 0; i < dimensions; i++) {
    entry->dimension_sizes[i] = sizes[i];
    total_size *= sizes[i];
  }
  entry->size = total_size;

  return 1;
}

int symbol_table_set_function_params(SymbolTable *st, const char *name,
                                     int num_params, DataType *param_types) {
  if (!st || !name || (num_params > 0 && !param_types))
    return 0;

  SymbolEntry *entry = symbol_table_lookup(st, name);
  if (!entry) {
    fprintf(stderr, "Error: Symbol '%s' not found\n", name);
    st->error_count++;
    return 0;
  }

  if (entry->kind != SYMBOL_FUNCTION) {
    fprintf(stderr, "Error: Symbol '%s' is not a function\n", name);
    st->error_count++;
    return 0;
  }

  // Free previous parameter types if any
  if (entry->param_types) {
    free(entry->param_types);
  }

  entry->num_parameters = num_params;
  if (num_params > 0) {
    entry->param_types = (DataType *)malloc(num_params * sizeof(DataType));
    if (!entry->param_types) {
      fprintf(stderr,
              "Error: Memory allocation failed for function parameters\n");
      st->error_count++;
      return 0;
    }

    for (int i = 0; i < num_params; i++) {
      entry->param_types[i] = param_types[i];
    }
  } else {
    entry->param_types = NULL;
  }

  return 1;
}

int symbol_table_mark_initialized(SymbolTable *st, const char *name) {
  if (!st || !name)
    return 0;

  SymbolEntry *entry = symbol_table_lookup(st, name);
  if (!entry) {
    fprintf(stderr, "Error: Symbol '%s' not found\n", name);
    st->error_count++;
    return 0;
  }

  entry->is_initialized = 1;
  return 1;
}

int symbol_table_is_declared(SymbolTable *st, const char *name) {
  if (!st || !name)
    return 0;

  SymbolEntry *entry = symbol_table_lookup(st, name);
  return (entry != NULL);
}

int symbol_table_set_initial_value(SymbolTable *st, const char *name,
                                   const char *value) {
  if (!st || !name || !value)
    return 0;

  SymbolEntry *entry = symbol_table_lookup(st, name);
  if (!entry) {
    fprintf(stderr, "Error: Symbol '%s' not found\n", name);
    st->error_count++;
    return 0;
  }

  strncpy(entry->initial_value, value, 63);
  entry->initial_value[63] = '\0';
  return 1;
}

int symbol_table_delete(SymbolTable *st, const char *name) {
  if (!st || !name)
    return 0;

  // Attempt to delete from hash table (which handles specific scope logic or
  // just all with that name?) Our hash table lookup finds the first one (most
  // local). But delete_symbol in hash_table.c deletes the first one it finds
  // matching the name. This is correct for shadowing: we delete the most local
  // one.

  if (delete_symbol(st->hash_table, name)) {
    return 1;
  }

  return 0;
}

void symbol_table_display(SymbolTable *st) {
  if (!st)
    return;

  display_hash_table(st->hash_table);
}

void symbol_table_display_current_scope(SymbolTable *st) {
  if (!st)
    return;

  int scope_level = get_current_scope_level(st->scope_manager);
  const char *scope_name = get_current_scope_name(st->scope_manager);

  display_scope_symbols(st->hash_table, scope_level, scope_name);
}

void symbol_table_display_all_scopes(SymbolTable *st) {
  if (!st)
    return;

  printf("\n========== SCOPE HIERARCHY ==========\n");
  if (st->scope_manager && st->scope_manager->global_scope) {
    print_scope_hierarchy(st->scope_manager->global_scope, 0);
  } else {
    printf("  (Scope information unavailable)\n");
  }
  printf("=====================================\n\n");
}

void free_symbol_table(SymbolTable *st) {
  if (!st)
    return;

  if (st->hash_table)
    free_hash_table(st->hash_table);
  if (st->scope_manager)
    free_scope_manager(st->scope_manager);
  free(st);
}

void symbol_table_error(SymbolTable *st, const char *message) {
  if (!st)
    return;

  fprintf(stderr, "Error: %s\n", message);
  st->error_count++;
}

int symbol_table_get_errors(SymbolTable *st) {
  if (!st)
    return -1;
  return st->error_count;
}
