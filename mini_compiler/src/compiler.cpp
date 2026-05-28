#include "compiler.h"
#include <iostream>
#include <iomanip>
#include <chrono>

Compiler::Compiler(const std::string& sourceCode) 
    : sourceCode(sourceCode), result(CompilationResult::SUCCESS) {
    lexer = std::make_unique<Lexer>(sourceCode);
}

CompilationResult Compiler::compile() {
    reset();
    
    std::cout << "=== Mini Compiler Front-End ===\n";
    std::cout << "Starting compilation...\n\n";
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Phase 1: Lexical Analysis
    printCompilationPhase("Phase 1: Lexical Analysis");
    if (!performLexicalAnalysis()) {
        result = CompilationResult::LEXICAL_ERROR;
        printPhaseResult(false, "Lexical Analysis");
        return result;
    }
    printPhaseResult(true, "Lexical Analysis");
    std::cout << "  Tokens generated: " << tokens.size() << "\n\n";
    
    // Phase 2: Syntactic Analysis
    printCompilationPhase("Phase 2: Syntactic Analysis");
    if (!performSyntacticAnalysis()) {
        result = CompilationResult::SYNTAX_ERROR;
        printPhaseResult(false, "Syntactic Analysis");
        return result;
    }
    printPhaseResult(true, "Syntactic Analysis");
    std::cout << "  AST constructed successfully\n\n";
    
    // Phase 3: Semantic Analysis
    printCompilationPhase("Phase 3: Semantic Analysis");
    if (!performSemanticAnalysis()) {
        result = CompilationResult::SEMANTIC_ERROR;
        printPhaseResult(false, "Semantic Analysis");
        return result;
    }
    printPhaseResult(true, "Semantic Analysis");
    std::cout << "  Semantic validation completed\n\n";
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    
    // Success
    result = CompilationResult::SUCCESS;
    std::cout << "=== Compilation Successful ===\n";
    std::cout << "Total time: " << duration.count() << " ms\n";
    std::cout << "Errors: " << getErrorCount() << "\n";
    std::cout << "Warnings: " << getWarningCount() << "\n\n";
    
    return result;
}

bool Compiler::performLexicalAnalysis() {
    if (!lexer) {
        return false;
    }
    
    tokens = lexer->tokenizeAll();
    
    // Check for lexer errors
    if (lexer->hasErrors()) {
        for (const auto& error : lexer->getErrors()) {
            allErrors.push_back("Lexical Error: " + error);
        }
        return false;
    }
    
    // Check for lexer warnings
    for (const auto& warning : lexer->getWarnings()) {
        allWarnings.push_back("Lexical Warning: " + warning);
    }
    
    return true;
}

bool Compiler::performSyntacticAnalysis() {
    if (tokens.empty()) {
        allErrors.push_back("Syntax Error: No tokens to parse");
        return false;
    }
    
    parser = std::make_unique<Parser>(std::move(tokens));
    ast = parser->parse();
    
    // Check for parser errors
    if (parser->hasErrors()) {
        for (const auto& error : parser->getErrors()) {
            allErrors.push_back("Syntax Error: " + error);
        }
        return false;
    }
    
    // Check for parser warnings
    for (const auto& warning : parser->getWarnings()) {
        allWarnings.push_back("Syntax Warning: " + warning);
    }
    
    return ast != nullptr;
}

bool Compiler::performSemanticAnalysis() {
    if (!ast) {
        allErrors.push_back("Semantic Error: No AST to analyze");
        return false;
    }
    
    auto symbolTable = parser ? parser->getSymbolTable() : nullptr;
    if (!symbolTable) {
        allErrors.push_back("Semantic Error: No symbol table available");
        return false;
    }
    
    semanticAnalyzer = std::make_unique<SemanticAnalyzer>(symbolTable);
    bool success = semanticAnalyzer->analyze(ast);
    
    // Check for semantic errors
    if (semanticAnalyzer->hasErrors()) {
        for (const auto& error : semanticAnalyzer->getErrors()) {
            allErrors.push_back("Semantic Error: " + error.toString());
        }
        return false;
    }
    
    // Check for semantic warnings
    for (const auto& warning : semanticAnalyzer->getWarnings()) {
        allWarnings.push_back("Semantic Warning: " + warning.toString());
    }
    
    return success;
}

void Compiler::aggregateErrors() {
    allErrors.clear();
    
    // Lexer errors
    if (lexer && lexer->hasErrors()) {
        for (const auto& error : lexer->getErrors()) {
            allErrors.push_back("Lexical: " + error);
        }
    }
    
    // Parser errors
    if (parser && parser->hasErrors()) {
        for (const auto& error : parser->getErrors()) {
            allErrors.push_back("Syntax: " + error);
        }
    }
    
    // Semantic errors
    if (semanticAnalyzer && semanticAnalyzer->hasErrors()) {
        for (const auto& error : semanticAnalyzer->getErrors()) {
            allErrors.push_back("Semantic: " + error.toString());
        }
    }
}

