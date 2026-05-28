#include "semantic_analyzer.h"
#include <sstream>
#include <iostream>

// SemanticError implementation
SemanticError::SemanticError(SemanticErrorType type, const std::string& message, int line, int column, const std::string& severity)
    : errorType(type), message(message), line(line), column(column), severity(severity) {}

std::string SemanticError::toString() const {
    std::ostringstream oss;
    oss << "[" << severity << "] " << errorTypeToString(errorType) << ": " << message;
    oss << " (Line " << line << ", Column " << column << ")";
    return oss.str();
}

void SemanticError::print() const {
    std::cout << toString() << std::endl;
}

std::string SemanticError::errorTypeToString(SemanticErrorType type) {
    switch (type) {
        case SemanticErrorType::UNDECLARED_VARIABLE: return "Undeclared Variable";
        case SemanticErrorType::UNDECLARED_FUNCTION: return "Undeclared Function";
        case SemanticErrorType::TYPE_MISMATCH: return "Type Mismatch";
        case SemanticErrorType::INVALID_ASSIGNMENT: return "Invalid Assignment";
        case SemanticErrorType::INVALID_OPERATION: return "Invalid Operation";
        case SemanticErrorType::FUNCTION_CALL_MISMATCH: return "Function Call Mismatch";
        case SemanticErrorType::RETURN_TYPE_MISMATCH: return "Return Type Mismatch";
        case SemanticErrorType::VOID_USAGE: return "Void Usage";
        case SemanticErrorType::REDECLARATION: return "Redeclaration";
        case SemanticErrorType::UNINITIALIZED_VARIABLE: return "Uninitialized Variable";
        case SemanticErrorType::BREAK_CONTINUE_OUTSIDE_LOOP: return "Break/Continue Outside Loop";
        case SemanticErrorType::INVALID_CAST: return "Invalid Cast";
        case SemanticErrorType::ARRAY_INDEX_ERROR: return "Array Index Error";
        case SemanticErrorType::UNKNOWN: return "Unknown";
        default: return "Unknown";
    }
}

// SemanticAnalyzer implementation
SemanticAnalyzer::SemanticAnalyzer(std::shared_ptr<SymbolTable> symbolTable)
    : symbolTable(symbolTable), currentReturnType(DataType::VOID), inLoop(false), hasReturn(false) {}

bool SemanticAnalyzer::analyze(std::shared_ptr<ASTNode> ast) {
    if (!ast) {
        return false;
    }
    
    errors.clear();
    warnings.clear();
    
    visitProgram(ast);
    
    return !hasErrors();
}

void SemanticAnalyzer::addError(SemanticErrorType type, const std::string& message, int line, int column) {
    errors.emplace_back(type, message, line, column, "error");
}

void SemanticAnalyzer::addWarning(SemanticErrorType type, const std::string& message, int line, int column) {
    warnings.emplace_back(type, message, line, column, "warning");
}

DataType SemanticAnalyzer::getExpressionType(std::shared_ptr<ASTNode> node) {
    if (!node) {
        return DataType::UNKNOWN;
    }
    
    switch (node->getType()) {
        case NodeType::LITERAL_EXPRESSION: {
            auto literal = std::dynamic_pointer_cast<LiteralExpressionNode>(node);
            return literal ? literal->getDataType() : DataType::UNKNOWN;
        }
        
        case NodeType::IDENTIFIER_EXPRESSION: {
            auto identifier = std::dynamic_pointer_cast<IdentifierExpressionNode>(node);
            if (identifier) {
                auto symbol = lookupSymbol(identifier->getIdentifierName());
                return symbol ? symbol->getDataType() : DataType::UNKNOWN;
            }
            return DataType::UNKNOWN;
        }
        
        case NodeType::BINARY_EXPRESSION: {
            auto binary = std::dynamic_pointer_cast<BinaryExpressionNode>(node);
            if (binary) {
                DataType leftType = getExpressionType(binary->getLeft());
                DataType rightType = getExpressionType(binary->getRight());
                return getResultType(leftType, rightType, binary->getOperator());
            }
            return DataType::UNKNOWN;
        }
        
        case NodeType::FUNCTION_CALL: {
            auto symbol = lookupSymbol(node->getValue());
            return symbol ? symbol->getReturnType() : DataType::UNKNOWN;
        }
        
        default:
            return DataType::UNKNOWN;
    }
}

