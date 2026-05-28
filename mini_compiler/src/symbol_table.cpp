#include "symbol_table.h"
#include <sstream>
#include <algorithm>

// SymbolInfo implementation
SymbolInfo::SymbolInfo(const std::string& name, SymbolKind kind, DataType type, 
                       int scopeLevel, int line, int column)
    : name(name), kind(kind), dataType(type), scopeLevel(scopeLevel), 
      line(line), column(column), isInitialized(false), isUsed(false), 
      returnType(DataType::UNKNOWN) {}

std::string SymbolInfo::toString() const {
    std::ostringstream oss;
    oss << name << " [" << symbolKindToString(kind) << ", " << dataTypeToString(dataType);
    oss << ", scope=" << scopeLevel << ", L" << line << ":C" << column;
    
    if (isFunction()) {
        oss << ", return=" << dataTypeToString(returnType);
        if (!parameterTypes.empty()) {
            oss << ", params=(";
            for (size_t i = 0; i < parameterTypes.size(); ++i) {
                if (i > 0) oss << ", ";
                oss << dataTypeToString(parameterTypes[i]);
            }
            oss << ")";
        }
    }
    
    oss << "]";
    if (isInitialized) oss << " [initialized]";
    if (isUsed) oss << " [used]";
    
    return oss.str();
}

void SymbolInfo::print() const {
    std::cout << toString() << std::endl;
}

std::string SymbolInfo::symbolKindToString(SymbolKind kind) {
    switch (kind) {
        case SymbolKind::VARIABLE: return "VAR";
        case SymbolKind::FUNCTION: return "FUNC";
        case SymbolKind::PARAMETER: return "PARAM";
        case SymbolKind::TYPE: return "TYPE";
        case SymbolKind::UNKNOWN: return "UNKNOWN";
        default: return "UNKNOWN";
    }
}

std::string SymbolInfo::dataTypeToString(DataType type) {
    switch (type) {
        case DataType::VOID: return "void";
        case DataType::INT: return "int";
        case DataType::FLOAT: return "float";
        case DataType::CHAR: return "char";
        case DataType::STRING: return "string";
        case DataType::UNKNOWN: return "unknown";
        default: return "unknown";
    }
}

// Scope implementation
Scope::Scope(int level, const std::string& name, int startLine)
    : scopeLevel(level), scopeName(name), startLine(startLine), endLine(-1) {}

bool Scope::addSymbol(std::shared_ptr<SymbolInfo> symbol) {
    if (symbols.find(symbol->getName()) != symbols.end()) {
        return false; // Symbol already exists in this scope
    }
    symbols[symbol->getName()] = symbol;
    return true;
}

std::shared_ptr<SymbolInfo> Scope::lookup(const std::string& name) {
    auto it = symbols.find(name);
    return (it != symbols.end()) ? it->second : nullptr;
}

bool Scope::contains(const std::string& name) const {
    return symbols.find(name) != symbols.end();
}

void Scope::print() const {
    std::cout << "Scope '" << scopeName << "' (level " << scopeLevel 
              << ", lines " << startLine << "-" << (endLine == -1 ? "?" : std::to_string(endLine))
              << "):\n";
    std::cout << "  Symbols:\n";
    for (const auto& pair : symbols) {
        std::cout << "    " << pair.second->toString() << std::endl;
    }
    std::cout << "  Total: " << symbols.size() << " symbols\n";
}

// SymbolTable implementation
SymbolTable::SymbolTable() : currentScopeLevel(0) {
    globalScope = std::make_shared<Scope>(0, "global", 1);
    scopes.push_back(globalScope);
    scopeStack.push(globalScope);
}

void SymbolTable::enterScope(const std::string& scopeName, int line) {
    currentScopeLevel++;
    auto newScope = std::make_shared<Scope>(currentScopeLevel, scopeName, line);
    scopes.push_back(newScope);
    scopeStack.push(newScope);
}

void SymbolTable::exitScope(int line) {
    if (!scopeStack.empty()) {
        auto currentScope = scopeStack.top();
        currentScope->setEndLine(line);
        scopeStack.pop();
        currentScopeLevel--;
    }
}

std::shared_ptr<Scope> SymbolTable::getCurrentScope() {
    return scopeStack.empty() ? nullptr : scopeStack.top();
}

bool SymbolTable::addSymbol(const std::string& name, SymbolKind kind, DataType dataType, 
                           int line, int column) {
    auto symbol = std::make_shared<SymbolInfo>(name, kind, dataType, currentScopeLevel, line, column);
    return addSymbol(symbol);
}

bool SymbolTable::addSymbol(std::shared_ptr<SymbolInfo> symbol) {
    auto currentScope = getCurrentScope();
    if (!currentScope) {
        return false;
    }
    
    // Check if symbol already exists in current scope
    if (currentScope->contains(symbol->getName())) {
        addError("Symbol '" + symbol->getName() + "' already declared in current scope");
        return false;
    }
    
    return currentScope->addSymbol(symbol);
}

