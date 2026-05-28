#ifndef PARSER_H
#define PARSER_H

#include "token.h"
#include "ast_node.h"
#include "symbol_table.h"
#include <vector>
#include <memory>
#include <stack>

class Parser {
private:
    std::vector<std::unique_ptr<Token>> tokens;
    int current;
    std::shared_ptr<SymbolTable> symbolTable;
    
    // Error tracking
    std::vector<std::string> errors;
    std::vector<std::string> warnings;
    
    // Parsing context
    std::stack<std::string> functionStack;
    bool inFunction;
    std::string currentFunction;
    
    // Helper methods
    std::unique_ptr<Token> getCurrentToken();
    std::unique_ptr<Token> peekToken(int offset = 1);
    std::unique_ptr<Token> advance();
    bool match(TokenType type);
    bool consume(TokenType type, const std::string& errorMessage);
    bool isAtEnd();
    
    // Error handling
    void addError(const std::string& message);
    void addWarning(const std::string& message);
    void addErrorAtToken(const std::string& message, const Token& token);
    
    // Grammar methods - Declarations
    std::shared_ptr<ASTNode> parseProgram();
    std::shared_ptr<ASTNode> parseDeclaration();
    std::shared_ptr<ASTNode> parseVariableDeclaration();
    std::shared_ptr<ASTNode> parseFunctionDeclaration();
    std::shared_ptr<ASTNode> parseFunctionDefinition();
    std::shared_ptr<ASTNode> parseParameterList();
    std::shared_ptr<ASTNode> parseParameter();
    
    // Grammar methods - Statements
    std::shared_ptr<ASTNode> parseStatement();
    std::shared_ptr<ASTNode> parseExpressionStatement();
    std::shared_ptr<ASTNode> parseCompoundStatement();
    std::shared_ptr<ASTNode> parseIfStatement();
    std::shared_ptr<ASTNode> parseWhileStatement();
    std::shared_ptr<ASTNode> parseForStatement();
    std::shared_ptr<ASTNode> parseReturnStatement();
    
    // Grammar methods - Expressions
    std::shared_ptr<ASTNode> parseExpression();
    std::shared_ptr<ASTNode> parseAssignmentExpression();
    std::shared_ptr<ASTNode> parseEqualityExpression();
    std::shared_ptr<ASTNode> parseRelationalExpression();
    std::shared_ptr<ASTNode> parseAdditiveExpression();
    std::shared_ptr<ASTNode> parseMultiplicativeExpression();
    std::shared_ptr<ASTNode> parseUnaryExpression();
    std::shared_ptr<ASTNode> parsePrimaryExpression();
    std::shared_ptr<ASTNode> parseFunctionCall();
    
    // Utility methods
    DataType tokenTypeToDataType(TokenType tokenType);
    std::string generateFunctionName(const std::string& baseName);
    void synchronize();
    
    // Symbol table integration
    void enterScope(const std::string& scopeName);
    void exitScope();
    bool declareSymbol(const std::string& name, SymbolKind kind, DataType dataType);
    bool useSymbol(const std::string& name);

public:
    explicit Parser(std::vector<std::unique_ptr<Token>> tokens);
    
    // Main parsing method
    std::shared_ptr<ASTNode> parse();
    
    // Error handling
    const std::vector<std::string>& getErrors() const { return errors; }
    const std::vector<std::string>& getWarnings() const { return warnings; }
    bool hasErrors() const { return !errors.empty(); }
    bool hasWarnings() const { return !warnings.empty(); }
    
    // Symbol table access
    std::shared_ptr<SymbolTable> getSymbolTable() { return symbolTable; }
    
    // Utility methods
    void printAST(std::shared_ptr<ASTNode> root) const;
    void printErrors() const;
    void printWarnings() const;
    
    // Static utility methods
    static std::string nodeTypeToString(NodeType type);
};

#endif // PARSER_H
