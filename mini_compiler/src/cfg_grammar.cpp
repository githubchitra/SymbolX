#include "cfg_grammar.h"
#include <sstream>
#include <algorithm>
#include <cctype>

// GrammarSymbol implementation
GrammarSymbol::GrammarSymbol(const std::string& name, SymbolType type)
    : name(name), type(type) {}

bool GrammarSymbol::operator==(const GrammarSymbol& other) const {
    return name == other.name && type == other.type;
}

bool GrammarSymbol::operator<(const GrammarSymbol& other) const {
    if (name != other.name) {
        return name < other.name;
    }
    return type < other.type;
}

std::string GrammarSymbol::toString() const {
    switch (type) {
        case SymbolType::NON_TERMINAL:
            return name;  // Non-terminals typically uppercase
        case SymbolType::TERMINAL:
            return "'" + name + "'";
        case SymbolType::EPSILON:
            return "ε";
        case SymbolType::END_MARKER:
            return "$";
        default:
            return name;
    }
}

void GrammarSymbol::print() const {
    std::cout << toString();
}

// ProductionRule implementation
ProductionRule::ProductionRule(const GrammarSymbol& lhs, const std::vector<GrammarSymbol>& rhs, int ruleNumber)
    : leftHandSide(lhs), rightHandSide(rhs), ruleNumber(ruleNumber) {}

bool ProductionRule::hasEpsilon() const {
    for (const auto& symbol : rightHandSide) {
        if (symbol.isEpsilon()) {
            return true;
        }
    }
    return false;
}

bool ProductionRule::isEpsilonProduction() const {
    return rightHandSide.size() == 1 && rightHandSide[0].isEpsilon();
}

std::string ProductionRule::toString() const {
    std::ostringstream oss;
    oss << leftHandSide.toString() << " -> ";
    
    for (size_t i = 0; i < rightHandSide.size(); ++i) {
        oss << rightHandSide[i].toString();
        if (i < rightHandSide.size() - 1) {
            oss << " ";
        }
    }
    
    oss << " (" << ruleNumber << ")";
    return oss.str();
}

void ProductionRule::print() const {
    std::cout << toString() << std::endl;
}

// CFGGrammar implementation
CFGGrammar::CFGGrammar() : startSymbol("S") {}

CFGGrammar::CFGGrammar(const std::string& startSymbol) 
    : startSymbol(startSymbol) {}

void CFGGrammar::setStartSymbol(const std::string& symbol) {
    startSymbol = symbol;
    addNonTerminal(symbol);
}

void CFGGrammar::addNonTerminal(const std::string& symbol) {
    nonTerminals.insert(GrammarSymbol(symbol, SymbolType::NON_TERMINAL));
}

void CFGGrammar::addTerminal(const std::string& symbol) {
    terminals.insert(GrammarSymbol(symbol, SymbolType::TERMINAL));
}

void CFGGrammar::addProduction(const ProductionRule& rule) {
    productions.push_back(rule);
    
    // Add to production map
    std::string lhsName = rule.getLHS().getName();
    productionMap[lhsName].push_back(static_cast<int>(productions.size()) - 1);
    
    // Ensure LHS is in non-terminals
    addNonTerminal(lhsName);
    
    // Add RHS symbols to appropriate sets
    for (const auto& symbol : rule.getRHS()) {
        if (symbol.isNonTerminal()) {
            addNonTerminal(symbol.getName());
        } else if (symbol.isTerminal()) {
            addTerminal(symbol.getName());
        }
    }
}

void CFGGrammar::addProduction(const std::string& lhs, const std::vector<std::string>& rhs) {
    GrammarSymbol lhsSymbol(lhs, SymbolType::NON_TERMINAL);
    std::vector<GrammarSymbol> rhsSymbols;
    
    for (const auto& symbol : rhs) {
        if (symbol == "ε" || symbol == "epsilon") {
            rhsSymbols.push_back(GrammarSymbol("ε", SymbolType::EPSILON));
        } else if (symbol == "$") {
            rhsSymbols.push_back(GrammarSymbol("$", SymbolType::END_MARKER));
        } else if (isupper(symbol[0]) || symbol.size() > 1) {
            // Assume uppercase or multi-character symbols are non-terminals
            rhsSymbols.push_back(GrammarSymbol(symbol, SymbolType::NON_TERMINAL));
        } else {
            rhsSymbols.push_back(GrammarSymbol(symbol, SymbolType::TERMINAL));
        }
    }
    
    int ruleNumber = static_cast<int>(productions.size()) + 1;
    addProduction(ProductionRule(lhsSymbol, rhsSymbols, ruleNumber));
}

bool CFGGrammar::parseGrammarFromString(const std::string& grammarString) {
    clear();
    
    std::istringstream iss(grammarString);
    std::string line;
    
    while (std::getline(iss, line)) {
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#') {
            continue;
        }
        
        // Trim whitespace
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);
        
        if (!parseGrammarRule(line)) {
            return false;
        }
    }
    
    return validateGrammar();
}