bool SemanticAnalyzer::isCompatibleTypes(DataType source, DataType target) {
    if (source == target) {
        return true;
    }
    
    // Allow implicit conversion between numeric types
    if (isNumericType(source) && isNumericType(target)) {
        return true;
    }
    
    return false;
}

bool SemanticAnalyzer::canImplicitCast(DataType from, DataType to) {
    if (from == to) {
        return true;
    }
    
    // Allow numeric conversions
    if (isNumericType(from) && isNumericType(to)) {
        return true;
    }
    
    // Allow char to int conversion
    if (from == DataType::CHAR && to == DataType::INT) {
        return true;
    }
    
    return false;
}

DataType SemanticAnalyzer::getResultType(DataType left, DataType right, const std::string& op) {
    // Handle assignment operators
    if (op == "=") {
        return left;
    }
    
    // Handle comparison operators (always result in int for boolean-like behavior)
    if (op == "==" || op == "!=" || op == "<" || op == ">" || op == "<=" || op == ">=") {
        return DataType::INT;
    }
    
    // Handle arithmetic operators
    if (op == "+" || op == "-" || op == "*" || op == "/") {
        if (!isNumericType(left) || !isNumericType(right)) {
            return DataType::UNKNOWN;
        }
        
        // If either is float, result is float
        if (left == DataType::FLOAT || right == DataType::FLOAT) {
            return DataType::FLOAT;
        }
        
        return DataType::INT;
    }
    
    return DataType::UNKNOWN;
}

bool SemanticAnalyzer::validateBinaryExpression(std::shared_ptr<ASTNode> node) {
    auto binary = std::dynamic_pointer_cast<BinaryExpressionNode>(node);
    if (!binary) {
        return false;
    }
    
    DataType leftType = getExpressionType(binary->getLeft());
    DataType rightType = getExpressionType(binary->getRight());
    
    // Check for void types in expressions
    if (leftType == DataType::VOID) {
        addError(SemanticErrorType::VOID_USAGE, "Cannot use void type in expression", 
                binary->getLeft()->getLine(), binary->getLeft()->getColumn());
        return false;
    }
    
    if (rightType == DataType::VOID) {
        addError(SemanticErrorType::VOID_USAGE, "Cannot use void type in expression", 
                binary->getRight()->getLine(), binary->getRight()->getColumn());
        return false;
    }
    
    // Check type compatibility
    if (!isCompatibleTypes(leftType, rightType)) {
        addError(SemanticErrorType::TYPE_MISMATCH, 
                "Type mismatch in binary expression: " + dataTypeToString(leftType) + 
                " " + binary->getOperator() + " " + dataTypeToString(rightType),
                node->getLine(), node->getColumn());
        return false;
    }
    
    // Set the result type
    DataType resultType = getResultType(leftType, rightType, binary->getOperator());
    node->setDataType(resultType);
    
    return true;
}

bool SemanticAnalyzer::validateUnaryExpression(std::shared_ptr<ASTNode> node) {
    // For simplicity, we'll assume unary expressions are valid
    // In a more complete implementation, we'd check operand types
    return true;
}

