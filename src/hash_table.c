#include "../include/hash_table.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


HashTable *create_hash_table() {
  HashTable *ht = (HashTable *)malloc(sizeof(HashTable));
  if (!ht) {
    fprintf(stderr, "Error: Memory allocation failed for hash table\n");
    return NULL;
  }

  for (int i = 0; i < HASH_TABLE_SIZE; i++) {
    ht->table[i] = NULL;
  }
  ht->count = 0;

  return ht;
}

unsigned int hash(const char *key) {
  unsigned int hash_value = 0;
  unsigned int i = 0;

  while (key[i] != '\0') {
    hash_value = hash_value * 31 + key[i];
    i++;
  }

  return hash_value % HASH_TABLE_SIZE;
}

int insert_symbol(HashTable *ht, const char *name, DataType type,
                  SymbolKind kind, int scope_level, const char *scope_name,
                  int line_number) {
  if (!ht || !name || !scope_name)
    return 0;

  unsigned int index = hash(name);

  // Check if symbol already exists in the same scope
  SymbolEntry *current = ht->table[index];
  while (current != NULL) {
    if (strcmp(current->name, name) == 0 &&
        current->scope_level == scope_level &&
        strcmp(current->scope_name, scope_name) == 0) {
      fprintf(stderr,
              "Error: Symbol '%s' already declared in scope '%s' at line %d\n",
              name, scope_name, current->line_number);
      return 0;
    }
    current = current->next;
  }

  // Create new symbol entry
  SymbolEntry *new_entry = (SymbolEntry *)malloc(sizeof(SymbolEntry));
  if (!new_entry) {
    fprintf(stderr, "Error: Memory allocation failed for symbol entry\n");
    return 0;
  }

  new_entry->name = (char *)malloc(strlen(name) + 1);
  if (!new_entry->name) {
    fprintf(stderr, "Error: Memory allocation failed for symbol name\n");
    free(new_entry);
    return 0;
  }
  strcpy(new_entry->name, name);

  new_entry->data_type = type;
  new_entry->kind = kind;
  new_entry->storage_class = STORAGE_AUTO;
  new_entry->scope_level = scope_level;
  strncpy(new_entry->scope_name, scope_name, 63);
  new_entry->scope_name[63] = '\0';

  new_entry->size = 0;
  new_entry->offset = 0;
  new_entry->line_number = line_number;
  new_entry->is_initialized = 0;
  new_entry->initial_value[0] = '\0';

  new_entry->num_parameters = 0;
  new_entry->param_types = NULL;

  new_entry->array_dimensions = 0;
  new_entry->dimension_sizes = NULL;

  // Insert at the beginning of the chain
  new_entry->next = ht->table[index];
  ht->table[index] = new_entry;
  ht->count++;

  return 1;
}

SymbolEntry *lookup_symbol(HashTable *ht, const char *name, int scope_level,
                           const char *scope_name) {
  if (!ht || !name || !scope_name)
    return NULL;

  unsigned int index = hash(name);
  SymbolEntry *current = ht->table[index];

  while (current != NULL) {
    if (strcmp(current->name, name) == 0 &&
        current->scope_level == scope_level &&
        strcmp(current->scope_name, scope_name) == 0) {
      return current;
    }
    current = current->next;
  }

  return NULL;
}

SymbolEntry *lookup_symbol_recursive(HashTable *ht, const char *name,
                                     ScopeManager *sm) {
  if (!ht || !name || !sm)
    return NULL;

  Scope *current_scope = sm->current_scope;

  // Search from current scope upwards to global scope
  while (current_scope != NULL) {
    SymbolEntry *entry = lookup_symbol(ht, name, current_scope->level,
                                       current_scope->scope_name);
    if (entry != NULL) {
      return entry;
    }
    current_scope = current_scope->parent;
  }

  return NULL;
}

int delete_symbol(HashTable *ht, const char *name) {
  if (!ht || !name)
    return 0;

  unsigned int index = hash(name);
  SymbolEntry *current = ht->table[index];
  SymbolEntry *prev = NULL;

  while (current != NULL) {
    if (strcmp(current->name, name) == 0) {
      // Found symbol to delete
      if (prev == NULL) {
        ht->table[index] = current->next;
      } else {
        prev->next = current->next;
      }

      // Free memory
      if (current->name)
        free(current->name);
      if (current->param_types)
        free(current->param_types);
      if (current->dimension_sizes)
        free(current->dimension_sizes);
      free(current);
      ht->count--;
      return 1; // Success
    }
    prev = current;
    current = current->next;
  }

  return 0; // Not found
}

