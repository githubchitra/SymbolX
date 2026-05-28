#ifndef DYNAMIC_LL1_PARSER_H
#define DYNAMIC_LL1_PARSER_H

#include "cfg_grammar.h"
#include "first_follow.h"
#include "ll1_parser_table.h"
#include "ast_node.h"
#include <stack>
#include <vector>
#include <string>
#include <memory>

// Parse tree node for dynamic parsing
class ParseTreeNode {
private:
    std::string symbol;
    bool isNonTerminal;
    std::vector<std::shared_ptr<ParseTreeNode>> children;
    int ruleNumber;  // If this node was created by a production rule
    
public:
    ParseTreeNode(const std::string& symbol, bool isNonTerminal, int ruleNumber = -1);
    
    // Getters
    const std::string& getSymbol() const { return symbol; }
    bool isNonTerminalNode() const { return isNonTerminal; }
    const std::vector<std::shared_ptr<ParseTreeNode>>& getChildren() const { return children; }
    int getRuleNumber() const { return ruleNumber; }
    
    // Tree manipulation
    void addChild(std::shared_ptr<ParseTreeNode> child);
    std::shared_ptr<ParseTreeNode> getChild(int index) const;
    int getChildCount() const { return static_cast<int>(children.size()); }
    
    // Display
    void printTree(int depth = 0) const;
    std::string toString(int depth = 0) const;
    
    // Convert to AST node
    std::shared_ptr<ASTNode> toASTNode() const;
};

class DynamicLL1Parser {
private:
    std::shared_ptr<CFGGrammar> grammar;
    std::shared_ptr<FirstFollowSets> firstFollow;
    std::shared_ptr<LL1ParserTable> parseTable;
    
    // Parsing state
    std::stack<std::string> stack;
    std::vector<std::string> inputTokens;
    int inputPosition;
    std::shared_ptr<ParseTreeNode> parseTree;
    
    // Error tracking
    std::vector<std::string> errors;
    std::vector<std::string> warnings;
    bool parsingSuccessful;
    
    // Parsing steps for debugging
    std::vector<std::string> parsingSteps;
    
    // Helper methods
    void initializeParsing(const std::string& input);
    bool parseStep();
    std::vector<std::string> tokenizeInput(const std::string& input);
    void recordParsingStep(const std::string& step);
    void buildParseTree(std::shared_ptr<ParseTreeNode> node, const std::string& symbol, int ruleIndex);
    
public:
    DynamicLL1Parser(std::shared_ptr<CFGGrammar> grammar, 
                    std::shared_ptr<FirstFollowSets> firstFollow,
                    std::shared_ptr<LL1ParserTable> parseTable);
    
    // Main parsing method
    bool parse(const std::string& input);
    
    // Result access
    std::shared_ptr<ParseTreeNode> getParseTree() const { return parseTree; }
    std::shared_ptr<ASTNode> getAST() const;
    bool isSuccessful() const { return parsingSuccessful; }
    
    // Error handling
    const std::vector<std::string>& getErrors() const { return errors; }
    const std::vector<std::string>& getWarnings() const { return warnings; }
    bool hasErrors() const { return !errors.empty(); }
    bool hasWarnings() const { return !warnings.empty(); }
    
    // Debugging
    const std::vector<std::string>& getParsingSteps() const { return parsingSteps; }
    void printParsingSteps() const;
    void printParseTree() const;
    
    // Utility methods
    void reset();
    std::string getParsingReport() const;
    
    // Validation
    bool validateGrammar() const;
    std::string getGrammarInfo() const;
};

#endif // DYNAMIC_LL1_PARSER_H