bool SemanticAnalyzer::validateAssignment(std::shared_ptr<ASTNode> node) {
    auto binary = std::dynamic_pointer_cast<BinaryExpressionNode>(node);
    if (!binary || binary->getOperator() != "=") {
        return false;
    }
    
    // Check if left side is a valid lvalue
    if (!isLValue(binary->getLeft())) {
        addError(SemanticErrorType::INVALID_ASSIGNMENT, "Invalid assignment target", 
                node->getLine(), node->getColumn());
        return false;
    }
    
    DataType leftType = getExpressionType(binary->getLeft());
    DataType rightType = getExpressionType(binary->getRight());
    
    // Check for void assignment
    if (rightType == DataType::VOID) {
        addError(SemanticErrorType::VOID_USAGE, "Cannot assign void value", 
                node->getLine(), node->getColumn());
        return false;
    }
    
    // Check type compatibility
    if (!isCompatibleTypes(rightType, leftType)) {
        addError(SemanticErrorType::TYPE_MISMATCH, 
                "Cannot assign " + dataTypeToString(rightType) + " to " + dataTypeToString(leftType),
                node->getLine(), node->getColumn());
        return false;
    }
    
    // Mark variable as initialized
    if (binary->getLeft()->getType() == NodeType::IDENTIFIER_EXPRESSION) {
        auto identifier = std::dynamic_pointer_cast<IdentifierExpressionNode>(binary->getLeft());
        if (identifier) {
            symbolTable->markAsInitialized(identifier->getIdentifierName());
        }
    }
    
    node->setDataType(leftType);
    return true;
}

bool SemanticAnalyzer::validateFunctionCall(std::shared_ptr<ASTNode> node) {
    if (node->getType() != NodeType::FUNCTION_CALL) {
        return false;
    }
    
    std::string functionName = node->getValue();
    auto symbol = lookupSymbol(functionName);
    
    if (!symbol) {
        addError(SemanticErrorType::UNDECLARED_FUNCTION, "Undeclared function: " + functionName,
                node->getLine(), node->getColumn());
        return false;
    }
    
    if (!symbol->isFunction()) {
        addError(SemanticErrorType::FUNCTION_CALL_MISMATCH, "'" + functionName + "' is not a function",
                node->getLine(), node->getColumn());
        return false;
    }
    
    // Check argument count and types
    std::vector<DataType> argTypes = getArgumentTypes(node);
    if (!checkFunctionParameters(functionName, argTypes)) {
        return false;
    }
    
    node->setDataType(symbol->getReturnType());
    return true;
}

bool SemanticAnalyzer::validateVariableDeclaration(std::shared_ptr<ASTNode> node) {
    auto varDecl = std::dynamic_pointer_cast<VariableDeclarationNode>(node);
    if (!varDecl) {
        return false;
    }
    
    // Check if variable is already declared in current scope
    if (symbolTable->isDeclaredInCurrentScope(varDecl->getVariableName())) {
        addError(SemanticErrorType::REDECLARATION, "Variable already declared: " + varDecl->getVariableName(),
                node->getLine(), node->getColumn());
        return false;
    }
    
    // Validate initializer if present
    auto initializer = varDecl->getInitializer();
    if (initializer) {
        DataType initType = getExpressionType(initializer);
        
        if (initType == DataType::VOID) {
            addError(SemanticErrorType::VOID_USAGE, "Cannot initialize variable with void value",
                    node->getLine(), node->getColumn());
            return false;
        }
        
        if (!isCompatibleTypes(initType, varDecl->getVariableType())) {
            addError(SemanticErrorType::TYPE_MISMATCH, 
                    "Cannot initialize " + dataTypeToString(varDecl->getVariableType()) + 
                    " with " + dataTypeToString(initType),
                    node->getLine(), node->getColumn());
            return false;
        }
    }
    
    return true;
}