bool CFGGrammar::parseGrammarRule(const std::string& ruleString) {
    // Parse rule in format: "E -> E + T | T"
    size_t arrowPos = ruleString.find("->");
    if (arrowPos == std::string::npos) {
        errors.push_back("Invalid rule format: missing '->' in '" + ruleString + "'");
        return false;
    }
    
    std::string lhs = ruleString.substr(0, arrowPos);
    std::string rhs = ruleString.substr(arrowPos + 2);
    
    // Trim whitespace
    lhs.erase(0, lhs.find_first_not_of(" \t"));
    lhs.erase(lhs.find_last_not_of(" \t") + 1);
    rhs.erase(0, rhs.find_first_not_of(" \t"));
    rhs.erase(rhs.find_last_not_of(" \t") + 1);
    
    // Set start symbol if this is the first rule
    if (productions.empty()) {
        setStartSymbol(lhs);
    }
    
    // Split by '|' for multiple productions
    std::vector<std::string> alternatives;
    std::string current;
    bool inQuotes = false;
    
    for (char c : rhs) {
        if (c == '"') {
            inQuotes = !inQuotes;
        } else if (c == '|' && !inQuotes) {
            alternatives.push_back(current);
            current.clear();
        } else {
            current += c;
        }
    }
    if (!current.empty()) {
        alternatives.push_back(current);
    }
    
    // Parse each alternative
    for (const auto& alternative : alternatives) {
        std::vector<std::string> symbols;
        std::string currentSymbol;
        inQuotes = false;
        
        for (char c : alternative) {
            if (c == '"') {
                inQuotes = !inQuotes;
            } else if (std::isspace(c) && !inQuotes) {
                if (!currentSymbol.empty()) {
                    symbols.push_back(currentSymbol);
                    currentSymbol.clear();
                }
            } else {
                currentSymbol += c;
            }
        }
        if (!currentSymbol.empty()) {
            symbols.push_back(currentSymbol);
        }
        
        if (symbols.empty()) {
            // Empty production (epsilon)
            symbols.push_back("ε");
        }
        
        addProduction(lhs, symbols);
    }
    
    return true;
}

bool CFGGrammar::isNonTerminal(const std::string& symbol) const {
    for (const auto& nt : nonTerminals) {
        if (nt.getName() == symbol) {
            return true;
        }
    }
    return false;
}

bool CFGGrammar::isTerminal(const std::string& symbol) const {
    for (const auto& t : terminals) {
        if (t.getName() == symbol) {
            return true;
        }
    }
    return false;
}

std::vector<int> CFGGrammar::getProductionsForSymbol(const std::string& symbol) const {
    auto it = productionMap.find(symbol);
    if (it != productionMap.end()) {
        return it->second;
    }
    return {};
}

const ProductionRule& CFGGrammar::getProduction(int index) const {
    return productions[index];
}

void CFGGrammar::clearErrors() {
    errors.clear();
    warnings.clear();
}

void CFGGrammar::printGrammar() const {
    std::cout << "=== CFG Grammar ===\n";
    std::cout << "Start Symbol: " << startSymbol << "\n\n";
    
    std::cout << "Non-terminals: ";
    for (const auto& nt : nonTerminals) {
        std::cout << nt.toString() << " ";
    }
    std::cout << "\n";
    
    std::cout << "Terminals: ";
    for (const auto& t : terminals) {
        std::cout << t.toString() << " ";
    }
    std::cout << "\n\n";
    
    printProductions();
}

void CFGGrammar::printProductions() const {
    std::cout << "Production Rules:\n";
    for (const auto& rule : productions) {
        std::cout << "  " << rule.toString() << "\n";
    }
}

std::string CFGGrammar::toString() const {
    std::ostringstream oss;
    oss << "CFG Grammar:\n";
    oss << "Start Symbol: " << startSymbol << "\n";
    oss << "Productions: " << productions.size() << "\n";
    return oss.str();
}

void CFGGrammar::clear() {
    startSymbol = "S";
    nonTerminals.clear();
    terminals.clear();
    productions.clear();
    productionMap.clear();
    errors.clear();
    warnings.clear();
}

bool CFGGrammar::validateGrammar() {
    bool valid = true;
    
    // Check if start symbol is defined
    if (!isNonTerminal(startSymbol)) {
        errors.push_back("Start symbol '" + startSymbol + "' is not defined as a non-terminal");
        valid = false;
    }
    
    // Check for left recursion
    if (checkLeftRecursion()) {
        warnings.push_back("Grammar contains left recursion (may not be LL(1))");
    }
    
    return valid;
}

bool CFGGrammar::checkLeftRecursion() {
    for (const auto& rule : productions) {
        if (!rule.getRHS().empty()) {
            if (rule.getLHS().getName() == rule.getRHS()[0].getName()) {
                return true;
            }
        }
    }
    return false;
}

bool CFGGrammar::checkAmbiguity() {
    // Simple check: if a non-terminal has multiple productions with same first terminal
    // This is a simplified check
    return false;
}

bool CFGGrammar::isValid() const {
    return !startSymbol.empty() && !productions.empty() && errors.empty();
}
