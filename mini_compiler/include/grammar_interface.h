#ifndef GRAMMAR_INTERFACE_H
#define GRAMMAR_INTERFACE_H

#include "cfg_grammar.h"
#include "first_follow.h"
#include "ll1_parser_table.h"
#include "dynamic_ll1_parser.h"
#include <string>
#include <vector>
#include <memory>

class GrammarInterface {
private:
    std::shared_ptr<CFGGrammar> grammar;
    std::shared_ptr<FirstFollowSets> firstFollow;
    std::shared_ptr<LL1ParserTable> parseTable;
    std::shared_ptr<DynamicLL1Parser> parser;
    
    // Current state
    bool grammarLoaded;
    bool tableBuilt;
    std::string currentGrammarString;
    
    // Error tracking
    std::vector<std::string> errors;
    std::vector<std::string> warnings;
    
    // Helper methods
    void initializeComponents();
    void clearComponents();
    
public:
    GrammarInterface();
    ~GrammarInterface() = default;
    
    // Grammar input methods
    bool loadGrammarFromString(const std::string& grammarString);
    bool loadGrammarFromFile(const std::string& filename);
    bool loadGrammarFromInteractive();
    
    // Predefined grammars
    bool loadExpressionGrammar();
    bool loadSimpleArithmeticGrammar();
    bool loadStatementGrammar();
    bool loadCustomGrammar(const std::string& grammarString);
    
    // Grammar analysis
    bool analyzeGrammar();
    bool buildParsingTable();
    bool validateGrammar();
    
    // Parsing methods
    bool parseInput(const std::string& input);
    bool parseInteractive();
    
    // Display methods
    void printGrammar() const;
    void printFirstFollowSets() const;
    void printParsingTable() const;
    void printParseTree() const;
    void printParsingSteps() const;
    void printErrors() const;
    void printWarnings() const;
    void printFullReport() const;
    
    // Export methods
    bool exportParseTreeDOT(const std::string& filename);
    bool exportParseTreeJSON(const std::string& filename);
    bool exportParsingTableCSV(const std::string& filename);
    
    // Query methods
    bool isGrammarLoaded() const { return grammarLoaded; }
    bool isTableBuilt() const { return tableBuilt; }
    bool isLL1() const;
    const std::vector<std::string>& getErrors() const { return errors; }
    const std::vector<std::string>& getWarnings() const { return warnings; }
    bool hasErrors() const { return !errors.empty(); }
    bool hasWarnings() const { return !warnings.empty(); }
    
    // Component access
    std::shared_ptr<CFGGrammar> getGrammar() { return grammar; }
    std::shared_ptr<FirstFollowSets> getFirstFollow() { return firstFollow; }
    std::shared_ptr<LL1ParserTable> getParseTable() { return parseTable; }
    std::shared_ptr<DynamicLL1Parser> getParser() { return parser; }
    
    // Utility methods
    void reset();
    std::string getStatus() const;
    std::string getHelp() const;
    
    // Example grammars
    static std::string getExpressionGrammarExample();
    static std::string getArithmeticGrammarExample();
    static std::string getStatementGrammarExample();
};

#endif // GRAMMAR_INTERFACE_H
