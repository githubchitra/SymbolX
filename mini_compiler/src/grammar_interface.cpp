#include "grammar_interface.h"
#include <iostream>
#include <fstream>
#include <sstream>

GrammarInterface::GrammarInterface() : grammarLoaded(false), tableBuilt(false) {
    initializeComponents();
}

void GrammarInterface::initializeComponents() {
    grammar = std::make_shared<CFGGrammar>();
    firstFollow = std::make_shared<FirstFollowSets>(grammar);
    parseTable = std::make_shared<LL1ParserTable>(grammar, firstFollow);
    parser = std::make_shared<DynamicLL1Parser>(grammar, firstFollow, parseTable);
}

void GrammarInterface::clearComponents() {
    grammar->clear();
    firstFollow->recompute();
    parseTable->rebuild();
    parser->reset();
    errors.clear();
    warnings.clear();
    grammarLoaded = false;
    tableBuilt = false;
}

bool GrammarInterface::loadGrammarFromString(const std::string& grammarString) {
    clearComponents();
    currentGrammarString = grammarString;
    
    if (!grammar->parseGrammarFromString(grammarString)) {
        errors = grammar->getErrors();
        return false;
    }
    
    grammarLoaded = true;
    return true;
}

bool GrammarInterface::loadGrammarFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        errors.push_back("Cannot open file: " + filename);
        return false;
    }
    
    std::string grammarString((std::istreambuf_iterator<char>(file)),
                             std::istreambuf_iterator<char>());
    file.close();
    
    return loadGrammarFromString(grammarString);
}

bool GrammarInterface::loadGrammarFromInteractive() {
    std::cout << "=== Interactive Grammar Input ===\n";
    std::cout << "Enter grammar rules (one per line, format: A -> B C | D)\n";
    std::cout << "Enter 'END' on a separate line to finish input\n\n";
    
    std::string grammarString;
    std::string line;
    int lineNumber = 1;
    
    while (true) {
        std::cout << lineNumber << "> ";
        std::getline(std::cin, line);
        
        if (line == "END" || line == "end") {
            break;
        }
        
        if (!line.empty()) {
            grammarString += line + "\n";
            lineNumber++;
        }
    }
    
    return loadGrammarFromString(grammarString);
}

bool GrammarInterface::loadExpressionGrammar() {
    std::string exprGrammar = R"(
E -> E + T | E - T | T
T -> T * F | T / F | F
F -> ( E ) | id
)";
    return loadGrammarFromString(exprGrammar);
}

bool GrammarInterface::loadSimpleArithmeticGrammar() {
    std::string arithmeticGrammar = R"(
E -> T E'
E' -> + T E' | - T E' | ε
T -> F T'
T' -> * F T' | / F T' | ε
F -> ( E ) | id | num
)";
    return loadGrammarFromString(arithmeticGrammar);
}

bool GrammarInterface::loadStatementGrammar() {
    std::string stmtGrammar = R"(
S -> if E then S else S | if E then S | while E do S | id = E
E -> E + E | E * E | ( E ) | id
)";
    return loadGrammarFromString(stmtGrammar);
}

bool GrammarInterface::loadCustomGrammar(const std::string& grammarString) {
    return loadGrammarFromString(grammarString);
}

bool GrammarInterface::analyzeGrammar() {
    if (!grammarLoaded) {
        errors.push_back("No grammar loaded");
        return false;
    }
    
    firstFollow->computeAll();
    
    // Check for LL(1) conflicts
    auto conflicts = firstFollow->getLL1Conflicts();
    for (const auto& conflict : conflicts) {
        warnings.push_back(conflict);
    }
    
    return true;
}

bool GrammarInterface::buildParsingTable() {
    if (!grammarLoaded) {
        errors.push_back("No grammar loaded");
        return false;
    }
    
    if (!firstFollow->isLL1()) {
        errors.push_back("Grammar is not LL(1), cannot build parsing table");
        return false;
    }
    
    parseTable->build();
    tableBuilt = parseTable->isTableBuilt();
    
    if (parseTable->hasConflicts()) {
        errors = parseTable->getConflicts();
        return false;
    }
    
    return tableBuilt;
}

bool GrammarInterface::validateGrammar() {
    if (!grammarLoaded) {
        return false;
    }
    
    return grammar->isValid() && firstFollow->isLL1();
}