void delete_scope_symbols(HashTable *ht, int scope_level,
                          const char *scope_name) {
  if (!ht || !scope_name)
    return;

  for (int i = 0; i < HASH_TABLE_SIZE; i++) {
    SymbolEntry *current = ht->table[i];
    SymbolEntry *prev = NULL;

    while (current != NULL) {
      if (current->scope_level == scope_level &&
          strcmp(current->scope_name, scope_name) == 0) {
        SymbolEntry *to_delete = current;

        if (prev == NULL) {
          ht->table[i] = current->next;
          current = current->next;
        } else {
          prev->next = current->next;
          current = current->next;
        }

        // Free memory
        free(to_delete->name);
        if (to_delete->param_types)
          free(to_delete->param_types);
        if (to_delete->dimension_sizes)
          free(to_delete->dimension_sizes);
        free(to_delete);
        ht->count--;
      } else {
        prev = current;
        current = current->next;
      }
    }
  }
}

void free_hash_table(HashTable *ht) {
  if (!ht)
    return;

  for (int i = 0; i < HASH_TABLE_SIZE; i++) {
    SymbolEntry *current = ht->table[i];
    while (current != NULL) {
      SymbolEntry *next = current->next;
      free(current->name);
      if (current->param_types)
        free(current->param_types);
      if (current->dimension_sizes)
        free(current->dimension_sizes);
      free(current);
      current = next;
    }
  }

  free(ht);
}

void display_hash_table(HashTable *ht) {
  if (!ht)
    return;

  printf("\n========== SYMBOL TABLE ==========\n");
  printf("Total symbols: %d\n\n", ht->count);
  printf("%-15s %-10s %-12s %-10s %-15s %-6s\n", "Name", "Type", "Kind",
         "Scope Lvl", "Scope Name", "Line");
  printf("---------------------------------------------------------------------"
         "-----------\n");

  for (int i = 0; i < HASH_TABLE_SIZE; i++) {
    SymbolEntry *current = ht->table[i];
    while (current != NULL) {
      printf("%-15s %-10s %-12s %-10d %-15s %-6d\n", current->name,
             data_type_to_string(current->data_type),
             symbol_kind_to_string(current->kind), current->scope_level,
             current->scope_name, current->line_number);

      // Display additional info for arrays
      if (current->kind == SYMBOL_ARRAY && current->array_dimensions > 0) {
        printf("                Array dimensions: ");
        for (int j = 0; j < current->array_dimensions; j++) {
          printf("[%d]", current->dimension_sizes[j]);
        }
        printf("\n");
      }

      // Display additional info for functions
      if (current->kind == SYMBOL_FUNCTION && current->num_parameters > 0) {
        printf("                Parameters: ");
        for (int j = 0; j < current->num_parameters; j++) {
          printf("%s", data_type_to_string(current->param_types[j]));
          if (j < current->num_parameters - 1)
            printf(", ");
        }
        printf("\n");
      }

      current = current->next;
    }
  }
  printf("==================================\n\n");
}

void display_scope_symbols(HashTable *ht, int scope_level,
                           const char *scope_name) {
  if (!ht || !scope_name)
    return;

  printf("\n========== SCOPE: %s (Level %d) ==========\n", scope_name,
         scope_level);
  printf("%-15s %-10s %-12s %-10s %-6s\n", "Name", "Type", "Kind", "Storage",
         "Line");
  printf("----------------------------------------------------------------\n");

  int count = 0;
  for (int i = 0; i < HASH_TABLE_SIZE; i++) {
    SymbolEntry *current = ht->table[i];
    while (current != NULL) {
      if (current->scope_level == scope_level &&
          strcmp(current->scope_name, scope_name) == 0) {
        printf("%-15s %-10s %-12s %-10s %-6d\n", current->name,
               data_type_to_string(current->data_type),
               symbol_kind_to_string(current->kind),
               storage_class_to_string(current->storage_class),
               current->line_number);
        count++;
      }
      current = current->next;
    }
  }

  if (count == 0) {
    printf("(No symbols in this scope)\n");
  }
  printf("==========================================\n\n");
}
