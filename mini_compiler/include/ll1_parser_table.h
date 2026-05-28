#ifndef LL1_PARSER_TABLE_H
#define LL1_PARSER_TABLE_H

#include "cfg_grammar.h"
#include "first_follow.h"
#include <map>
#include <set>
#include <string>
#include <vector>
#include <memory>

class LL1ParserTable {
private:
    std::shared_ptr<CFGGrammar> grammar;
    std::shared_ptr<FirstFollowSets> firstFollow;
    
    // Parsing table: M[A, a] = production rule index
    std::map<std::string, std::map<std::string, int>> table;
    
    // Table metadata
    std::set<std::string> nonTerminals;
    std::set<std::string> terminals;
    bool tableBuilt;
    bool isLL1;
    
    // Error tracking
    std::vector<std::string> conflicts;
    std::vector<std::string> errors;
    
    // Table construction
    void buildTable();
    void fillTableEntry(const std::string& nonTerminal, const std::string& terminal, int ruleIndex);
    
public:
    LL1ParserTable(std::shared_ptr<CFGGrammar> grammar, std::shared_ptr<FirstFollowSets> firstFollow);
    
    // Table construction
    void build();
    void rebuild();
    
    // Table access
    int getEntry(const std::string& nonTerminal, const std::string& terminal) const;
    bool hasEntry(const std::string& nonTerminal, const std::string& terminal) const;
    const std::map<std::string, int>& getRow(const std::string& nonTerminal) const;
    
    // Query methods
    bool isGrammarLL1() const { return isLL1; }
    const std::vector<std::string>& getConflicts() const { return conflicts; }
    const std::vector<std::string>& getErrors() const { return errors; }
    bool hasConflicts() const { return !conflicts.empty(); }
    bool hasErrors() const { return !errors.empty(); }
    
    // Getters
    const std::set<std::string>& getNonTerminals() const { return nonTerminals; }
    const std::set<std::string>& getTerminals() const { return terminals; }
    bool isTableBuilt() const { return tableBuilt; }
    
    // Display
    void printTable() const;
    void printTableAsCSV() const;
    std::string tableToString() const;
    
    // Validation
    bool validate() const;
    std::string getValidationReport() const;
    
    // Utility
    void clear();
    int getTableSize() const;
    int getEntryCount() const;
};

#endif // LL1_PARSER_TABLE_H