bool GrammarInterface::parseInput(const std::string& input) {
    if (!grammarLoaded) {
        errors.push_back("No grammar loaded");
        return false;
    }
    
    if (!tableBuilt) {
        if (!buildParsingTable()) {
            return false;
        }
    }
    
    bool success = parser->parse(input);
    
    errors = parser->getErrors();
    warnings = parser->getWarnings();
    
    return success;
}

bool GrammarInterface::parseInteractive() {
    if (!grammarLoaded) {
        std::cout << "No grammar loaded. Please load a grammar first.\n";
        return false;
    }
    
    if (!tableBuilt) {
        if (!buildParsingTable()) {
            std::cout << "Failed to build parsing table.\n";
            return false;
        }
    }
    
    std::cout << "=== Interactive Parsing ===\n";
    std::cout << "Enter input strings to parse (type 'EXIT' to quit)\n\n";
    
    while (true) {
        std::cout << "Input> ";
        std::string input;
        std::getline(std::cin, input);
        
        if (input == "EXIT" || input == "exit") {
            break;
        }
        
        if (input.empty()) {
            continue;
        }
        
        bool success = parseInput(input);
        
        if (success) {
            std::cout << "Parsing successful!\n";
            printParseTree();
        } else {
            std::cout << "Parsing failed!\n";
            printErrors();
        }
        
        std::cout << "\n";
    }
    
    return true;
}

void GrammarInterface::printGrammar() const {
    if (grammarLoaded) {
        grammar->printGrammar();
    } else {
        std::cout << "No grammar loaded.\n";
    }
}

void GrammarInterface::printFirstFollowSets() const {
    if (grammarLoaded) {
        firstFollow->printAllSets();
    } else {
        std::cout << "No grammar loaded.\n";
    }
}

void GrammarInterface::printParsingTable() const {
    if (tableBuilt) {
        parseTable->printTable();
    } else {
        std::cout << "Parsing table not built.\n";
    }
}

void GrammarInterface::printParseTree() const {
    if (parser) {
        parser->printParseTree();
    } else {
        std::cout << "No parser available.\n";
    }
}

void GrammarInterface::printParsingSteps() const {
    if (parser) {
        parser->printParsingSteps();
    } else {
        std::cout << "No parser available.\n";
    }
}

void GrammarInterface::printErrors() const {
    std::cout << "=== Errors ===\n";
    if (errors.empty()) {
        std::cout << "No errors.\n";
    } else {
        for (size_t i = 0; i < errors.size(); ++i) {
            std::cout << (i + 1) << ". " << errors[i] << "\n";
        }
    }
}

void GrammarInterface::printWarnings() const {
    std::cout << "=== Warnings ===\n";
    if (warnings.empty()) {
        std::cout << "No warnings.\n";
    } else {
        for (size_t i = 0; i < warnings.size(); ++i) {
            std::cout << (i + 1) << ". " << warnings[i] << "\n";
        }
    }
}

void GrammarInterface::printFullReport() const {
    std::cout << "=== Full Grammar Analysis Report ===\n\n";
    
    std::cout << "Grammar Status: " << (grammarLoaded ? "Loaded" : "Not Loaded") << "\n";
    std::cout << "Table Status: " << (tableBuilt ? "Built" : "Not Built") << "\n";
    std::cout << "LL(1) Status: " << (isLL1() ? "Yes" : "No") << "\n\n";
    
    if (grammarLoaded) {
        printGrammar();
        std::cout << "\n";
        printFirstFollowSets();
        std::cout << "\n";
    }
    
    if (tableBuilt) {
        printParsingTable();
        std::cout << "\n";
    }
    
    printErrors();
    std::cout << "\n";
    printWarnings();
}

bool GrammarInterface::exportParseTreeDOT(const std::string& filename) {
    if (!parser || !parser->getParseTree()) {
        return false;
    }
    
    std::ofstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    // Simple DOT export
    file << "digraph ParseTree {\n";
    file << "  rankdir=TB;\n";
    
    std::function<void(std::shared_ptr<ParseTreeNode>, int)> exportNode;
    int nodeId = 0;
    
    exportNode = [&](std::shared_ptr<ParseTreeNode> node, int parentId) {
        if (!node) return;
        
        int currentId = nodeId++;
        file << "  node" << currentId << " [label=\"" << node->getSymbol() << "\"];\n";
        
        if (parentId != -1) {
            file << "  node" << parentId << " -> node" << currentId << ";\n";
        }
        
        for (const auto& child : node->getChildren()) {
            exportNode(child, currentId);
        }
    };
    
    exportNode(parser->getParseTree(), -1);
    file << "}\n";
    
    file.close();
    return true;
}

