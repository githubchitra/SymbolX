#include <stdio.h>
#include "../include/data_types.h"

const char* data_type_to_string(DataType type) {
    switch(type) {
        case TYPE_INT:      return "int";
        case TYPE_FLOAT:    return "float";
        case TYPE_CHAR:     return "char";
        case TYPE_DOUBLE:   return "double";
        case TYPE_VOID:     return "void";
        case TYPE_BOOL:     return "bool";
        case TYPE_STRING:   return "string";
        case TYPE_ARRAY:    return "array";
        case TYPE_POINTER:  return "pointer";
        case TYPE_STRUCT:   return "struct";
        case TYPE_FUNCTION: return "function";
        default:            return "unknown";
    }
}

const char* symbol_kind_to_string(SymbolKind kind) {
    switch(kind) {
        case SYMBOL_VARIABLE:   return "variable";
        case SYMBOL_FUNCTION:   return "function";
        case SYMBOL_PARAMETER:  return "parameter";
        case SYMBOL_CONSTANT:   return "constant";
        case SYMBOL_ARRAY:      return "array";
        case SYMBOL_STRUCT:     return "struct";
        case SYMBOL_TYPEDEF:    return "typedef";
        default:                return "unknown";
    }
}

const char* storage_class_to_string(StorageClass sc) {
    switch(sc) {
        case STORAGE_AUTO:      return "auto";
        case STORAGE_STATIC:    return "static";
        case STORAGE_EXTERN:    return "extern";
        case STORAGE_REGISTER:  return "register";
        default:                return "none";
    }
}
