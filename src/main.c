#include <stdio.h>
#include <stdlib.h>
#include "../include/symbol_table.h"

void demonstrate_basic_usage();
void demonstrate_scoping();
void demonstrate_functions();
void demonstrate_arrays();
void simulate_compiler_parsing();

int main() {
    printf("╔═══════════════════════════════════════════════════════╗\n");
    printf("║   SYMBOL TABLE - COMPILER DESIGN PROJECT             ║\n");
    printf("║   Comprehensive Implementation with Scope Management  ║\n");
    printf("╚═══════════════════════════════════════════════════════╝\n\n");
    
    int choice = -1;
    
    do {
        printf("\n========== MENU ==========\n");
        printf("1. Basic Usage Demo\n");
        printf("2. Scoping Demo\n");
        printf("3. Functions Demo\n");
        printf("4. Arrays Demo\n");
        printf("5. Simulate Compiler Parsing\n");
        printf("0. Exit\n");
        printf("==========================\n");
        printf("Enter your choice: ");
        
        if (scanf("%d", &choice) != 1) {
            printf("Invalid input!\n");
            while(getchar() != '\n'); // Clear input buffer
            continue;
        }
        
        switch(choice) {
            case 1:
                demonstrate_basic_usage();
                break;
            case 2:
                demonstrate_scoping();
                break;
            case 3:
                demonstrate_functions();
                break;
            case 4:
                demonstrate_arrays();
                break;
            case 5:
                simulate_compiler_parsing();
                break;
            case 0:
                printf("\nExiting program. Thank you!\n");
                break;
            default:
                printf("\nInvalid choice! Please try again.\n");
        }
    } while(choice != 0);
    
    return 0;
}

void demonstrate_basic_usage() {
    printf("\n========== BASIC USAGE DEMONSTRATION ==========\n\n");
    
    SymbolTable* st = create_symbol_table();
    if (!st) {
        printf("Failed to create symbol table!\n");
        return;
    }
    
    printf("Creating symbol table...\n");
    printf("Inserting global variables...\n\n");
    
    // Insert some symbols
    symbol_table_insert(st, "x", TYPE_INT, SYMBOL_VARIABLE, 1);
    symbol_table_insert(st, "y", TYPE_FLOAT, SYMBOL_VARIABLE, 2);
    symbol_table_insert(st, "PI", TYPE_DOUBLE, SYMBOL_CONSTANT, 3);
    
    // Mark PI as initialized
    symbol_table_mark_initialized(st, "PI");
    
    // Display the symbol table
    symbol_table_display(st);
    
    // Lookup a symbol
    printf("Looking up symbol 'x':\n");
    SymbolEntry* entry = symbol_table_lookup(st, "x");
    if (entry) {
        printf("  Found: %s (%s) at line %d\n",
               entry->name,
               data_type_to_string(entry->data_type),
               entry->line_number);
    }
    
    // Try to insert duplicate
    printf("\nTrying to insert duplicate 'x':\n");
    symbol_table_insert(st, "x", TYPE_INT, SYMBOL_VARIABLE, 5);
    
    printf("\nError count: %d\n", symbol_table_get_errors(st));
    
    free_symbol_table(st);
    printf("\n================================================\n");
}

void demonstrate_scoping() {
    printf("\n========== SCOPING DEMONSTRATION ==========\n\n");
    
    SymbolTable* st = create_symbol_table();
    if (!st) return;
    
    printf("Code simulation:\n");
    printf("----------------\n");
    printf("int x = 10;        // Global scope\n");
    printf("int y = 20;\n\n");
    printf("void func() {\n");
    printf("    int x = 5;     // Local scope (shadows global x)\n");
    printf("    int z = 15;\n");
    printf("    {\n");
    printf("        int x = 1; // Nested scope\n");
    printf("        int w = 25;\n");
    printf("    }\n");
    printf("}\n");
    printf("----------------\n\n");
    
    // Global scope
    symbol_table_insert(st, "x", TYPE_INT, SYMBOL_VARIABLE, 1);
    symbol_table_insert(st, "y", TYPE_INT, SYMBOL_VARIABLE, 2);
    
    // Enter function scope
    symbol_table_enter_scope(st, "func");
    symbol_table_insert(st, "x", TYPE_INT, SYMBOL_VARIABLE, 4);
    symbol_table_insert(st, "z", TYPE_INT, SYMBOL_VARIABLE, 5);
    
    printf("After entering func() scope:\n");
    symbol_table_display_current_scope(st);
    
    // Enter nested block
    symbol_table_enter_scope(st, "block_1");
    symbol_table_insert(st, "x", TYPE_INT, SYMBOL_VARIABLE, 7);
    symbol_table_insert(st, "w", TYPE_INT, SYMBOL_VARIABLE, 8);
    
    printf("After entering nested block:\n");
    symbol_table_display_current_scope(st);
    
    // Lookup x (should find the innermost one)
    printf("Looking up 'x' from nested block:\n");
    SymbolEntry* entry = symbol_table_lookup(st, "x");
    if (entry) {
        printf("  Found: %s in scope '%s' (level %d) at line %d\n",
               entry->name, entry->scope_name, entry->scope_level, entry->line_number);
    }
    
    // Exit nested block
    symbol_table_exit_scope(st);
    printf("\nAfter exiting nested block (symbols deleted):\n");
    symbol_table_display(st);
    
    // Display scope hierarchy
    symbol_table_display_all_scopes(st);
    
    symbol_table_exit_scope(st);
    free_symbol_table(st);
    printf("===========================================\n");
}