bool SemanticAnalyzer::validateReturnStatement(std::shared_ptr<ASTNode> node) {
    if (currentReturnType == DataType::VOID) {
        // If function returns void, there should be no return value
        if (node->getChildCount() > 0) {
            addError(SemanticErrorType::RETURN_TYPE_MISMATCH, "Void function cannot return a value",
                    node->getLine(), node->getColumn());
            return false;
        }
    } else {
        // If function returns non-void, there must be a return value
        if (node->getChildCount() == 0) {
            addError(SemanticErrorType::RETURN_TYPE_MISMATCH, "Function must return a value",
                    node->getLine(), node->getColumn());
            return false;
        }
        
        // Check return value type
        DataType returnValueType = getExpressionType(node->getChild(0));
        if (returnValueType == DataType::VOID) {
            addError(SemanticErrorType::VOID_USAGE, "Cannot return void value",
                    node->getLine(), node->getColumn());
            return false;
        }
        
        if (!isCompatibleTypes(returnValueType, currentReturnType)) {
            addError(SemanticErrorType::RETURN_TYPE_MISMATCH, 
                    "Cannot return " + dataTypeToString(returnValueType) + 
                    " from function returning " + dataTypeToString(currentReturnType),
                    node->getLine(), node->getColumn());
            return false;
        }
    }
    
    hasReturn = true;
    return true;
}

bool SemanticAnalyzer::validateIfStatement(std::shared_ptr<ASTNode> node) {
    if (node->getChildCount() < 2) {
        return false;
    }
    
    // Validate condition
    auto condition = node->getChild(0);
    DataType conditionType = getExpressionType(condition);
    
    if (conditionType == DataType::VOID) {
        addError(SemanticErrorType::VOID_USAGE, "Condition cannot be void",
                condition->getLine(), condition->getColumn());
        return false;
    }
    
    // Validate then branch
    auto thenBranch = node->getChild(1);
    visitStatement(thenBranch);
    
    // Validate else branch if present
    if (node->getChildCount() > 2) {
        auto elseBranch = node->getChild(2);
        visitStatement(elseBranch);
    }
    
    return true;
}

bool SemanticAnalyzer::validateWhileStatement(std::shared_ptr<ASTNode> node) {
    if (node->getChildCount() < 2) {
        return false;
    }
    
    // Validate condition
    auto condition = node->getChild(0);
    DataType conditionType = getExpressionType(condition);
    
    if (conditionType == DataType::VOID) {
        addError(SemanticErrorType::VOID_USAGE, "Condition cannot be void",
                condition->getLine(), condition->getColumn());
        return false;
    }
    
    // Validate body
    bool wasInLoop = inLoop;
    inLoop = true;
    auto body = node->getChild(1);
    visitStatement(body);
    inLoop = wasInLoop;
    
    return true;
}

bool SemanticAnalyzer::validateForStatement(std::shared_ptr<ASTNode> node) {
    if (node->getChildCount() < 4) {
        return false;
    }
    
    // Validate initialization
    if (node->getChild(0)) {
        visitExpression(node->getChild(0));
    }
    
    // Validate condition
    if (node->getChild(1)) {
        auto condition = node->getChild(1);
        DataType conditionType = getExpressionType(condition);
        if (conditionType == DataType::VOID) {
            addError(SemanticErrorType::VOID_USAGE, "Condition cannot be void",
                    condition->getLine(), condition->getColumn());
            return false;
        }
    }
    
    // Validate increment
    if (node->getChild(2)) {
        visitExpression(node->getChild(2));
    }
    
    // Validate body
    bool wasInLoop = inLoop;
    inLoop = true;
    auto body = node->getChild(3);
    visitStatement(body);
    inLoop = wasInLoop;
    
    return true;
}

std::shared_ptr<SymbolInfo> SemanticAnalyzer::lookupSymbol(const std::string& name) {
    return symbolTable ? symbolTable->lookupInAllScopes(name) : nullptr;
}

bool SemanticAnalyzer::isLValue(std::shared_ptr<ASTNode> node) {
    return node && node->getType() == NodeType::IDENTIFIER_EXPRESSION;
}

std::vector<DataType> SemanticAnalyzer::getArgumentTypes(std::shared_ptr<ASTNode> functionCall) {
    std::vector<DataType> argTypes;
    
    for (const auto& child : functionCall->getChildren()) {
        argTypes.push_back(getExpressionType(child));
    }
    
    return argTypes;
}

