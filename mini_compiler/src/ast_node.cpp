#include "ast_node.h"
#include <sstream>
#include <iomanip>

ASTNode::ASTNode(NodeType type, const std::string& value, int line, int column)
    : type(type), value(value), dataType(DataType::UNKNOWN), line(line), column(column) {}

void ASTNode::addChild(std::shared_ptr<ASTNode> child) {
    if (child) {
        children.push_back(child);
    }
}

void ASTNode::addChild(std::unique_ptr<ASTNode> child) {
    if (child) {
        children.push_back(std::move(child));
    }
}

std::shared_ptr<ASTNode> ASTNode::getChild(int index) const {
    if (index >= 0 && index < static_cast<int>(children.size())) {
        return children[index];
    }
    return nullptr;
}

bool ASTNode::isExpression() const {
    return type == NodeType::BINARY_EXPRESSION ||
           type == NodeType::UNARY_EXPRESSION ||
           type == NodeType::ASSIGNMENT_EXPRESSION ||
           type == NodeType::FUNCTION_CALL ||
           type == NodeType::IDENTIFIER_EXPRESSION ||
           type == NodeType::LITERAL_EXPRESSION;
}

bool ASTNode::isStatement() const {
    return type == NodeType::EXPRESSION_STATEMENT ||
           type == NodeType::IF_STATEMENT ||
           type == NodeType::WHILE_STATEMENT ||
           type == NodeType::FOR_STATEMENT ||
           type == NodeType::RETURN_STATEMENT ||
           type == NodeType::COMPOUND_STATEMENT;
}

bool ASTNode::isDeclaration() const {
    return type == NodeType::VARIABLE_DECLARATION ||
           type == NodeType::VARIABLE_DECLARATION_LIST ||
           type == NodeType::FUNCTION_DECLARATION ||
           type == NodeType::FUNCTION_DEFINITION ||
           type == NodeType::PARAMETER_LIST ||
           type == NodeType::PARAMETER;
}

std::string ASTNode::toString() const {
    std::ostringstream oss;
    oss << nodeTypeToString(type);
    if (!value.empty()) {
        oss << "[" << value << "]";
    }
    if (dataType != DataType::UNKNOWN) {
        oss << ":" << dataTypeToString(dataType);
    }
    oss << " (L" << line << ":C" << column << ")";
    return oss.str();
}

void ASTNode::printTree(int depth) const {
    std::cout << std::string(depth * 2, ' ') << toString() << std::endl;
    for (const auto& child : children) {
        child->printTree(depth + 1);
    }
}

std::string ASTNode::nodeTypeToString(NodeType type) {
    switch (type) {
        // Program structure
        case NodeType::PROGRAM: return "Program";
        case NodeType::FUNCTION_DECLARATION: return "FunctionDecl";
        case NodeType::FUNCTION_DEFINITION: return "FunctionDef";
        case NodeType::PARAMETER_LIST: return "ParamList";
        case NodeType::PARAMETER: return "Parameter";
        case NodeType::BLOCK: return "Block";
        
        // Declarations
        case NodeType::VARIABLE_DECLARATION: return "VarDecl";
        case NodeType::VARIABLE_DECLARATION_LIST: return "VarDeclList";
        
        // Statements
        case NodeType::EXPRESSION_STATEMENT: return "ExprStmt";
        case NodeType::IF_STATEMENT: return "IfStmt";
        case NodeType::WHILE_STATEMENT: return "WhileStmt";
        case NodeType::FOR_STATEMENT: return "ForStmt";
        case NodeType::RETURN_STATEMENT: return "ReturnStmt";
        case NodeType::COMPOUND_STATEMENT: return "CompoundStmt";
        
        // Expressions
        case NodeType::BINARY_EXPRESSION: return "BinaryExpr";
        case NodeType::UNARY_EXPRESSION: return "UnaryExpr";
        case NodeType::ASSIGNMENT_EXPRESSION: return "AssignExpr";
        case NodeType::FUNCTION_CALL: return "FunctionCall";
        case NodeType::IDENTIFIER_EXPRESSION: return "Identifier";
        case NodeType::LITERAL_EXPRESSION: return "Literal";
        
        // Types
        case NodeType::TYPE_SPECIFIER: return "Type";
        
        // Unknown
        case NodeType::UNKNOWN: return "Unknown";
        
        default: return "Unknown";
    }
}

std::string ASTNode::dataTypeToString(DataType type) {
    switch (type) {
        case DataType::VOID: return "void";
        case DataType::INT: return "int";
        case DataType::FLOAT: return "float";
        case DataType::CHAR: return "char";
        case DataType::STRING: return "string";
        case DataType::UNKNOWN: return "unknown";
        default: return "unknown";
    }
}

// ProgramNode implementation
ProgramNode::ProgramNode(int line, int column) 
    : ASTNode(NodeType::PROGRAM, "program", line, column) {}

void ProgramNode::addDeclaration(std::shared_ptr<ASTNode> declaration) {
    addChild(declaration);
}

// FunctionDeclarationNode implementation
FunctionDeclarationNode::FunctionDeclarationNode(const std::string& name, DataType returnType, int line, int column)
    : ASTNode(NodeType::FUNCTION_DECLARATION, name, line, column), functionName(name), returnType(returnType) {
    setDataType(returnType);
}

void FunctionDeclarationNode::addParameter(std::shared_ptr<ASTNode> parameter) {
    parameters.push_back(parameter);
    addChild(parameter);
}

// VariableDeclarationNode implementation
VariableDeclarationNode::VariableDeclarationNode(const std::string& name, DataType type, 
                                               std::shared_ptr<ASTNode> initializer, int line, int column)
    : ASTNode(NodeType::VARIABLE_DECLARATION, name, line, column), 
      variableName(name), variableType(type), initializer(initializer) {
    setDataType(type);
    if (initializer) {
        addChild(initializer);
    }
}

// BinaryExpressionNode implementation
BinaryExpressionNode::BinaryExpressionNode(const std::string& op, std::shared_ptr<ASTNode> left, 
                                          std::shared_ptr<ASTNode> right, int line, int column)
    : ASTNode(NodeType::BINARY_EXPRESSION, op, line, column), op(op), left(left), right(right) {
    if (left) addChild(left);
    if (right) addChild(right);
}

// IdentifierExpressionNode implementation
IdentifierExpressionNode::IdentifierExpressionNode(const std::string& name, int line, int column)
    : ASTNode(NodeType::IDENTIFIER_EXPRESSION, name, line, column), identifierName(name) {}

// LiteralExpressionNode implementation
LiteralExpressionNode::LiteralExpressionNode(const std::string& value, DataType type, int line, int column)
    : ASTNode(NodeType::LITERAL_EXPRESSION, value, line, column), literalValue(value) {
    setDataType(type);
}