void demonstrate_functions() {
    printf("\n========== FUNCTIONS DEMONSTRATION ==========\n\n");
    
    SymbolTable* st = create_symbol_table();
    if (!st) return;
    
    printf("Code simulation:\n");
    printf("----------------\n");
    printf("int add(int a, int b);\n");
    printf("float compute(float x, float y, int flag);\n");
    printf("void print();\n");
    printf("----------------\n\n");
    
    // Insert function declarations
    symbol_table_insert(st, "add", TYPE_INT, SYMBOL_FUNCTION, 1);
    symbol_table_insert(st, "compute", TYPE_FLOAT, SYMBOL_FUNCTION, 2);
    symbol_table_insert(st, "print", TYPE_VOID, SYMBOL_FUNCTION, 3);
    
    // Set parameters for add function
    DataType add_params[] = {TYPE_INT, TYPE_INT};
    symbol_table_set_function_params(st, "add", 2, add_params);
    
    // Set parameters for compute function
    DataType compute_params[] = {TYPE_FLOAT, TYPE_FLOAT, TYPE_INT};
    symbol_table_set_function_params(st, "compute", 3, compute_params);
    
    // Set parameters for print function (no parameters)
    symbol_table_set_function_params(st, "print", 0, NULL);
    
    // Display the symbol table
    symbol_table_display(st);
    
    // Demonstrate function scope with parameters
    printf("Entering 'add' function scope:\n");
    symbol_table_enter_scope(st, "add");
    symbol_table_insert(st, "a", TYPE_INT, SYMBOL_PARAMETER, 1);
    symbol_table_insert(st, "b", TYPE_INT, SYMBOL_PARAMETER, 1);
    symbol_table_insert(st, "result", TYPE_INT, SYMBOL_VARIABLE, 2);
    
    symbol_table_display_current_scope(st);
    
    symbol_table_exit_scope(st);
    free_symbol_table(st);
    printf("=============================================\n");
}

void demonstrate_arrays() {
    printf("\n========== ARRAYS DEMONSTRATION ==========\n\n");
    
    SymbolTable* st = create_symbol_table();
    if (!st) return;
    
    printf("Code simulation:\n");
    printf("----------------\n");
    printf("int arr[10];\n");
    printf("float matrix[3][4];\n");
    printf("char str[100];\n");
    printf("----------------\n\n");
    
    // Insert array declarations
    symbol_table_insert(st, "arr", TYPE_INT, SYMBOL_ARRAY, 1);
    symbol_table_insert(st, "matrix", TYPE_FLOAT, SYMBOL_ARRAY, 2);
    symbol_table_insert(st, "str", TYPE_CHAR, SYMBOL_ARRAY, 3);
    
    // Set array dimensions
    int arr_dims[] = {10};
    symbol_table_set_array_dims(st, "arr", 1, arr_dims);
    
    int matrix_dims[] = {3, 4};
    symbol_table_set_array_dims(st, "matrix", 2, matrix_dims);
    
    int str_dims[] = {100};
    symbol_table_set_array_dims(st, "str", 1, str_dims);
    
    // Display the symbol table
    symbol_table_display(st);
    
    // Lookup and display array info
    printf("Array 'matrix' details:\n");
    SymbolEntry* entry = symbol_table_lookup(st, "matrix");
    if (entry) {
        printf("  Name: %s\n", entry->name);
        printf("  Type: %s\n", data_type_to_string(entry->data_type));
        printf("  Kind: %s\n", symbol_kind_to_string(entry->kind));
        printf("  Dimensions: %d\n", entry->array_dimensions);
        printf("  Size: ");
        for (int i = 0; i < entry->array_dimensions; i++) {
            printf("[%d]", entry->dimension_sizes[i]);
        }
        printf("\n  Total elements: %d\n", entry->size);
    }
    
    free_symbol_table(st);
    printf("\n==========================================\n");
}