void Compiler::aggregateWarnings() {
    allWarnings.clear();
    
    // Lexer warnings
    if (lexer && lexer->hasWarnings()) {
        for (const auto& warning : lexer->getWarnings()) {
            allWarnings.push_back("Lexical: " + warning);
        }
    }
    
    // Parser warnings
    if (parser && parser->hasWarnings()) {
        for (const auto& warning : parser->getWarnings()) {
            allWarnings.push_back("Syntax: " + warning);
        }
    }
    
    // Semantic warnings
    if (semanticAnalyzer && semanticAnalyzer->hasWarnings()) {
        for (const auto& warning : semanticAnalyzer->getWarnings()) {
            allWarnings.push_back("Semantic: " + warning.toString());
        }
    }
}

void Compiler::reset() {
    tokens.clear();
    ast = nullptr;
    result = CompilationResult::SUCCESS;
    allErrors.clear();
    allWarnings.clear();
    
    // Reset lexer
    if (lexer) {
        lexer->clearErrors();
    }
    
    // Reset parser and semantic analyzer will be created fresh
    parser.reset();
    semanticAnalyzer.reset();
}

void Compiler::printCompilationPhase(const std::string& phase) {
    std::cout << std::string(50, '=') << "\n";
    std::cout << phase << "\n";
    std::cout << std::string(50, '-') << "\n";
}

void Compiler::printPhaseResult(bool success, const std::string& phase) {
    std::cout << std::string(50, '-') << "\n";
    std::cout << (success ? "✓ " : "✗ ") << phase;
    std::cout << (success ? " completed successfully\n" : " failed\n");
    std::cout << std::string(50, '=') << "\n";
}

bool Compiler::tokenize() {
    return performLexicalAnalysis();
}

bool Compiler::parse() {
    if (tokens.empty()) {
        return performLexicalAnalysis() && performSyntacticAnalysis();
    }
    return performSyntacticAnalysis();
}

bool Compiler::analyze() {
    if (!ast) {
        return performLexicalAnalysis() && performSyntacticAnalysis() && performSemanticAnalysis();
    }
    return performSemanticAnalysis();
}

std::shared_ptr<SymbolTable> Compiler::getSymbolTable() const {
    return parser ? parser->getSymbolTable() : nullptr;
}

void Compiler::printTokens() const {
    std::cout << "=== Tokens ===\n";
    if (tokens.empty()) {
        std::cout << "No tokens available\n";
        return;
    }
    
    for (const auto& token : tokens) {
        token->print();
    }
    std::cout << "Total tokens: " << tokens.size() << "\n";
}

void Compiler::printAST() const {
    std::cout << "=== Abstract Syntax Tree ===\n";
    if (!ast) {
        std::cout << "No AST available\n";
        return;
    }
    
    ast->printTree();
}

void Compiler::printSymbolTable() const {
    std::cout << "=== Symbol Table ===\n";
    auto symbolTable = getSymbolTable();
    if (!symbolTable) {
        std::cout << "No symbol table available\n";
        return;
    }
    
    symbolTable->printSymbolTable();
}

void Compiler::printErrors() const {
    std::cout << "=== Compilation Errors ===\n";
    if (allErrors.empty()) {
        std::cout << "No errors\n";
        return;
    }
    
    for (size_t i = 0; i < allErrors.size(); ++i) {
        std::cout << (i + 1) << ". " << allErrors[i] << "\n";
    }
    std::cout << "Total errors: " << allErrors.size() << "\n";
}

void Compiler::printWarnings() const {
    std::cout << "=== Compilation Warnings ===\n";
    if (allWarnings.empty()) {
        std::cout << "No warnings\n";
        return;
    }
    
    for (size_t i = 0; i < allWarnings.size(); ++i) {
        std::cout << (i + 1) << ". " << allWarnings[i] << "\n";
    }
    std::cout << "Total warnings: " << allWarnings.size() << "\n";
}