bool SemanticAnalyzer::checkFunctionParameters(const std::string& functionName, const std::vector<DataType>& argTypes) {
    auto symbol = lookupSymbol(functionName);
    if (!symbol || !symbol->isFunction()) {
        return false;
    }
    
    const auto& expectedParams = symbol->getParameterTypes();
    
    if (argTypes.size() != expectedParams.size()) {
        addError(SemanticErrorType::FUNCTION_CALL_MISMATCH,
                "Function " + functionName + " expects " + std::to_string(expectedParams.size()) + 
                " arguments, but " + std::to_string(argTypes.size()) + " were provided",
                0, 0);
        return false;
    }
    
    for (size_t i = 0; i < argTypes.size(); ++i) {
        if (!isCompatibleTypes(argTypes[i], expectedParams[i])) {
            addError(SemanticErrorType::FUNCTION_CALL_MISMATCH,
                    "Argument " + std::to_string(i + 1) + " type mismatch: expected " + 
                    dataTypeToString(expectedParams[i]) + ", got " + dataTypeToString(argTypes[i]),
                    0, 0);
            return false;
        }
    }
    
    return true;
}

void SemanticAnalyzer::visitProgram(std::shared_ptr<ASTNode> node) {
    for (const auto& child : node->getChildren()) {
        if (child->getType() == NodeType::FUNCTION_DECLARATION) {
            visitFunctionDeclaration(child);
        } else if (child->getType() == NodeType::VARIABLE_DECLARATION) {
            visitVariableDeclaration(child);
        }
    }
}

void SemanticAnalyzer::visitFunctionDeclaration(std::shared_ptr<ASTNode> node) {
    auto funcDecl = std::dynamic_pointer_cast<FunctionDeclarationNode>(node);
    if (!funcDecl) {
        return;
    }
    
    // Set current function context
    currentFunction = funcDecl->getFunctionName();
    currentReturnType = funcDecl->getReturnType();
    hasReturn = false;
    
    // Visit function body
    if (node->getChildCount() > 0) {
        visitStatement(node->getChild(0));
    }
    
    // Check for missing return in non-void functions
    if (currentReturnType != DataType::VOID && !hasReturn) {
        addWarning(SemanticErrorType::RETURN_TYPE_MISMATCH, "Function may not return a value",
                  node->getLine(), node->getColumn());
    }
    
    // Reset context
    currentFunction = "";
    currentReturnType = DataType::VOID;
}

void SemanticAnalyzer::visitVariableDeclaration(std::shared_ptr<ASTNode> node) {
    validateVariableDeclaration(node);
}

void SemanticAnalyzer::visitStatement(std::shared_ptr<ASTNode> node) {
    switch (node->getType()) {
        case NodeType::EXPRESSION_STATEMENT:
            if (node->getChildCount() > 0) {
                visitExpression(node->getChild(0));
            }
            break;
        case NodeType::COMPOUND_STATEMENT:
            visitCompoundStatement(node);
            break;
        case NodeType::IF_STATEMENT:
            visitIfStatement(node);
            break;
        case NodeType::WHILE_STATEMENT:
            visitWhileStatement(node);
            break;
        case NodeType::FOR_STATEMENT:
            visitForStatement(node);
            break;
        case NodeType::RETURN_STATEMENT:
            visitReturnStatement(node);
            break;
        case NodeType::VARIABLE_DECLARATION:
            visitVariableDeclaration(node);
            break;
        default:
            break;
    }
}

