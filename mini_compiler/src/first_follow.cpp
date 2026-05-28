#include "first_follow.h"
#include <iostream>
#include <algorithm>

FirstFollowSets::FirstFollowSets(std::shared_ptr<CFGGrammar> grammar)
    : grammar(grammar), firstComputed(false), followComputed(false) {}

void FirstFollowSets::computeAll() {
    computeFirst();
    computeFollow();
}

void FirstFollowSets::computeFirst() {
    if (!grammar) return;
    
    firstSets.clear();
    firstComputed = false;
    computeFirstSets();
    firstComputed = true;
}

void FirstFollowSets::computeFirstSets() {
    // Initialize First sets for all symbols
    for (const auto& nt : grammar->getNonTerminals()) {
        firstSets[nt.getName()];
    }
    for (const auto& t : grammar->getTerminals()) {
        firstSets[t.getName()].insert(t.getName());
    }
    
    // Compute First sets iteratively until no changes
    bool changed = true;
    while (changed) {
        changed = false;
        
        for (const auto& nt : grammar->getNonTerminals()) {
            const std::string& symbol = nt.getName();
            std::set<std::string> originalFirst = firstSets[symbol];
            
            computeFirstSet(symbol);
            
            if (firstSets[symbol] != originalFirst) {
                changed = true;
            }
        }
    }
}

void FirstFollowSets::computeFirstSet(const std::string& symbol) {
    if (!isNonTerminal(symbol)) {
        // It's a terminal, First(symbol) = {symbol}
        firstSets[symbol].insert(symbol);
        return;
    }
    
    // For each production A -> α
    std::vector<int> ruleIndices = grammar->getProductionsForSymbol(symbol);
    for (int ruleIndex : ruleIndices) {
        const ProductionRule& rule = grammar->getProduction(ruleIndex);
        const std::vector<GrammarSymbol>& rhs = rule.getRHS();
        
        if (rhs.empty()) continue;
        
        // For each symbol in RHS
        bool allHaveEpsilon = true;
        for (const GrammarSymbol& rhsSymbol : rhs) {
            std::string rhsName = rhsSymbol.getName();
            
            if (rhsSymbol.isEpsilon()) {
                firstSets[symbol].insert("ε");
                break;
            }
            
            // Add First(rhsSymbol) - {ε} to First(symbol)
            for (const std::string& firstSymbol : firstSets[rhsName]) {
                if (firstSymbol != "ε") {
                    if (firstSets[symbol].insert(firstSymbol).second) {
                        // Insertion successful
                    }
                }
            }
            
            // If ε is not in First(rhsSymbol), stop
            if (firstSets[rhsName].find("ε") == firstSets[rhsName].end()) {
                allHaveEpsilon = false;
                break;
            }
        }
        
        // If all symbols in RHS have ε in their First sets, add ε to First(symbol)
        if (allHaveEpsilon) {
            firstSets[symbol].insert("ε");
        }
    }
}

std::set<std::string> FirstFollowSets::computeFirstOfString(const std::vector<std::string>& symbols) {
    std::set<std::string> result;
    bool allHaveEpsilon = true;
    
    for (const std::string& symbol : symbols) {
        // Add First(symbol) - {ε} to result
        for (const std::string& firstSymbol : firstSets[symbol]) {
            if (firstSymbol != "ε") {
                result.insert(firstSymbol);
            }
        }
        
        // If ε is not in First(symbol), stop
        if (firstSets[symbol].find("ε") == firstSets[symbol].end()) {
            allHaveEpsilon = false;
            break;
        }
    }
    
    // If all symbols have ε in their First sets, add ε to result
    if (allHaveEpsilon) {
        result.insert("ε");
    }
    
    return result;
}

void FirstFollowSets::computeFollow() {
    if (!grammar || !firstComputed) {
        computeFirst();
    }
    
    followSets.clear();
    followComputed = false;
    computeFollowSets();
    followComputed = true;
}

void FirstFollowSets::computeFollowSets() {
    // Initialize Follow sets for all non-terminals
    for (const auto& nt : grammar->getNonTerminals()) {
        followSets[nt.getName()];
    }
    
    // Add $ to Follow(start symbol)
    std::string startSymbol = grammar->getStartSymbol();
    followSets[startSymbol].insert("$");
    
    // Compute Follow sets iteratively until no changes
    bool changed = true;
    while (changed) {
        changed = false;
        
        for (const auto& nt : grammar->getNonTerminals()) {
            const std::string& symbol = nt.getName();
            std::set<std::string> originalFollow = followSets[symbol];
            
            computeFollowSet(symbol);
            
            if (followSets[symbol] != originalFollow) {
                changed = true;
            }
        }
    }
}