void simulate_compiler_parsing() {
    printf("\n========== SIMULATING COMPILER PARSING ==========\n\n");
    
    SymbolTable* st = create_symbol_table();
    if (!st) return;
    
    printf("Parsing sample C code:\n");
    printf("----------------------\n");
    printf("#include <stdio.h>\n\n");
    printf("int global_var = 100;\n");
    printf("float pi = 3.14159;\n\n");
    printf("int factorial(int n) {\n");
    printf("    if (n <= 1) return 1;\n");
    printf("    int result = n * factorial(n - 1);\n");
    printf("    return result;\n");
    printf("}\n\n");
    printf("int main() {\n");
    printf("    int num = 5;\n");
    printf("    int fact;\n");
    printf("    fact = factorial(num);\n");
    printf("    return 0;\n");
    printf("}\n");
    printf("----------------------\n\n");
    
    // Simulate parsing
    printf("PARSING PHASE:\n\n");
    
    // Global declarations
    printf("[Line 3] Declaring global variable 'global_var'\n");
    symbol_table_insert(st, "global_var", TYPE_INT, SYMBOL_VARIABLE, 3);
    symbol_table_mark_initialized(st, "global_var");
    
    printf("[Line 4] Declaring global variable 'pi'\n");
    symbol_table_insert(st, "pi", TYPE_FLOAT, SYMBOL_VARIABLE, 4);
    symbol_table_mark_initialized(st, "pi");
    
    // Function factorial
    printf("[Line 6] Declaring function 'factorial'\n");
    symbol_table_insert(st, "factorial", TYPE_INT, SYMBOL_FUNCTION, 6);
    DataType fact_params[] = {TYPE_INT};
    symbol_table_set_function_params(st, "factorial", 1, fact_params);
    
    printf("[Line 6] Entering scope 'factorial'\n");
    symbol_table_enter_scope(st, "factorial");
    symbol_table_insert(st, "n", TYPE_INT, SYMBOL_PARAMETER, 6);
    
    printf("[Line 8] Declaring variable 'result'\n");
    symbol_table_insert(st, "result", TYPE_INT, SYMBOL_VARIABLE, 8);
    
    printf("[Line 10] Exiting scope 'factorial'\n");
    symbol_table_exit_scope(st);
    
    // Function main
    printf("[Line 12] Declaring function 'main'\n");
    symbol_table_insert(st, "main", TYPE_INT, SYMBOL_FUNCTION, 12);
    symbol_table_set_function_params(st, "main", 0, NULL);
    
    printf("[Line 12] Entering scope 'main'\n");
    symbol_table_enter_scope(st, "main");
    
    printf("[Line 13] Declaring variable 'num'\n");
    symbol_table_insert(st, "num", TYPE_INT, SYMBOL_VARIABLE, 13);
    symbol_table_mark_initialized(st, "num");
    
    printf("[Line 14] Declaring variable 'fact'\n");
    symbol_table_insert(st, "fact", TYPE_INT, SYMBOL_VARIABLE, 14);
    
    printf("[Line 15] Using 'factorial' - checking if declared...\n");
    if (symbol_table_is_declared(st, "factorial")) {
        printf("         ✓ 'factorial' is declared\n");
    }
    
    printf("[Line 15] Using 'num' - checking if declared...\n");
    if (symbol_table_is_declared(st, "num")) {
        printf("         ✓ 'num' is declared\n");
    }
    
    printf("[Line 17] Exiting scope 'main'\n");
    symbol_table_exit_scope(st);
    
    printf("\n\nFINAL SYMBOL TABLE:\n");
    symbol_table_display(st);
    
    printf("SCOPE HIERARCHY:\n");
    symbol_table_display_all_scopes(st);
    
    printf("\nCompilation summary:\n");
    printf("  Total errors: %d\n", symbol_table_get_errors(st));
    printf("  Status: %s\n", 
           symbol_table_get_errors(st) == 0 ? "SUCCESS" : "FAILED");
    
    free_symbol_table(st);
    printf("\n=================================================\n");
}
