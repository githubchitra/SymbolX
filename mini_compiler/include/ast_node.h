#ifndef AST_NODE_H
#define AST_NODE_H

#include <vector>
#include <string>
#include <memory>
#include <iostream>

// Forward declarations
class SymbolTable;
class SymbolInfo;

enum class NodeType {
    // Program structure
    PROGRAM,
    FUNCTION_DECLARATION,
    FUNCTION_DEFINITION,
    PARAMETER_LIST,
    PARAMETER,
    BLOCK,
    
    // Declarations
    VARIABLE_DECLARATION,
    VARIABLE_DECLARATION_LIST,
    
    // Statements
    EXPRESSION_STATEMENT,
    IF_STATEMENT,
    WHILE_STATEMENT,
    FOR_STATEMENT,
    RETURN_STATEMENT,
    COMPOUND_STATEMENT,
    
    // Expressions
    BINARY_EXPRESSION,
    UNARY_EXPRESSION,
    ASSIGNMENT_EXPRESSION,
    FUNCTION_CALL,
    IDENTIFIER_EXPRESSION,
    LITERAL_EXPRESSION,
    
    // Types
    TYPE_SPECIFIER,
    
    // Unknown
    UNKNOWN
};

enum class DataType {
    VOID,
    INT,
    FLOAT,
    CHAR,
    STRING,
    UNKNOWN
};

class ASTNode {
protected:
    NodeType type;
    std::string value;
    DataType dataType;
    int line;
    int column;
    std::vector<std::shared_ptr<ASTNode>> children;
    std::shared_ptr<SymbolInfo> symbolInfo;

public:
    ASTNode(NodeType type, const std::string& value, int line, int column);
    virtual ~ASTNode() = default;
    
    // Getters
    NodeType getType() const { return type; }
    const std::string& getValue() const { return value; }
    DataType getDataType() const { return dataType; }
    int getLine() const { return line; }
    int getColumn() const { return column; }
    const std::vector<std::shared_ptr<ASTNode>>& getChildren() const { return children; }
    std::shared_ptr<SymbolInfo> getSymbolInfo() const { return symbolInfo; }
    
    // Setters
    void setDataType(DataType type) { dataType = type; }
    void setSymbolInfo(std::shared_ptr<SymbolInfo> info) { symbolInfo = info; }
    
    // Tree manipulation
    void addChild(std::shared_ptr<ASTNode> child);
    void addChild(std::unique_ptr<ASTNode> child);
    std::shared_ptr<ASTNode> getChild(int index) const;
    int getChildCount() const { return children.size(); }
    
    // Utility methods
    bool isExpression() const;
    bool isStatement() const;
    bool isDeclaration() const;
    
    // Display and debugging
    virtual std::string toString() const;
    void printTree(int depth = 0) const;
    
    // Static utility methods
    static std::string nodeTypeToString(NodeType type);
    static std::string dataTypeToString(DataType type);
};

// Specific AST node types
class ProgramNode : public ASTNode {
public:
    ProgramNode(int line, int column);
    void addDeclaration(std::shared_ptr<ASTNode> declaration);
};

class FunctionDeclarationNode : public ASTNode {
private:
    std::string functionName;
    DataType returnType;
    std::vector<std::shared_ptr<ASTNode>> parameters;

public:
    FunctionDeclarationNode(const std::string& name, DataType returnType, int line, int column);
    
    const std::string& getFunctionName() const { return functionName; }
    DataType getReturnType() const { return returnType; }
    const std::vector<std::shared_ptr<ASTNode>>& getParameters() const { return parameters; }
    
    void addParameter(std::shared_ptr<ASTNode> parameter);
};

class VariableDeclarationNode : public ASTNode {
private:
    std::string variableName;
    DataType variableType;
    std::shared_ptr<ASTNode> initializer;

public:
    VariableDeclarationNode(const std::string& name, DataType type, 
                           std::shared_ptr<ASTNode> initializer, int line, int column);
    
    const std::string& getVariableName() const { return variableName; }
    DataType getVariableType() const { return variableType; }
    std::shared_ptr<ASTNode> getInitializer() const { return initializer; }
};

class BinaryExpressionNode : public ASTNode {
private:
    std::string op;
    std::shared_ptr<ASTNode> left;
    std::shared_ptr<ASTNode> right;

public:
    BinaryExpressionNode(const std::string& op, std::shared_ptr<ASTNode> left, 
                        std::shared_ptr<ASTNode> right, int line, int column);
    
    const std::string& getOperator() const { return op; }
    std::shared_ptr<ASTNode> getLeft() const { return left; }
    std::shared_ptr<ASTNode> getRight() const { return right; }
};

class IdentifierExpressionNode : public ASTNode {
private:
    std::string identifierName;

public:
    IdentifierExpressionNode(const std::string& name, int line, int column);
    const std::string& getIdentifierName() const { return identifierName; }
};

class LiteralExpressionNode : public ASTNode {
private:
    std::string literalValue;

public:
    LiteralExpressionNode(const std::string& value, DataType type, int line, int column);
    const std::string& getLiteralValue() const { return literalValue; }
};

#endif // AST_NODE_H