void FirstFollowSets::computeFollowSet(const std::string& symbol) {
    // For each production A -> αBβ
    for (const auto& rule : grammar->getProductions()) {
        const std::vector<GrammarSymbol>& rhs = rule.getRHS();
        
        for (size_t i = 0; i < rhs.size(); ++i) {
            if (rhs[i].getName() == symbol && rhs[i].isNonTerminal()) {
                // Found B in position i
                std::vector<std::string> beta;
                for (size_t j = i + 1; j < rhs.size(); ++j) {
                    beta.push_back(rhs[j].getName());
                }
                
                // Compute First(β)
                std::set<std::string> firstBeta = computeFirstOfString(beta);
                
                // Add First(β) - {ε} to Follow(B)
                for (const std::string& fb : firstBeta) {
                    if (fb != "ε") {
                        if (followSets[symbol].insert(fb).second) {
                            // Insertion successful
                        }
                    }
                }
                
                // If ε is in First(β), add Follow(A) to Follow(B)
                if (firstBeta.find("ε") != firstBeta.end()) {
                    const std::string& a = rule.getLHS().getName();
                    for (const std::string& followA : followSets[a]) {
                        if (followSets[symbol].insert(followA).second) {
                            // Insertion successful
                        }
                    }
                }
            }
        }
    }
}

bool FirstFollowSets::isNonTerminal(const std::string& symbol) const {
    return grammar->isNonTerminal(symbol);
}

bool FirstFollowSets::isTerminal(const std::string& symbol) const {
    return grammar->isTerminal(symbol);
}

const std::set<std::string>& FirstFollowSets::getFirstSet(const std::string& symbol) const {
    static const std::set<std::string> emptySet;
    auto it = firstSets.find(symbol);
    return (it != firstSets.end()) ? it->second : emptySet;
}

const std::set<std::string>& FirstFollowSets::getFollowSet(const std::string& symbol) const {
    static const std::set<std::string> emptySet;
    auto it = followSets.find(symbol);
    return (it != followSets.end()) ? it->second : emptySet;
}

bool FirstFollowSets::isInFirst(const std::string& symbol, const std::string& terminal) const {
    const auto& firstSet = getFirstSet(symbol);
    return firstSet.find(terminal) != firstSet.end();
}

bool FirstFollowSets::isInFollow(const std::string& symbol, const std::string& terminal) const {
    const auto& followSet = getFollowSet(symbol);
    return followSet.find(terminal) != followSet.end();
}

bool FirstFollowSets::hasEpsilonInFirst(const std::string& symbol) const {
    const auto& firstSet = getFirstSet(symbol);
    return firstSet.find("ε") != firstSet.end();
}

std::string FirstFollowSets::firstSetToString(const std::string& symbol) const {
    std::ostringstream oss;
    oss << "First(" << symbol << ") = {";
    
    const auto& firstSet = getFirstSet(symbol);
    bool first = true;
    for (const auto& elem : firstSet) {
        if (!first) oss << ", ";
        oss << elem;
        first = false;
    }
    
    oss << "}";
    return oss.str();
}

std::string FirstFollowSets::followSetToString(const std::string& symbol) const {
    std::ostringstream oss;
    oss << "Follow(" << symbol << ") = {";
    
    const auto& followSet = getFollowSet(symbol);
    bool first = true;
    for (const auto& elem : followSet) {
        if (!first) oss << ", ";
        oss << elem;
        first = false;
    }
    
    oss << "}";
    return oss.str();
}

void FirstFollowSets::printFirstSets() const {
    std::cout << "=== First Sets ===\n";
    for (const auto& pair : firstSets) {
        std::cout << firstSetToString(pair.first) << "\n";
    }
}

void FirstFollowSets::printFollowSets() const {
    std::cout << "=== Follow Sets ===\n";
    for (const auto& pair : followSets) {
        std::cout << followSetToString(pair.first) << "\n";
    }
}

void FirstFollowSets::printAllSets() const {
    printFirstSets();
    std::cout << "\n";
    printFollowSets();
}

bool FirstFollowSets::isLL1() const {
    return getLL1Conflicts().empty();
}

std::vector<std::string> FirstFollowSets::getLL1Conflicts() const {
    std::vector<std::string> conflicts;
    
    // Check for First/First conflicts
    for (const auto& nt : grammar->getNonTerminals()) {
        const std::string& symbol = nt.getName();
        std::vector<int> ruleIndices = grammar->getProductionsForSymbol(symbol);
        
        std::set<std::string> seenTerminals;
        for (int ruleIndex : ruleIndices) {
            const ProductionRule& rule = grammar->getProduction(ruleIndex);
            const std::vector<GrammarSymbol>& rhs = rule.getRHS();
            
            if (rhs.empty()) continue;
            
            std::string firstSymbol = rhs[0].getName();
            if (rhs[0].isEpsilon()) {
                // For epsilon productions, check Follow set
                for (const auto& followSymbol : followSets.at(symbol)) {
                    if (seenTerminals.find(followSymbol) != seenTerminals.end()) {
                        conflicts.push_back("First/Follow conflict for " + symbol + " on terminal " + followSymbol);
                    }
                    seenTerminals.insert(followSymbol);
                }
            } else {
                for (const auto& firstSym : firstSets.at(firstSymbol)) {
                    if (firstSym != "ε" && seenTerminals.find(firstSym) != seenTerminals.end()) {
                        conflicts.push_back("First/First conflict for " + symbol + " on terminal " + firstSym);
                    }
                    if (firstSym != "ε") {
                        seenTerminals.insert(firstSym);
                    }
                }
            }
        }
    }
    
    return conflicts;
}

void FirstFollowSets::recompute() {
    computeAll();
}
