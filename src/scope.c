#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/scope.h"

ScopeManager* create_scope_manager() {
    ScopeManager* manager = (ScopeManager*)malloc(sizeof(ScopeManager));
    if (!manager) {
        fprintf(stderr, "Error: Memory allocation failed for scope manager\n");
        return NULL;
    }
    
    // Create global scope
    Scope* global = (Scope*)malloc(sizeof(Scope));
    if (!global) {
        fprintf(stderr, "Error: Memory allocation failed for global scope\n");
        free(manager);
        return NULL;
    }
    
    global->level = 0;
    strcpy(global->scope_name, "global");
    global->parent = NULL;
    global->next = NULL;
    global->children = NULL;
    
    manager->global_scope = global;
    manager->current_scope = global;
    manager->scope_counter = 0;
    
    return manager;
}

void enter_scope(ScopeManager* manager, const char* scope_name) {
    if (!manager) return;
    
    Scope* new_scope = (Scope*)malloc(sizeof(Scope));
    if (!new_scope) {
        fprintf(stderr, "Error: Memory allocation failed for new scope\n");
        return;
    }
    
    new_scope->level = manager->current_scope->level + 1;
    
    // Generate unique scope name if not provided
    if (scope_name && strlen(scope_name) > 0) {
        strncpy(new_scope->scope_name, scope_name, 63);
        new_scope->scope_name[63] = '\0';
    } else {
        sprintf(new_scope->scope_name, "block_%d", manager->scope_counter++);
    }
    
    new_scope->parent = manager->current_scope;
    new_scope->next = NULL;
    new_scope->children = NULL;
    
    // Add to parent's children list
    if (manager->current_scope->children == NULL) {
        manager->current_scope->children = new_scope;
    } else {
        Scope* sibling = manager->current_scope->children;
        while (sibling->next != NULL) {
            sibling = sibling->next;
        }
        sibling->next = new_scope;
    }
    
    manager->current_scope = new_scope;
}

void exit_scope(ScopeManager* manager) {
    if (!manager || !manager->current_scope) return;
    
    if (manager->current_scope->parent == NULL) {
        fprintf(stderr, "Warning: Cannot exit global scope\n");
        return;
    }
    
    manager->current_scope = manager->current_scope->parent;
}

int get_current_scope_level(ScopeManager* manager) {
    if (!manager || !manager->current_scope) return -1;
    return manager->current_scope->level;
}

const char* get_current_scope_name(ScopeManager* manager) {
    if (!manager || !manager->current_scope) return NULL;
    return manager->current_scope->scope_name;
}

void free_scope_recursive(Scope* scope) {
    if (!scope) return;
    
    // Free all children
    Scope* child = scope->children;
    while (child) {
        Scope* next = child->next;
        free_scope_recursive(child);
        child = next;
    }
    
    free(scope);
}

void free_scope_manager(ScopeManager* manager) {
    if (!manager) return;
    
    free_scope_recursive(manager->global_scope);
    free(manager);
}

void print_scope_hierarchy(Scope* scope, int indent) {
    if (!scope) return;
    
    for (int i = 0; i < indent; i++) {
        printf("  ");
    }
    printf("├─ %s (level %d)\n", scope->scope_name, scope->level);
    
    // Print all children
    Scope* child = scope->children;
    while (child) {
        print_scope_hierarchy(child, indent + 1);
        child = child->next;
    }
}
