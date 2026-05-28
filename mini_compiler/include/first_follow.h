#ifndef FIRST_FOLLOW_H
#define FIRST_FOLLOW_H

#include "cfg_grammar.h"
#include <set>
#include <map>
#include <vector>
#include <string>

class FirstFollowSets {
private:
    std::shared_ptr<CFGGrammar> grammar;
    
    // First and Follow sets
    std::map<std::string, std::set<std::string>> firstSets;
    std::map<std::string, std::set<std::string>> followSets;
    
    // Computed flag
    bool firstComputed;
    bool followComputed;
    
    // Computation methods
    void computeFirstSets();
    void computeFirstSet(const std::string& symbol);
    std::set<std::string> computeFirstOfString(const std::vector<std::string>& symbols);
    void computeFollowSets();
    void computeFollowSet(const std::string& symbol);
    
    // Helper methods
    bool isNonTerminal(const std::string& symbol) const;
    bool isTerminal(const std::string& symbol) const;
    std::vector<std::string> getRHSOfProductions(const std::string& nonTerminal) const;

public:
    explicit FirstFollowSets(std::shared_ptr<CFGGrammar> grammar);
    
    // Computation methods
    void computeAll();
    void computeFirst();
    void computeFollow();
    
    // Getters
    const std::set<std::string>& getFirstSet(const std::string& symbol) const;
    const std::set<std::string>& getFollowSet(const std::string& symbol) const;
    const std::map<std::string, std::set<std::string>>& getAllFirstSets() const { return firstSets; }
    const std::map<std::string, std::set<std::string>>& getAllFollowSets() const { return followSets; }
    
    // Query methods
    bool isInFirst(const std::string& symbol, const std::string& terminal) const;
    bool isInFollow(const std::string& symbol, const std::string& terminal) const;
    bool hasEpsilonInFirst(const std::string& symbol) const;
    
    // Utility methods
    std::string firstSetToString(const std::string& symbol) const;
    std::string followSetToString(const std::string& symbol) const;
    void printFirstSets() const;
    void printFollowSets() const;
    void printAllSets() const;
    
    // Validation
    bool isLL1() const;
    std::vector<std::string> getLL1Conflicts() const;
    
    // Reset
    void recompute();
};

#endif // FIRST_FOLLOW_H
