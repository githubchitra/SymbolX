#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <stack>
#include <iostream>
#include "ast_node.h"

enum class SymbolKind {
    VARIABLE,
    FUNCTION,
    PARAMETER,
    TYPE,
    UNKNOWN
};

class SymbolInfo {
private:
    std::string name;
    SymbolKind kind;
    DataType dataType;
    int scopeLevel;
    int line;
    int column;
    bool isInitialized;
    bool isUsed;
    std::vector<DataType> parameterTypes;  // For functions
    DataType returnType;                   // For functions

public:
    SymbolInfo(const std::string& name, SymbolKind kind, DataType type, 
               int scopeLevel, int line, int column);
    
    // Getters
    const std::string& getName() const { return name; }
    SymbolKind getKind() const { return kind; }
    DataType getDataType() const { return dataType; }
    int getScopeLevel() const { return scopeLevel; }
    int getLine() const { return line; }
    int getColumn() const { return column; }
    bool isInitializedSymbol() const { return isInitialized; }
    bool isSymbolUsed() const { return isUsed; }
    const std::vector<DataType>& getParameterTypes() const { return parameterTypes; }
    DataType getReturnType() const { return returnType; }
    
    // Setters
    void setInitialized(bool initialized) { isInitialized = initialized; }
    void setUsed(bool used) { isUsed = used; }
    void addParameterType(DataType type) { parameterTypes.push_back(type); }
    void setReturnType(DataType type) { returnType = type; }
    
    // Utility methods
    bool isFunction() const { return kind == SymbolKind::FUNCTION; }
    bool isVariable() const { return kind == SymbolKind::VARIABLE; }
    bool isParameter() const { return kind == SymbolKind::PARAMETER; }
    
    std::string toString() const;
    void print() const;
    
    // Static utility methods
    static std::string symbolKindToString(SymbolKind kind);
    static std::string dataTypeToString(DataType type);
};

class Scope {
private:
    std::unordered_map<std::string, std::shared_ptr<SymbolInfo>> symbols;
    int scopeLevel;
    std::string scopeName;
    int startLine;
    int endLine;

public:
    Scope(int level, const std::string& name, int startLine);
    
    // Symbol management
    bool addSymbol(std::shared_ptr<SymbolInfo> symbol);
    std::shared_ptr<SymbolInfo> lookup(const std::string& name);
    bool contains(const std::string& name) const;
    
    // Getters
    int getScopeLevel() const { return scopeLevel; }
    const std::string& getScopeName() const { return scopeName; }
    int getStartLine() const { return startLine; }
    int getEndLine() const { return endLine; }
    void setEndLine(int line) { endLine = line; }
    
    // Utility
    const std::unordered_map<std::string, std::shared_ptr<SymbolInfo>>& getSymbols() const { return symbols; }
    void print() const;
    int getSymbolCount() const { return symbols.size(); }
};

class SymbolTable {
private:
    std::vector<std::shared_ptr<Scope>> scopes;
    std::stack<std::shared_ptr<Scope>> scopeStack;
    int currentScopeLevel;
    std::shared_ptr<Scope> globalScope;
    
    // Error tracking
    std::vector<std::string> errors;
    std::vector<std::string> warnings;

public:
    SymbolTable();
    ~SymbolTable() = default;
    
    // Scope management
    void enterScope(const std::string& scopeName, int line);
    void exitScope(int line);
    std::shared_ptr<Scope> getCurrentScope();
    std::shared_ptr<Scope> getGlobalScope() { return globalScope; }
    
    // Symbol management
    bool addSymbol(const std::string& name, SymbolKind kind, DataType dataType, 
                   int line, int column);
    bool addSymbol(std::shared_ptr<SymbolInfo> symbol);
    std::shared_ptr<SymbolInfo> lookup(const std::string& name);
    std::shared_ptr<SymbolInfo> lookupInCurrentScope(const std::string& name);
    std::shared_ptr<SymbolInfo> lookupInAllScopes(const std::string& name);
    
    // Semantic analysis helpers
    bool isDeclared(const std::string& name);
    bool isDeclaredInCurrentScope(const std::string& name);
    bool markAsUsed(const std::string& name);
    bool markAsInitialized(const std::string& name);
    
    // Function specific
    bool addFunction(const std::string& name, DataType returnType, int line, int column);
    bool addParameter(const std::string& name, DataType dataType, int line, int column);
    
    // Error handling
    void addError(const std::string& error);
    void addWarning(const std::string& warning);
    const std::vector<std::string>& getErrors() const { return errors; }
    const std::vector<std::string>& getWarnings() const { return warnings; }
    bool hasErrors() const { return !errors.empty(); }
    bool hasWarnings() const { return !warnings.empty(); }
    void clearErrors();
    
    // Utility and debugging
    void printCurrentScope() const;
    void printAllScopes() const;
    void printSymbolTable() const;
    int getTotalSymbolCount() const;
    int getCurrentScopeLevel() const { return currentScopeLevel; }
    
    // Validation
    bool validateSymbolUsage();
    std::vector<std::string> getUnusedSymbols();
    std::vector<std::string> getUninitializedSymbols();
};

#endif // SYMBOL_TABLE_H
