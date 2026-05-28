#ifndef COMPILER_H
#define COMPILER_H

#include "lexer.h"
#include "parser.h"
#include "semantic_analyzer.h"
#include "ast_node.h"
#include "symbol_table.h"
#include <string>
#include <memory>

enum class CompilationResult {
    SUCCESS,
    LEXICAL_ERROR,
    SYNTAX_ERROR,
    SEMANTIC_ERROR,
    INTERNAL_ERROR
};

class Compiler {
private:
    std::string sourceCode;
    std::unique_ptr<Lexer> lexer;
    std::unique_ptr<Parser> parser;
    std::unique_ptr<SemanticAnalyzer> semanticAnalyzer;
    
    // Compilation results
    std::vector<std::unique_ptr<Token>> tokens;
    std::shared_ptr<ASTNode> ast;
    CompilationResult result;
    
    // Error tracking
    std::vector<std::string> allErrors;
    std::vector<std::string> allWarnings;
    
    // Compilation phases
    bool performLexicalAnalysis();
    bool performSyntacticAnalysis();
    bool performSemanticAnalysis();
    
    // Error aggregation
    void aggregateErrors();
    void aggregateWarnings();
    
    // Utility methods
    void reset();
    void printCompilationPhase(const std::string& phase);
    void printPhaseResult(bool success, const std::string& phase);

public:
    explicit Compiler(const std::string& sourceCode);
    ~Compiler() = default;
    
    // Main compilation method
    CompilationResult compile();
    
    // Individual compilation phases
    bool tokenize();
    bool parse();
    bool analyze();
    
    // Result access
    CompilationResult getResult() const { return result; }
    bool isSuccessful() const { return result == CompilationResult::SUCCESS; }
    const std::vector<std::unique_ptr<Token>>& getTokens() const { return tokens; }
    std::shared_ptr<ASTNode> getAST() const { return ast; }
    std::shared_ptr<SymbolTable> getSymbolTable() const;
    
    // Error reporting
    const std::vector<std::string>& getAllErrors() const { return allErrors; }
    const std::vector<std::string>& getAllWarnings() const { return allWarnings; }
    bool hasErrors() const { return !allErrors.empty(); }
    bool hasWarnings() const { return !allWarnings.empty(); }
    int getErrorCount() const { return static_cast<int>(allErrors.size()); }
    int getWarningCount() const { return static_cast<int>(allWarnings.size()); }
    
    // Display methods
    void printTokens() const;
    void printAST() const;
    void printSymbolTable() const;
    void printErrors() const;
    void printWarnings() const;
    void printCompilationReport() const;
    void printDetailedReport() const;
    
    // Static utility methods
    static std::string resultToString(CompilationResult result);
    static std::string getCompilerInfo();
    
    // Test and debugging
    bool runSelfTest();
    void enableDebugMode(bool enable = true);
};

#endif // COMPILER_H
