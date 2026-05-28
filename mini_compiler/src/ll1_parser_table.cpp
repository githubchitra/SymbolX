#include "ll1_parser_table.h"
#include <iostream>
#include <iomanip>
#include <sstream>

LL1ParserTable::LL1ParserTable(std::shared_ptr<CFGGrammar> grammar, std::shared_ptr<FirstFollowSets> firstFollow)
    : grammar(grammar), firstFollow(firstFollow), tableBuilt(false), isLL1(false) {}

void LL1ParserTable::build() {
    if (!grammar || !firstFollow) {
        errors.push_back("Grammar or First/Follow sets not provided");
        return;
    }
    
    // Ensure First/Follow sets are computed
    firstFollow->computeAll();
    
    clear();
    buildTable();
    tableBuilt = true;
    
    // Check if grammar is LL(1)
    isLL1 = conflicts.empty();
}

void LL1ParserTable::rebuild() {
    clear();
    build();
}

void LL1ParserTable::buildTable() {
    // Initialize non-terminals and terminals
    for (const auto& nt : grammar->getNonTerminals()) {
        nonTerminals.insert(nt.getName());
    }
    for (const auto& t : grammar->getTerminals()) {
        terminals.insert(t.getName());
    }
    terminals.insert("$"); // End marker
    
    // Build table entries
    for (const auto& nt : grammar->getNonTerminals()) {
        const std::string& A = nt.getName();
        std::vector<int> ruleIndices = grammar->getProductionsForSymbol(A);
        
        for (int ruleIndex : ruleIndices) {
            const ProductionRule& rule = grammar->getProduction(ruleIndex);
            const std::vector<GrammarSymbol>& rhs = rule.getRHS();
            
            if (rhs.empty()) continue;
            
            // For each terminal a in First(α)
            std::vector<std::string> alpha;
            for (const auto& symbol : rhs) {
                alpha.push_back(symbol.getName());
            }
            
            const auto& firstAlpha = firstFollow->getFirstSet(rhs[0].getName());
            
            for (const std::string& a : firstAlpha) {
                if (a != "ε") {
                    fillTableEntry(A, a, ruleIndex);
                }
            }
            
            // If ε is in First(α), for each terminal b in Follow(A)
            if (firstAlpha.find("ε") != firstAlpha.end()) {
                const auto& followA = firstFollow->getFollowSet(A);
                for (const std::string& b : followA) {
                    fillTableEntry(A, b, ruleIndex);
                }
            }
        }
    }
}

void LL1ParserTable::fillTableEntry(const std::string& nonTerminal, const std::string& terminal, int ruleIndex) {
    // Check for conflict
    if (table[nonTerminal].find(terminal) != table[nonTerminal].end()) {
        int existingRule = table[nonTerminal][terminal];
        if (existingRule != ruleIndex) {
            std::ostringstream oss;
            oss << "Conflict at [" << nonTerminal << ", " << terminal << "]: "
                << "Rule " << existingRule << " vs Rule " << ruleIndex;
            conflicts.push_back(oss.str());
        }
    } else {
        table[nonTerminal][terminal] = ruleIndex;
    }
}

int LL1ParserTable::getEntry(const std::string& nonTerminal, const std::string& terminal) const {
    auto ntIt = table.find(nonTerminal);
    if (ntIt != table.end()) {
        auto termIt = ntIt->second.find(terminal);
        if (termIt != ntIt->second.end()) {
            return termIt->second;
        }
    }
    return -1; // No entry
}

bool LL1ParserTable::hasEntry(const std::string& nonTerminal, const std::string& terminal) const {
    return getEntry(nonTerminal, terminal) != -1;
}

const std::map<std::string, int>& LL1ParserTable::getRow(const std::string& nonTerminal) const {
    static const std::map<std::string, int> emptyRow;
    auto it = table.find(nonTerminal);
    return (it != table.end()) ? it->second : emptyRow;
}

