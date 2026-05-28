#include "dynamic_ll1_parser.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <stack>

// ParseTreeNode implementation
ParseTreeNode::ParseTreeNode(const std::string& symbol, bool isNonTerminal, int ruleNumber)
    : symbol(symbol), isNonTerminal(isNonTerminal), ruleNumber(ruleNumber) {}

void ParseTreeNode::addChild(std::shared_ptr<ParseTreeNode> child) {
    if (child) {
        children.push_back(child);
    }
}

std::shared_ptr<ParseTreeNode> ParseTreeNode::getChild(int index) const {
    if (index >= 0 && index < static_cast<int>(children.size())) {
        return children[index];
    }
    return nullptr;
}

void ParseTreeNode::printTree(int depth) const {
    std::cout << toString(depth);
}

std::string ParseTreeNode::toString(int depth) const {
    std::ostringstream oss;
    std::string indent(depth * 2, ' ');
    
    oss << indent << symbol;
    if (ruleNumber != -1) {
        oss << " [rule " << ruleNumber << "]";
    }
    oss << "\n";
    
    for (const auto& child : children) {
        oss << child->toString(depth + 1);
    }
    
    return oss.str();
}

std::shared_ptr<ASTNode> ParseTreeNode::toASTNode() const {
    auto astNode = std::make_shared<ASTNode>(NodeType::UNKNOWN, symbol, 0, 0);
    
    for (const auto& child : children) {
        astNode->addChild(child->toASTNode());
    }
    
    return astNode;
}

// DynamicLL1Parser implementation
DynamicLL1Parser::DynamicLL1Parser(std::shared_ptr<CFGGrammar> grammar,
                                   std::shared_ptr<FirstFollowSets> firstFollow,
                                   std::shared_ptr<LL1ParserTable> parseTable)
    : grammar(grammar), firstFollow(firstFollow), parseTable(parseTable),
      inputPosition(0), parsingSuccessful(false) {}

bool DynamicLL1Parser::parse(const std::string& input) {
    reset();
    
    if (!grammar || !firstFollow || !parseTable) {
        errors.push_back("Parser components not properly initialized");
        return false;
    }
    
    if (!parseTable->isGrammarLL1()) {
        errors.push_back("Grammar is not LL(1), cannot parse");
        return false;
    }
    
    initializeParsing(input);
    
    // Main parsing loop
    while (!stack.empty() && inputPosition < static_cast<int>(inputTokens.size())) {
        if (!parseStep()) {
            parsingSuccessful = false;
            return false;
        }
    }
    
    // Check if parsing was successful
    parsingSuccessful = stack.empty() && inputPosition == static_cast<int>(inputTokens.size());
    
    if (!parsingSuccessful) {
        if (!stack.empty()) {
            errors.push_back("Parsing failed: stack not empty at end");
        }
        if (inputPosition < static_cast<int>(inputTokens.size())) {
            errors.push_back("Parsing failed: input not fully consumed");
        }
    }
    
    return parsingSuccessful;
}

void DynamicLL1Parser::initializeParsing(const std::string& input) {
    // Tokenize input
    inputTokens = tokenizeInput(input);
    inputPosition = 0;
    
    // Initialize stack with start symbol and end marker
    stack.push("$");
    stack.push(grammar->getStartSymbol());
    
    // Initialize parse tree root
    parseTree = std::make_shared<ParseTreeNode>(grammar->getStartSymbol(), true);
    
    recordParsingStep("Initialized parsing with input: " + input);
}

bool DynamicLL1Parser::parseStep() {
    if (stack.empty()) {
        errors.push_back("Stack empty during parsing");
        return false;
    }
    
    std::string stackTop = stack.top();
    std::string currentInput = (inputPosition < static_cast<int>(inputTokens.size())) 
                              ? inputTokens[inputPosition] : "$";
    
    recordParsingStep("Stack top: " + stackTop + ", Input: " + currentInput);
    
    // Case 1: Stack top is terminal
    if (grammar->isTerminal(stackTop) || stackTop == "$") {
        if (stackTop == currentInput) {
            stack.pop();
            inputPosition++;
            recordParsingStep("Matched terminal: " + stackTop);
            return true;
        } else {
            std::ostringstream oss;
            oss << "Mismatch: expected '" << stackTop << "', got '" << currentInput << "'";
            errors.push_back(oss.str());
            return false;
        }
    }
    
    // Case 2: Stack top is non-terminal
    if (grammar->isNonTerminal(stackTop)) {
        int ruleIndex = parseTable->getEntry(stackTop, currentInput);
        
        if (ruleIndex == -1) {
            std::ostringstream oss;
            oss << "No entry in parsing table for [" << stackTop << ", " << currentInput << "]";
            errors.push_back(oss.str());
            return false;
        }
        
        const ProductionRule& rule = grammar->getProduction(ruleIndex);
        stack.pop();
        
        // Push RHS symbols onto stack in reverse order
        const auto& rhs = rule.getRHS();
        for (int i = static_cast<int>(rhs.size()) - 1; i >= 0; --i) {
            if (!rhs[i].isEpsilon()) {
                stack.push(rhs[i].getName());
            }
        }
        
        recordParsingStep("Applied rule " + std::to_string(ruleIndex) + ": " + rule.toString());
        
        // Build parse tree
        buildParseTree(parseTree, stackTop, ruleIndex);
        
        return true;
    }
    
    errors.push_back("Unknown symbol on stack: " + stackTop);
    return false;
}