void SemanticAnalyzer::visitExpression(std::shared_ptr<ASTNode> node) {
    switch (node->getType()) {
        case NodeType::BINARY_EXPRESSION:
            visitBinaryExpression(node);
            break;
        case NodeType::ASSIGNMENT_EXPRESSION:
            visitAssignmentExpression(node);
            break;
        case NodeType::FUNCTION_CALL:
            visitFunctionCall(node);
            break;
        case NodeType::IDENTIFIER_EXPRESSION:
            visitIdentifierExpression(node);
            break;
        case NodeType::LITERAL_EXPRESSION:
            visitLiteralExpression(node);
            break;
        default:
            break;
    }
}

void SemanticAnalyzer::visitBinaryExpression(std::shared_ptr<ASTNode> node) {
    validateBinaryExpression(node);
}

void SemanticAnalyzer::visitUnaryExpression(std::shared_ptr<ASTNode> node) {
    validateUnaryExpression(node);
}

void SemanticAnalyzer::visitAssignmentExpression(std::shared_ptr<ASTNode> node) {
    validateAssignment(node);
}

void SemanticAnalyzer::visitFunctionCall(std::shared_ptr<ASTNode> node) {
    validateFunctionCall(node);
    
    // Visit arguments
    for (const auto& child : node->getChildren()) {
        visitExpression(child);
    }
}

void SemanticAnalyzer::visitIdentifierExpression(std::shared_ptr<ASTNode> node) {
    auto identifier = std::dynamic_pointer_cast<IdentifierExpressionNode>(node);
    if (identifier) {
        auto symbol = lookupSymbol(identifier->getIdentifierName());
        if (!symbol) {
            addError(SemanticErrorType::UNDECLARED_VARIABLE, 
                    "Undeclared variable: " + identifier->getIdentifierName(),
                    node->getLine(), node->getColumn());
        } else {
            node->setDataType(symbol->getDataType());
            symbolTable->markAsUsed(identifier->getIdentifierName());
        }
    }
}

void SemanticAnalyzer::visitLiteralExpression(std::shared_ptr<ASTNode> node) {
    auto literal = std::dynamic_pointer_cast<LiteralExpressionNode>(node);
    if (literal) {
        node->setDataType(literal->getDataType());
    }
}

void SemanticAnalyzer::visitIfStatement(std::shared_ptr<ASTNode> node) {
    validateIfStatement(node);
}

void SemanticAnalyzer::visitWhileStatement(std::shared_ptr<ASTNode> node) {
    validateWhileStatement(node);
}

void SemanticAnalyzer::visitForStatement(std::shared_ptr<ASTNode> node) {
    validateForStatement(node);
}

void SemanticAnalyzer::visitReturnStatement(std::shared_ptr<ASTNode> node) {
    validateReturnStatement(node);
}

void SemanticAnalyzer::visitCompoundStatement(std::shared_ptr<ASTNode> node) {
    for (const auto& child : node->getChildren()) {
        visitStatement(child);
    }
}

void SemanticAnalyzer::printErrors() const {
    std::cout << "=== Semantic Analysis Errors ===\n";
    if (errors.empty()) {
        std::cout << "No semantic errors\n";
    } else {
        for (const auto& error : errors) {
            error.print();
        }
    }
}

void SemanticAnalyzer::printWarnings() const {
    std::cout << "=== Semantic Analysis Warnings ===\n";
    if (warnings.empty()) {
        std::cout << "No semantic warnings\n";
    } else {
        for (const auto& warning : warnings) {
            warning.print();
        }
    }
}

void SemanticAnalyzer::printAllIssues() const {
    printErrors();
    printWarnings();
}

void SemanticAnalyzer::clearIssues() {
    errors.clear();
    warnings.clear();
}

bool SemanticAnalyzer::isNumericType(DataType type) {
    return type == DataType::INT || type == DataType::FLOAT;
}

bool SemanticAnalyzer::isIntegralType(DataType type) {
    return type == DataType::INT || type == DataType::CHAR;
}

bool SemanticAnalyzer::isVoidType(DataType type) {
    return type == DataType::VOID;
}

std::string SemanticAnalyzer::dataTypeToString(DataType type) {
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