bool GrammarInterface::exportParseTreeJSON(const std::string& filename) {
    if (!parser || !parser->getParseTree()) {
        return false;
    }
    
    std::ofstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    // Simple JSON export
    std::function<void(std::shared_ptr<ParseTreeNode>)> exportNode;
    
    exportNode = [&](std::shared_ptr<ParseTreeNode> node) {
        if (!node) {
            file << "null";
            return;
        }
        
        file << "{\n";
        file << "  \"symbol\": \"" << node->getSymbol() << "\",\n";
        file << "  \"isNonTerminal\": " << (node->isNonTerminalNode() ? "true" : "false") << ",\n";
        
        if (node->getRuleNumber() != -1) {
            file << "  \"ruleNumber\": " << node->getRuleNumber() << ",\n";
        }
        
        file << "  \"children\": [";
        
        const auto& children = node->getChildren();
        for (size_t i = 0; i < children.size(); ++i) {
            exportNode(children[i]);
            if (i < children.size() - 1) {
                file << ",";
            }
        }
        
        file << "]\n";
        file << "}";
    };
    
    exportNode(parser->getParseTree());
    file.close();
    
    return true;
}

bool GrammarInterface::exportParsingTableCSV(const std::string& filename) {
    if (!tableBuilt) {
        return false;
    }
    
    std::ofstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    parseTable->printTableAsCSV();
    
    file.close();
    return true;
}

bool GrammarInterface::isLL1() const {
    return firstFollow && firstFollow->isLL1();
}

void GrammarInterface::reset() {
    clearComponents();
}

std::string GrammarInterface::getStatus() const {
    std::ostringstream oss;
    
    oss << "Grammar Interface Status:\n";
    oss << "  Grammar Loaded: " << (grammarLoaded ? "Yes" : "No") << "\n";
    oss << "  Table Built: " << (tableBuilt ? "Yes" : "No") << "\n";
    oss << "  Is LL(1): " << (isLL1() ? "Yes" : "No") << "\n";
    oss << "  Errors: " << errors.size() << "\n";
    oss << "  Warnings: " << warnings.size() << "\n";
    
    return oss.str();
}

std::string GrammarInterface::getHelp() const {
    return R"(
Grammar Interface Help
=====================

Commands:
  load <file>           - Load grammar from file
  load-interactive      - Load grammar interactively
  load-expression       - Load expression grammar
  load-arithmetic       - Load arithmetic grammar
  load-statement        - Load statement grammar
  analyze               - Analyze grammar (First/Follow sets)
  build-table           - Build LL(1) parsing table
  parse <input>         - Parse input string
  parse-interactive     - Interactive parsing mode
  print-grammar         - Print current grammar
  print-first-follow     - Print First/Follow sets
  print-table           - Print parsing table
  print-tree            - Print parse tree
  print-steps           - Print parsing steps
  print-errors          - Print errors
  print-warnings        - Print warnings
  print-report          - Print full report
  export-dot <file>     - Export parse tree as DOT
  export-json <file>    - Export parse tree as JSON
  export-csv <file>     - Export parsing table as CSV
  status                - Show current status
  reset                 - Reset all components
  help                  - Show this help
  exit                  - Exit interface

Grammar Format:
  A -> B C | D
  Non-terminals: uppercase letters or multi-character names
  Terminals: lowercase letters or quoted strings
  Epsilon: ε or epsilon
  End marker: $
)";
}

std::string GrammarInterface::getExpressionGrammarExample() {
    return R"(
E -> E + T | E - T | T
T -> T * F | T / F | F
F -> ( E ) | id
)";
}

std::string GrammarInterface::getArithmeticGrammarExample() {
    return R"(
E -> T E'
E' -> + T E' | - T E' | ε
T -> F T'
T' -> * F T' | / F T' | ε
F -> ( E ) | id | num
)";
}

std::string GrammarInterface::getStatementGrammarExample() {
    return R"(
S -> if E then S else S | if E then S | while E do S | id = E
E -> E + E | E * E | ( E ) | id
)";
}