void LL1ParserTable::printTable() const {
    if (!tableBuilt) {
        std::cout << "Table not built yet.\n";
        return;
    }
    
    std::cout << "=== LL(1) Parsing Table ===\n";
    std::cout << "Grammar is " << (isLL1 ? "LL(1)" : "NOT LL(1)") << "\n\n";
    
    if (!conflicts.empty()) {
        std::cout << "Conflicts:\n";
        for (const auto& conflict : conflicts) {
            std::cout << "  " << conflict << "\n";
        }
        std::cout << "\n";
    }
    
    // Print table header
    std::cout << std::setw(15) << "NT\\T";
    for (const auto& terminal : terminals) {
        std::cout << std::setw(10) << terminal;
    }
    std::cout << "\n";
    
    // Print separator
    std::cout << std::string(15 + terminals.size() * 10, '-') << "\n";
    
    // Print rows
    for (const auto& nonTerminal : nonTerminals) {
        std::cout << std::setw(15) << nonTerminal;
        
        for (const auto& terminal : terminals) {
            int entry = getEntry(nonTerminal, terminal);
            if (entry != -1) {
                std::cout << std::setw(10) << entry;
            } else {
                std::cout << std::setw(10) << "-";
            }
        }
        std::cout << "\n";
    }
}

void LL1ParserTable::printTableAsCSV() const {
    if (!tableBuilt) {
        std::cout << "Table not built yet.\n";
        return;
    }
    
    // Print header
    std::cout << "NT";
    for (const auto& terminal : terminals) {
        std::cout << "," << terminal;
    }
    std::cout << "\n";
    
    // Print rows
    for (const auto& nonTerminal : nonTerminals) {
        std::cout << nonTerminal;
        for (const auto& terminal : terminals) {
            int entry = getEntry(nonTerminal, terminal);
            std::cout << ",";
            if (entry != -1) {
                std::cout << entry;
            }
        }
        std::cout << "\n";
    }
}

std::string LL1ParserTable::tableToString() const {
    std::ostringstream oss;
    
    oss << "LL(1) Parsing Table:\n";
    oss << "Grammar is " << (isLL1 ? "LL(1)" : "NOT LL(1)") << "\n";
    oss << "Non-terminals: " << nonTerminals.size() << "\n";
    oss << "Terminals: " << terminals.size() << "\n";
    oss << "Entries: " << getEntryCount() << "\n";
    
    return oss.str();
}

bool LL1ParserTable::validate() const {
    return tableBuilt && isLL1;
}

std::string LL1ParserTable::getValidationReport() const {
    std::ostringstream oss;
    
    oss << "LL(1) Validation Report:\n";
    oss << "========================\n";
    oss << "Table Built: " << (tableBuilt ? "Yes" : "No") << "\n";
    oss << "Is LL(1): " << (isLL1 ? "Yes" : "No") << "\n";
    oss << "Conflicts: " << conflicts.size() << "\n";
    oss << "Errors: " << errors.size() << "\n";
    
    if (!conflicts.empty()) {
        oss << "\nConflicts:\n";
        for (const auto& conflict : conflicts) {
            oss << "  " << conflict << "\n";
        }
    }
    
    if (!errors.empty()) {
        oss << "\nErrors:\n";
        for (const auto& error : errors) {
            oss << "  " << error << "\n";
        }
    }
    
    return oss.str();
}

void LL1ParserTable::clear() {
    table.clear();
    nonTerminals.clear();
    terminals.clear();
    conflicts.clear();
    errors.clear();
    tableBuilt = false;
    isLL1 = false;
}

int LL1ParserTable::getTableSize() const {
    return static_cast<int>(nonTerminals.size() * terminals.size());
}

int LL1ParserTable::getEntryCount() const {
    int count = 0;
    for (const auto& pair : table) {
        count += static_cast<int>(pair.second.size());
    }
    return count;
}
