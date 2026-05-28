#ifndef SEMANTIC_ANALYZER_H
#define SEMANTIC_ANALYZER_H

#include "ast_node.h"
#include "symbol_table.h"
#include <vector>
#include <memory>
#include <string>

enum class SemanticErrorType {
    UNDECLARED_VARIABLE,
    UNDECLARED_FUNCTION,
    TYPE_MISMATCH,
    INVALID_ASSIGNMENT,
    INVALID_OPERATION,
    FUNCTION_CALL_MISMATCH,
    RETURN_TYPE_MISMATCH,
    VOID_USAGE,
    REDECLARATION,
    UNINITIALIZED_VARIABLE,
    BREAK_CONTINUE_OUTSIDE_LOOP,
    INVALID_CAST,
    ARRAY_INDEX_ERROR,
    UNKNOWN
};

class SemanticError {
private:
    SemanticErrorType errorType;
    std::string message;
    int line;
    int column;
    std::string severity; // "error" or "warning"

public:
    SemanticError(SemanticErrorType type, const std::string& message, int line, int column, const std::string& severity = "error");
    
    // Getters
    SemanticErrorType getErrorType() const { return errorType; }
    const std::string& getMessage() const { return message; }
    int getLine() const { return line; }
    int getColumn() const { return column; }
    const std::string& getSeverity() const { return severity; }
    
    // Display
    std::string toString() const;
    void print() const;
    
    // Static utility
    static std::string errorTypeToString(SemanticErrorType type);
};

class SemanticAnalyzer {
private:
    std::shared_ptr<SymbolTable> symbolTable;
    std::vector<SemanticError> errors;
    std::vector<SemanticError> warnings;
    
    // Analysis context
    std::string currentFunction;
    DataType currentReturnType;
    bool inLoop;
    bool hasReturn;
    
    // Error reporting
    void addError(SemanticErrorType type, const std::string& message, int line, int column);
    void addWarning(SemanticErrorType type, const std::string& message, int line, int column);
    
    // Type checking methods
    DataType getExpressionType(std::shared_ptr<ASTNode> node);
    bool isCompatibleTypes(DataType source, DataType target);
    bool canImplicitCast(DataType from, DataType to);
    DataType getResultType(DataType left, DataType right, const std::string& op);
    
    // Validation methods
    bool validateBinaryExpression(std::shared_ptr<ASTNode> node);
    bool validateUnaryExpression(std::shared_ptr<ASTNode> node);
    bool validateAssignment(std::shared_ptr<ASTNode> node);
    bool validateFunctionCall(std::shared_ptr<ASTNode> node);
    bool validateVariableDeclaration(std::shared_ptr<ASTNode> node);
    bool validateReturnStatement(std::shared_ptr<ASTNode> node);
    bool validateIfStatement(std::shared_ptr<ASTNode> node);
    bool validateWhileStatement(std::shared_ptr<ASTNode> node);
    bool validateForStatement(std::shared_ptr<ASTNode> node);
    
    // Helper methods
    std::shared_ptr<SymbolInfo> lookupSymbol(const std::string& name);
    bool isLValue(std::shared_ptr<ASTNode> node);
    std::vector<DataType> getArgumentTypes(std::shared_ptr<ASTNode> functionCall);
    bool checkFunctionParameters(const std::string& functionName, const std::vector<DataType>& argTypes);

public:
    explicit SemanticAnalyzer(std::shared_ptr<SymbolTable> symbolTable);
    
    // Main analysis method
    bool analyze(std::shared_ptr<ASTNode> ast);
    
    // Node visitors
    void visitProgram(std::shared_ptr<ASTNode> node);
    void visitFunctionDeclaration(std::shared_ptr<ASTNode> node);
    void visitVariableDeclaration(std::shared_ptr<ASTNode> node);
    void visitStatement(std::shared_ptr<ASTNode> node);
    void visitExpression(std::shared_ptr<ASTNode> node);
    void visitBinaryExpression(std::shared_ptr<ASTNode> node);
    void visitUnaryExpression(std::shared_ptr<ASTNode> node);
    void visitAssignmentExpression(std::shared_ptr<ASTNode> node);
    void visitFunctionCall(std::shared_ptr<ASTNode> node);
    void visitIdentifierExpression(std::shared_ptr<ASTNode> node);
    void visitLiteralExpression(std::shared_ptr<ASTNode> node);
    void visitIfStatement(std::shared_ptr<ASTNode> node);
    void visitWhileStatement(std::shared_ptr<ASTNode> node);
    void visitForStatement(std::shared_ptr<ASTNode> node);
    void visitReturnStatement(std::shared_ptr<ASTNode> node);
    void visitCompoundStatement(std::shared_ptr<ASTNode> node);
    
    // Error handling
    const std::vector<SemanticError>& getErrors() const { return errors; }
    const std::vector<SemanticError>& getWarnings() const { return warnings; }
    bool hasErrors() const { return !errors.empty(); }
    bool hasWarnings() const { return !warnings.empty(); }
    bool hasIssues() const { return hasErrors() || hasWarnings(); }
    
    // Utility methods
    void printErrors() const;
    void printWarnings() const;
    void printAllIssues() const;
    void clearIssues();
    
    // Statistics
    int getErrorCount() const { return static_cast<int>(errors.size()); }
    int getWarningCount() const { return static_cast<int>(warnings.size()); }
    int getTotalIssueCount() const { return getErrorCount() + getWarningCount(); }
    
    // Type system utilities
    static bool isNumericType(DataType type);
    static bool isIntegralType(DataType type);
    static bool isVoidType(DataType type);
    static std::string dataTypeToString(DataType type);
};

#endif // SEMANTIC_ANALYZER_H