std::vector<std::string> DynamicLL1Parser::tokenizeInput(const std::string& input) {
    std::vector<std::string> tokens;
    std::string currentToken;
    
    for (char c : input) {
        if (std::isspace(c)) {
            if (!currentToken.empty()) {
                tokens.push_back(currentToken);
                currentToken.clear();
            }
        } else {
            currentToken += c;
        }
    }
    
    if (!currentToken.empty()) {
        tokens.push_back(currentToken);
    }
    
    // Add end marker
    tokens.push_back("$");
    
    return tokens;
}

void DynamicLL1Parser::recordParsingStep(const std::string& step) {
    parsingSteps.push_back(step);
}

void DynamicLL1Parser::buildParseTree(std::shared_ptr<ParseTreeNode> node, const std::string& symbol, int ruleIndex) {
    if (!node) return;
    
    // Find the node with the matching symbol
    if (node->getSymbol() == symbol && node->getChildCount() == 0) {
        // This is the node to expand
        const ProductionRule& rule = grammar->getProduction(ruleIndex);
        const auto& rhs = rule.getRHS();
        
        for (const auto& rhsSymbol : rhs) {
            if (!rhsSymbol.isEpsilon()) {
                bool isNT = grammar->isNonTerminal(rhsSymbol.getName());
                auto child = std::make_shared<ParseTreeNode>(rhsSymbol.getName(), isNT, ruleIndex);
                node->addChild(child);
            }
        }
    } else {
        // Search in children
        for (auto& child : node->getChildren()) {
            buildParseTree(child, symbol, ruleIndex);
        }
    }
}

std::shared_ptr<ASTNode> DynamicLL1Parser::getAST() const {
    if (parseTree) {
        return parseTree->toASTNode();
    }
    return nullptr;
}

void DynamicLL1Parser::printParsingSteps() const {
    std::cout << "=== Parsing Steps ===\n";
    for (size_t i = 0; i < parsingSteps.size(); ++i) {
        std::cout << (i + 1) << ". " << parsingSteps[i] << "\n";
    }
}

void DynamicLL1Parser::printParseTree() const {
    if (parseTree) {
        std::cout << "=== Parse Tree ===\n";
        parseTree->printTree();
    } else {
        std::cout << "No parse tree available\n";
    }
}

void DynamicLL1Parser::reset() {
    while (!stack.empty()) {
        stack.pop();
    }
    inputTokens.clear();
    inputPosition = 0;
    parseTree = nullptr;
    errors.clear();
    warnings.clear();
    parsingSteps.clear();
    parsingSuccessful = false;
}

std::string DynamicLL1Parser::getParsingReport() const {
    std::ostringstream oss;
    
    oss << "=== Parsing Report ===\n";
    oss << "Success: " << (parsingSuccessful ? "Yes" : "No") << "\n";
    oss << "Errors: " << errors.size() << "\n";
    oss << "Warnings: " << warnings.size() << "\n";
    oss << "Parsing Steps: " << parsingSteps.size() << "\n";
    
    if (!errors.empty()) {
        oss << "\nErrors:\n";
        for (const auto& error : errors) {
            oss << "  " << error << "\n";
        }
    }
    
    if (!warnings.empty()) {
        oss << "\nWarnings:\n";
        for (const auto& warning : warnings) {
            oss << "  " << warning << "\n";
        }
    }
    
    return oss.str();
}

bool DynamicLL1Parser::validateGrammar() const {
    return grammar && grammar->isValid() && parseTable && parseTable->isGrammarLL1();
}

std::string DynamicLL1Parser::getGrammarInfo() const {
    std::ostringstream oss;
    
    oss << "Grammar Information:\n";
    if (grammar) {
        oss << "  Start Symbol: " << grammar->getStartSymbol() << "\n";
        oss << "  Non-terminals: " << grammar->getNonTerminals().size() << "\n";
        oss << "  Terminals: " << grammar->getTerminals().size() << "\n";
        oss << "  Productions: " << grammar->getProductions().size() << "\n";
    }
    
    if (parseTable) {
        oss << "  LL(1): " << (parseTable->isGrammarLL1() ? "Yes" : "No") << "\n";
        oss << "  Table Entries: " << parseTable->getEntryCount() << "\n";
    }
    
    return oss.str();
}