std::shared_ptr<SymbolInfo> SymbolTable::lookup(const std::string& name) {
    return lookupInAllScopes(name);
}

std::shared_ptr<SymbolInfo> SymbolTable::lookupInCurrentScope(const std::string& name) {
    auto currentScope = getCurrentScope();
    return currentScope ? currentScope->lookup(name) : nullptr;
}

std::shared_ptr<SymbolInfo> SymbolTable::lookupInAllScopes(const std::string& name) {
    // Search from current scope outward
    std::stack<std::shared_ptr<Scope>> tempStack = scopeStack;
    
    while (!tempStack.empty()) {
        auto scope = tempStack.top();
        auto symbol = scope->lookup(name);
        if (symbol) {
            return symbol;
        }
        tempStack.pop();
    }
    
    return nullptr;
}

bool SymbolTable::isDeclared(const std::string& name) {
    return lookupInAllScopes(name) != nullptr;
}

bool SymbolTable::isDeclaredInCurrentScope(const std::string& name) {
    return lookupInCurrentScope(name) != nullptr;
}

bool SymbolTable::markAsUsed(const std::string& name) {
    auto symbol = lookupInAllScopes(name);
    if (symbol) {
        symbol->setUsed(true);
        return true;
    }
    return false;
}

bool SymbolTable::markAsInitialized(const std::string& name) {
    auto symbol = lookupInAllScopes(name);
    if (symbol) {
        symbol->setInitialized(true);
        return true;
    }
    return false;
}

bool SymbolTable::addFunction(const std::string& name, DataType returnType, int line, int column) {
    auto symbol = std::make_shared<SymbolInfo>(name, SymbolKind::FUNCTION, returnType, 
                                               currentScopeLevel, line, column);
    symbol->setReturnType(returnType);
    return addSymbol(symbol);
}

bool SymbolTable::addParameter(const std::string& name, DataType dataType, int line, int column) {
    auto symbol = std::make_shared<SymbolInfo>(name, SymbolKind::PARAMETER, dataType, 
                                               currentScopeLevel, line, column);
    return addSymbol(symbol);
}

void SymbolTable::addError(const std::string& error) {
    errors.push_back(error);
}

void SymbolTable::addWarning(const std::string& warning) {
    warnings.push_back(warning);
}

void SymbolTable::clearErrors() {
    errors.clear();
    warnings.clear();
}

void SymbolTable::printCurrentScope() const {
    auto currentScope = getCurrentScope();
    if (currentScope) {
        currentScope->print();
    } else {
        std::cout << "No current scope\n";
    }
}

void SymbolTable::printAllScopes() const {
    std::cout << "=== Symbol Table ===\n";
    for (const auto& scope : scopes) {
        scope->print();
        std::cout << std::endl;
    }
}

void SymbolTable::printSymbolTable() const {
    printAllScopes();
    
    std::cout << "=== Errors ===\n";
    if (errors.empty()) {
        std::cout << "No errors\n";
    } else {
        for (const auto& error : errors) {
            std::cout << "Error: " << error << std::endl;
        }
    }
    
    std::cout << "\n=== Warnings ===\n";
    if (warnings.empty()) {
        std::cout << "No warnings\n";
    } else {
        for (const auto& warning : warnings) {
            std::cout << "Warning: " << warning << std::endl;
        }
    }
}

int SymbolTable::getTotalSymbolCount() const {
    int total = 0;
    for (const auto& scope : scopes) {
        total += scope->getSymbolCount();
    }
    return total;
}

bool SymbolTable::validateSymbolUsage() {
    bool hasIssues = false;
    
    // Check for unused symbols
    auto unusedSymbols = getUnusedSymbols();
    for (const auto& symbolName : unusedSymbols) {
        addWarning("Unused symbol: " + symbolName);
        hasIssues = true;
    }
    
    // Check for uninitialized variables
    auto uninitializedSymbols = getUninitializedSymbols();
    for (const auto& symbolName : uninitializedSymbols) {
        addError("Uninitialized variable: " + symbolName);
        hasIssues = true;
    }
    
    return hasIssues;
}

std::vector<std::string> SymbolTable::getUnusedSymbols() {
    std::vector<std::string> unused;
    
    for (const auto& scope : scopes) {
        for (const auto& pair : scope->getSymbols()) {
            const auto& symbol = pair.second;
            if (!symbol->isSymbolUsed() && symbol->getKind() != SymbolKind::PARAMETER) {
                unused.push_back(symbol->getName());
            }
        }
    }
    
    return unused;
}

std::vector<std::string> SymbolTable::getUninitializedSymbols() {
    std::vector<std::string> uninitialized;
    
    for (const auto& scope : scopes) {
        for (const auto& pair : scope->getSymbols()) {
            const auto& symbol = pair.second;
            if (symbol->isVariable() && !symbol->isInitializedSymbol() && symbol->isSymbolUsed()) {
                uninitialized.push_back(symbol->getName());
            }
        }
    }
    
    return uninitialized;
}