void Compiler::printCompilationReport() const {
    std::cout << "\n=== Compilation Report ===\n";
    std::cout << "Result: " << resultToString(result) << "\n";
    std::cout << "Errors: " << getErrorCount() << "\n";
    std::cout << "Warnings: " << getWarningCount() << "\n";
    
    if (hasErrors()) {
        std::cout << "\nErrors:\n";
        for (size_t i = 0; i < std::min(size_t(5), allErrors.size()); ++i) {
            std::cout << "  " << (i + 1) << ". " << allErrors[i] << "\n";
        }
        if (allErrors.size() > 5) {
            std::cout << "  ... and " << (allErrors.size() - 5) << " more errors\n";
        }
    }
    
    if (hasWarnings()) {
        std::cout << "\nWarnings:\n";
        for (size_t i = 0; i < std::min(size_t(5), allWarnings.size()); ++i) {
            std::cout << "  " << (i + 1) << ". " << allWarnings[i] << "\n";
        }
        if (allWarnings.size() > 5) {
            std::cout << "  ... and " << (allWarnings.size() - 5) << " more warnings\n";
        }
    }
}

void Compiler::printDetailedReport() const {
    std::cout << "\n=== Detailed Compilation Report ===\n";
    
    // Compiler info
    std::cout << getCompilerInfo() << "\n\n";
    
    // Source code info
    std::cout << "Source Code Statistics:\n";
    std::cout << "  Lines: " << std::count(sourceCode.begin(), sourceCode.end(), '\n') + 1 << "\n";
    std::cout << "  Characters: " << sourceCode.length() << "\n\n";
    
    // Compilation phases
    std::cout << "Compilation Phases:\n";
    std::cout << "  1. Lexical Analysis: " << (tokens.empty() ? "Failed" : "Success") << "\n";
    std::cout << "  2. Syntactic Analysis: " << (ast ? "Success" : "Failed") << "\n";
    std::cout << "  3. Semantic Analysis: " << (semanticAnalyzer ? "Success" : "Failed") << "\n\n";
    
    // Statistics
    std::cout << "Statistics:\n";
    std::cout << "  Tokens: " << tokens.size() << "\n";
    std::cout << "  AST Nodes: " << (ast ? "N/A" : "0") << "\n";
    
    auto symbolTable = getSymbolTable();
    if (symbolTable) {
        std::cout << "  Symbols: " << symbolTable->getTotalSymbolCount() << "\n";
        std::cout << "  Scopes: " << symbolTable->getCurrentScopeLevel() + 1 << "\n";
    }
    
    std::cout << "  Errors: " << getErrorCount() << "\n";
    std::cout << "  Warnings: " << getWarningCount() << "\n\n";
    
    // All errors and warnings
    printErrors();
    std::cout << "\n";
    printWarnings();
}

std::string Compiler::resultToString(CompilationResult result) {
    switch (result) {
        case CompilationResult::SUCCESS: return "Success";
        case CompilationResult::LEXICAL_ERROR: return "Lexical Error";
        case CompilationResult::SYNTAX_ERROR: return "Syntax Error";
        case CompilationResult::SEMANTIC_ERROR: return "Semantic Error";
        case CompilationResult::INTERNAL_ERROR: return "Internal Error";
        default: return "Unknown";
    }
}

std::string Compiler::getCompilerInfo() {
    return "Mini Compiler Front-End v1.0\n"
           "Features: Lexical Analysis, Syntax Analysis, Semantic Analysis\n"
           "Language: Subset of C (int, float, char, void)\n"
           "Author: AI Assistant";
}

bool Compiler::runSelfTest() {
    std::cout << "=== Compiler Self-Test ===\n";
    
    // Test 1: Simple variable declaration
    std::string test1 = "int x = 5;";
    Compiler testCompiler1(test1);
    auto result1 = testCompiler1.compile();
    
    std::cout << "Test 1 - Variable Declaration: " << resultToString(result1) << "\n";
    if (result1 != CompilationResult::SUCCESS) {
        testCompiler1.printErrors();
    }
    
    // Test 2: Function definition
    std::string test2 = "int add(int a, int b) { return a + b; }";
    Compiler testCompiler2(test2);
    auto result2 = testCompiler2.compile();
    
    std::cout << "Test 2 - Function Definition: " << resultToString(result2) << "\n";
    if (result2 != CompilationResult::SUCCESS) {
        testCompiler2.printErrors();
    }
    
    // Test 3: Error case (undeclared variable)
    std::string test3 = "int x = y + 1;";
    Compiler testCompiler3(test3);
    auto result3 = testCompiler3.compile();
    
    std::cout << "Test 3 - Undeclared Variable: " << resultToString(result3) << "\n";
    if (result3 == CompilationResult::SUCCESS) {
        std::cout << "ERROR: Should have detected undeclared variable\n";
        return false;
    }
    
    std::cout << "Self-Test completed\n";
    return true;
}

void Compiler::enableDebugMode(bool enable) {
    if (enable) {
        std::cout << "Debug mode enabled\n";
    }
}
