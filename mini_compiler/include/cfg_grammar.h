#ifndef CFG_GRAMMAR_H
#define CFG_GRAMMAR_H

#include <string>
#include <vector>
#include <set>
#include <map>
#include <memory>
#include <iostream>

// Symbol types
enum class SymbolType {
    NON_TERMINAL,
    TERMINAL,
    EPSILON,
    END_MARKER  // $
};

class GrammarSymbol {
private:
    std::string name;
    SymbolType type;
    
public:
    GrammarSymbol(const std::string& name, SymbolType type);
    
    // Getters
    const std::string& getName() const { return name; }
    SymbolType getType() const { return type; }
    
    // Comparison operators
    bool operator==(const GrammarSymbol& other) const;
    bool operator<(const GrammarSymbol& other) const;
    
    // Utility methods
    bool isNonTerminal() const { return type == SymbolType::NON_TERMINAL; }
    bool isTerminal() const { return type == SymbolType::TERMINAL; }
    bool isEpsilon() const { return type == SymbolType::EPSILON; }
    bool isEndMarker() const { return type == SymbolType::END_MARKER; }
    
    // Display
    std::string toString() const;
    void print() const;
};

// Production rule: A -> α
class ProductionRule {
private:
    GrammarSymbol leftHandSide;
    std::vector<GrammarSymbol> rightHandSide;
    int ruleNumber;
    
public:
    ProductionRule(const GrammarSymbol& lhs, const std::vector<GrammarSymbol>& rhs, int ruleNumber);
    
    // Getters
    const GrammarSymbol& getLHS() const { return leftHandSide; }
    const std::vector<GrammarSymbol>& getRHS() const { return rightHandSide; }
    int getRuleNumber() const { return ruleNumber; }
    
    // Setters
    void setRuleNumber(int num) { ruleNumber = num; }
    
    // Utility methods
    bool hasEpsilon() const;
    bool isEpsilonProduction() const;
    int getLength() const { return static_cast<int>(rightHandSide.size()); }
    
    // Display
    std::string toString() const;
    void print() const;
};

// CFG Grammar
class CFGGrammar {
private:
    std::string startSymbol;
    std::set<GrammarSymbol> nonTerminals;
    std::set<GrammarSymbol> terminals;
    std::vector<ProductionRule> productions;
    std::map<std::string, std::vector<int>> productionMap; // Maps non-terminal to rule indices
    
    // Error tracking
    std::vector<std::string> errors;
    std::vector<std::string> warnings;
    
    // Validation
    bool validateGrammar();
    bool checkLeftRecursion();
    bool checkAmbiguity();
    
public:
    CFGGrammar();
    explicit CFGGrammar(const std::string& startSymbol);
    
    // Grammar construction
    void setStartSymbol(const std::string& symbol);
    void addNonTerminal(const std::string& symbol);
    void addTerminal(const std::string& symbol);
    void addProduction(const ProductionRule& rule);
    void addProduction(const std::string& lhs, const std::vector<std::string>& rhs);
    
    // Grammar parsing from string
    bool parseGrammarFromString(const std::string& grammarString);
    bool parseGrammarRule(const std::string& ruleString);
    
    // Getters
    const std::string& getStartSymbol() const { return startSymbol; }
    const std::set<GrammarSymbol>& getNonTerminals() const { return nonTerminals; }
    const std::set<GrammarSymbol>& getTerminals() const { return terminals; }
    const std::vector<ProductionRule>& getProductions() const { return productions; }
    
    // Query methods
    bool isNonTerminal(const std::string& symbol) const;
    bool isTerminal(const std::string& symbol) const;
    std::vector<int> getProductionsForSymbol(const std::string& symbol) const;
    const ProductionRule& getProduction(int index) const;
    
    // Error handling
    const std::vector<std::string>& getErrors() const { return errors; }
    const std::vector<std::string>& getWarnings() const { return warnings; }
    bool hasErrors() const { return !errors.empty(); }
    bool hasWarnings() const { return !warnings.empty(); }
    void clearErrors();
    
    // Display
    void printGrammar() const;
    void printProductions() const;
    std::string toString() const;
    
    // Utility
    void clear();
    bool isValid() const;
};

#endif // CFG_GRAMMAR_H
