#ifndef DATA_TYPES_H
#define DATA_TYPES_H

// Data types supported by the compiler
typedef enum {
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_CHAR,
    TYPE_DOUBLE,
    TYPE_VOID,
    TYPE_BOOL,
    TYPE_STRING,
    TYPE_ARRAY,
    TYPE_POINTER,
    TYPE_STRUCT,
    TYPE_FUNCTION,
    TYPE_UNKNOWN
} DataType;

// Symbol kinds
typedef enum {
    SYMBOL_VARIABLE,
    SYMBOL_FUNCTION,
    SYMBOL_PARAMETER,
    SYMBOL_CONSTANT,
    SYMBOL_ARRAY,
    SYMBOL_STRUCT,
    SYMBOL_TYPEDEF
} SymbolKind;

// Storage classes
typedef enum {
    STORAGE_AUTO,
    STORAGE_STATIC,
    STORAGE_EXTERN,
    STORAGE_REGISTER
} StorageClass;

// Function to convert data type to string
const char* data_type_to_string(DataType type);

// Function to convert symbol kind to string
const char* symbol_kind_to_string(SymbolKind kind);

// Function to convert storage class to string
const char* storage_class_to_string(StorageClass sc);

#endif
