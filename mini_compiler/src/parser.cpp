#include "parser.h"
#include <iostream>
#include <sstream>

Parser::Parser(std::vector<std::unique_ptr<Token>> tokens) 
    : tokens(std::move(tokens)), current(0), inFunction(false) {
    symbolTable = std::make_shared<SymbolTable>();
}

std::unique_ptr<Token> Parser::getCurrentToken() {
    if (current < static_cast<int>(tokens.size())) {
        return std::make_unique<Token>(*tokens[current]);
    }
    return nullptr;
}

std::unique_ptr<Token> Parser::peekToken(int offset) {
    int peekIndex = current + offset;
    if (peekIndex < static_cast<int>(tokens.size())) {
        return std::make_unique<Token>(*tokens[peekIndex]);
    }
    return nullptr;
}

std::unique_ptr<Token> Parser::advance() {
    if (!isAtEnd()) {
        current++;
    }
    return getCurrentToken();
}

bool Parser::match(TokenType type) {
    auto token = getCurrentToken();
    return token && token->getType() == type;
}

bool Parser::consume(TokenType type, const std::string& errorMessage) {
    if (match(type)) {
        advance();
        return true;
    }
    
    auto token = getCurrentToken();
    if (token) {
        addErrorAtToken(errorMessage, *token);
    } else {
        addError(errorMessage + " (unexpected end of file)");
    }
    return false;
}

bool Parser::isAtEnd() {
    auto token = getCurrentToken();
    return !token || token->getType() == TokenType::END_OF_FILE;
}

void Parser::addError(const std::string& message) {
    errors.push_back(message);
}

void Parser::addWarning(const std::string& message) {
    warnings.push_back(message);
}

void Parser::addErrorAtToken(const std::string& message, const Token& token) {
    std::ostringstream oss;
    oss << "Line " << token.getLine() << ", Column " << token.getColumn() << ": " << message;
    errors.push_back(oss.str());
}

std::shared_ptr<ASTNode> Parser::parse() {
    auto program = parseProgram();
    
    if (!isAtEnd()) {
        addWarning("Extra tokens after complete program");
    }
    
    // Validate symbol usage
    symbolTable->validateSymbolUsage();
    
    // Add symbol table errors to parser errors
    for (const auto& error : symbolTable->getErrors()) {
        addError(error);
    }
    
    // Add symbol table warnings to parser warnings
    for (const auto& warning : symbolTable->getWarnings()) {
        addWarning(warning);
    }
    
    return program;
}

std::shared_ptr<ASTNode> Parser::parseProgram() {
    auto program = std::make_shared<ProgramNode>(1, 1);
    
    while (!isAtEnd()) {
        auto declaration = parseDeclaration();
        if (declaration) {
            program->addDeclaration(declaration);
        } else {
            synchronize();
        }
    }
    
    return program;
}

std::shared_ptr<ASTNode> Parser::parseDeclaration() {
    // Try to parse function declaration first
    if (match(TokenType::KEYWORD_INT) || match(TokenType::KEYWORD_FLOAT) || 
        match(TokenType::KEYWORD_CHAR) || match(TokenType::KEYWORD_VOID)) {
        
        auto typeToken = getCurrentToken();
        auto dataType = tokenTypeToDataType(typeToken->getType());
        advance();
        
        if (match(TokenType::IDENTIFIER)) {
            auto identifierToken = getCurrentToken();
            std::string name = identifierToken->getValue();
            advance();
            
            // Check if it's a function
            if (match(TokenType::LEFT_PAREN)) {
                advance();
                return parseFunctionDefinition(name, dataType, identifierToken->getLine(), identifierToken->getColumn());
            }
            // It's a variable declaration
            else {
                return parseVariableDeclaration(name, dataType, identifierToken->getLine(), identifierToken->getColumn());
            }
        }
    }
    
    addError("Expected declaration");
    return nullptr;
}

std::shared_ptr<ASTNode> Parser::parseVariableDeclaration(const std::string& name, DataType dataType, int line, int column) {
    // Check if already declared in current scope
    if (symbolTable->isDeclaredInCurrentScope(name)) {
        addError("Variable '" + name + "' already declared in current scope");
    }
    
    std::shared_ptr<ASTNode> initializer = nullptr;
    
    // Check for initialization
    if (match(TokenType::ASSIGN)) {
        advance();
        initializer = parseExpression();
    }
    
    // Expect semicolon
    if (!consume(TokenType::SEMICOLON, "Expected ';' after variable declaration")) {
        return nullptr;
    }
    
    // Add to symbol table
    if (!declareSymbol(name, SymbolKind::VARIABLE, dataType)) {
        return nullptr;
    }
    
    // Mark as initialized if there's an initializer
    if (initializer) {
        symbolTable->markAsInitialized(name);
    }
    
    return std::make_shared<VariableDeclarationNode>(name, dataType, initializer, line, column);
}

std::shared_ptr<ASTNode> Parser::parseFunctionDefinition(const std::string& name, DataType returnType, int line, int column) {
    // Add function to symbol table
    if (!declareSymbol(name, SymbolKind::FUNCTION, returnType)) {
        return nullptr;
    }
    
    auto functionNode = std::make_shared<FunctionDeclarationNode>(name, returnType, line, column);
    
    // Enter function scope
    enterScope("function_" + name);
    inFunction = true;
    currentFunction = name;
    
    // Parse parameters
    auto parameterList = parseParameterList();
    if (parameterList) {
        functionNode->addParameter(parameterList);
    }
    
    // Parse function body
    if (!consume(TokenType::LEFT_BRACE, "Expected '{' after function parameter list")) {
        exitScope();
        return nullptr;
    }
    
    auto body = parseCompoundStatement();
    if (!body) {
        exitScope();
        return nullptr;
    }
    
    functionNode->addChild(body);
    
    // Exit function scope
    exitScope();
    inFunction = false;
    currentFunction = "";
    
    return functionNode;
}

std::shared_ptr<ASTNode> Parser::parseParameterList() {
    auto paramList = std::make_shared<ASTNode>(NodeType::PARAMETER_LIST, "parameters", 0, 0);
    
    // Handle empty parameter list
    if (match(TokenType::RIGHT_PAREN)) {
        advance();
        return paramList;
    }
    
    // Parse first parameter
    auto param = parseParameter();
    if (param) {
        paramList->addChild(param);
    }
    
    // Parse remaining parameters
    while (match(TokenType::COMMA)) {
        advance();
        param = parseParameter();
        if (param) {
            paramList->addChild(param);
        }
    }
    
    // Expect closing parenthesis
    consume(TokenType::RIGHT_PAREN, "Expected ')' after parameter list");
    
    return paramList;
}

std::shared_ptr<ASTNode> Parser::parseParameter() {
    // Parse parameter type
    DataType paramType = DataType::UNKNOWN;
    if (match(TokenType::KEYWORD_INT)) {
        paramType = DataType::INT;
        advance();
    } else if (match(TokenType::KEYWORD_FLOAT)) {
        paramType = DataType::FLOAT;
        advance();
    } else if (match(TokenType::KEYWORD_CHAR)) {
        paramType = DataType::CHAR;
        advance();
    } else if (match(TokenType::KEYWORD_VOID)) {
        addError("Parameter cannot have void type");
        advance();
    } else {
        addError("Expected parameter type");
        return nullptr;
    }
    
    // Parse parameter name
    if (match(TokenType::IDENTIFIER)) {
        auto identifierToken = getCurrentToken();
        std::string name = identifierToken->getValue();
        advance();
        
        // Add parameter to symbol table
        if (symbolTable->addParameter(name, paramType, identifierToken->getLine(), identifierToken->getColumn())) {
            return std::make_shared<ASTNode>(NodeType::PARAMETER, name, identifierToken->getLine(), identifierToken->getColumn());
        }
    } else {
        addError("Expected parameter name");
    }
    
    return nullptr;
}

std::shared_ptr<ASTNode> Parser::parseStatement() {
    if (match(TokenType::KEYWORD_IF)) {
        return parseIfStatement();
    } else if (match(TokenType::KEYWORD_WHILE)) {
        return parseWhileStatement();
    } else if (match(TokenType::KEYWORD_FOR)) {
        return parseForStatement();
    } else if (match(TokenType::KEYWORD_RETURN)) {
        return parseReturnStatement();
    } else if (match(TokenType::LEFT_BRACE)) {
        return parseCompoundStatement();
    } else {
        return parseExpressionStatement();
    }
}

std::shared_ptr<ASTNode> Parser::parseExpressionStatement() {
    auto expression = parseExpression();
    if (!expression) {
        return nullptr;
    }
    
    // Expect semicolon
    if (!consume(TokenType::SEMICOLON, "Expected ';' after expression")) {
        return nullptr;
    }
    
    auto stmt = std::make_shared<ASTNode>(NodeType::EXPRESSION_STATEMENT, "expr_stmt", 0, 0);
    stmt->addChild(expression);
    return stmt;
}

std::shared_ptr<ASTNode> Parser::parseCompoundStatement() {
    if (!consume(TokenType::LEFT_BRACE, "Expected '{'")) {
        return nullptr;
    }
    
    enterScope("block");
    
    auto compound = std::make_shared<ASTNode>(NodeType::COMPOUND_STATEMENT, "compound", 0, 0);
    
    while (!isAtEnd() && !match(TokenType::RIGHT_BRACE)) {
        auto stmt = parseStatement();
        if (stmt) {
            compound->addChild(stmt);
        } else {
            synchronize();
        }
    }
    
    consume(TokenType::RIGHT_BRACE, "Expected '}' after block");
    
    exitScope();
    
    return compound;
}

std::shared_ptr<ASTNode> Parser::parseIfStatement() {
    advance(); // consume 'if'
    
    if (!consume(TokenType::LEFT_PAREN, "Expected '(' after 'if'")) {
        return nullptr;
    }
    
    auto condition = parseExpression();
    if (!condition) {
        return nullptr;
    }
    
    if (!consume(TokenType::RIGHT_PAREN, "Expected ')' after if condition")) {
        return nullptr;
    }
    
    auto thenBranch = parseStatement();
    if (!thenBranch) {
        return nullptr;
    }
    
    auto ifStmt = std::make_shared<ASTNode>(NodeType::IF_STATEMENT, "if", 0, 0);
    ifStmt->addChild(condition);
    ifStmt->addChild(thenBranch);
    
    // Parse else branch if present
    if (match(TokenType::KEYWORD_ELSE)) {
        advance();
        auto elseBranch = parseStatement();
        if (elseBranch) {
            ifStmt->addChild(elseBranch);
        }
    }
    
    return ifStmt;
}

std::shared_ptr<ASTNode> Parser::parseWhileStatement() {
    advance(); // consume 'while'
    
    if (!consume(TokenType::LEFT_PAREN, "Expected '(' after 'while'")) {
        return nullptr;
    }
    
    auto condition = parseExpression();
    if (!condition) {
        return nullptr;
    }
    
    if (!consume(TokenType::RIGHT_PAREN, "Expected ')' after while condition")) {
        return nullptr;
    }
    
    auto body = parseStatement();
    if (!body) {
        return nullptr;
    }
    
    auto whileStmt = std::make_shared<ASTNode>(NodeType::WHILE_STATEMENT, "while", 0, 0);
    whileStmt->addChild(condition);
    whileStmt->addChild(body);
    
    return whileStmt;
}

std::shared_ptr<ASTNode> Parser::parseForStatement() {
    advance(); // consume 'for'
    
    if (!consume(TokenType::LEFT_PAREN, "Expected '(' after 'for'")) {
        return nullptr;
    }
    
    // Parse initialization
    std::shared_ptr<ASTNode> initialization = nullptr;
    if (!match(TokenType::SEMICOLON)) {
        initialization = parseExpression();
        if (!initialization) {
            return nullptr;
        }
    }
    
    if (!consume(TokenType::SEMICOLON, "Expected ';' after for initialization")) {
        return nullptr;
    }
    
    // Parse condition
    std::shared_ptr<ASTNode> condition = nullptr;
    if (!match(TokenType::SEMICOLON)) {
        condition = parseExpression();
        if (!condition) {
            return nullptr;
        }
    }
    
    if (!consume(TokenType::SEMICOLON, "Expected ';' after for condition")) {
        return nullptr;
    }
    
    // Parse increment
    std::shared_ptr<ASTNode> increment = nullptr;
    if (!match(TokenType::RIGHT_PAREN)) {
        increment = parseExpression();
        if (!increment) {
            return nullptr;
        }
    }
    
    if (!consume(TokenType::RIGHT_PAREN, "Expected ')' after for clauses")) {
        return nullptr;
    }
    
    auto body = parseStatement();
    if (!body) {
        return nullptr;
    }
    
    auto forStmt = std::make_shared<ASTNode>(NodeType::FOR_STATEMENT, "for", 0, 0);
    if (initialization) forStmt->addChild(initialization);
    if (condition) forStmt->addChild(condition);
    if (increment) forStmt->addChild(increment);
    forStmt->addChild(body);
    
    return forStmt;
}

std::shared_ptr<ASTNode> Parser::parseReturnStatement() {
    advance(); // consume 'return'
    
    std::shared_ptr<ASTNode> value = nullptr;
    if (!match(TokenType::SEMICOLON)) {
        value = parseExpression();
        if (!value) {
            return nullptr;
        }
    }
    
    if (!consume(TokenType::SEMICOLON, "Expected ';' after return value")) {
        return nullptr;
    }
    
    auto returnStmt = std::make_shared<ASTNode>(NodeType::RETURN_STATEMENT, "return", 0, 0);
    if (value) {
        returnStmt->addChild(value);
    }
    
    return returnStmt;
}

std::shared_ptr<ASTNode> Parser::parseExpression() {
    return parseAssignmentExpression();
}

std::shared_ptr<ASTNode> Parser::parseAssignmentExpression() {
    auto left = parseEqualityExpression();
    if (!left) {
        return nullptr;
    }
    
    if (match(TokenType::ASSIGN)) {
        auto opToken = getCurrentToken();
        advance();
        
        auto right = parseAssignmentExpression();
        if (!right) {
            return nullptr;
        }
        
        // Check if left side is a valid lvalue (identifier)
        if (left->getType() != NodeType::IDENTIFIER_EXPRESSION) {
            addError("Invalid assignment target");
            return nullptr;
        }
        
        auto assignExpr = std::make_shared<BinaryExpressionNode>(
            opToken->getValue(), left, right, opToken->getLine(), opToken->getColumn()
        );
        assignExpr->setType(NodeType::ASSIGNMENT_EXPRESSION);
        
        return assignExpr;
    }
    
    return left;
}

std::shared_ptr<ASTNode> Parser::parseEqualityExpression() {
    auto left = parseRelationalExpression();
    if (!left) {
        return nullptr;
    }
    
    while (match(TokenType::EQUAL) || match(TokenType::NOT_EQUAL)) {
        auto opToken = getCurrentToken();
        advance();
        
        auto right = parseRelationalExpression();
        if (!right) {
            return nullptr;
        }
        
        left = std::make_shared<BinaryExpressionNode>(
            opToken->getValue(), left, right, opToken->getLine(), opToken->getColumn()
        );
    }
    
    return left;
}

std::shared_ptr<ASTNode> Parser::parseRelationalExpression() {
    auto left = parseAdditiveExpression();
    if (!left) {
        return nullptr;
    }
    
    while (match(TokenType::LESS_THAN) || match(TokenType::GREATER_THAN) ||
           match(TokenType::LESS_EQUAL) || match(TokenType::GREATER_EQUAL)) {
        auto opToken = getCurrentToken();
        advance();
        
        auto right = parseAdditiveExpression();
        if (!right) {
            return nullptr;
        }
        
        left = std::make_shared<BinaryExpressionNode>(
            opToken->getValue(), left, right, opToken->getLine(), opToken->getColumn()
        );
    }
    
    return left;
}

std::shared_ptr<ASTNode> Parser::parseAdditiveExpression() {
    auto left = parseMultiplicativeExpression();
    if (!left) {
        return nullptr;
    }
    
    while (match(TokenType::PLUS) || match(TokenType::MINUS)) {
        auto opToken = getCurrentToken();
        advance();
        
        auto right = parseMultiplicativeExpression();
        if (!right) {
            return nullptr;
        }
        
        left = std::make_shared<BinaryExpressionNode>(
            opToken->getValue(), left, right, opToken->getLine(), opToken->getColumn()
        );
    }
    
    return left;
}

std::shared_ptr<ASTNode> Parser::parseMultiplicativeExpression() {
    auto left = parseUnaryExpression();
    if (!left) {
        return nullptr;
    }
    
    while (match(TokenType::MULTIPLY) || match(TokenType::DIVIDE)) {
        auto opToken = getCurrentToken();
        advance();
        
        auto right = parseUnaryExpression();
        if (!right) {
            return nullptr;
        }
        
        left = std::make_shared<BinaryExpressionNode>(
            opToken->getValue(), left, right, opToken->getLine(), opToken->getColumn()
        );
    }
    
    return left;
}

std::shared_ptr<ASTNode> Parser::parseUnaryExpression() {
    if (match(TokenType::PLUS) || match(TokenType::MINUS)) {
        auto opToken = getCurrentToken();
        advance();
        
        auto operand = parseUnaryExpression();
        if (!operand) {
            return nullptr;
        }
        
        return std::make_shared<BinaryExpressionNode>(
            opToken->getValue(), nullptr, operand, opToken->getLine(), opToken->getColumn()
        );
    }
    
    return parsePrimaryExpression();
}

std::shared_ptr<ASTNode> Parser::parsePrimaryExpression() {
    // Parenthesized expression
    if (match(TokenType::LEFT_PAREN)) {
        advance();
        auto expr = parseExpression();
        if (!expr) {
            return nullptr;
        }
        
        if (!consume(TokenType::RIGHT_PAREN, "Expected ')' after expression")) {
            return nullptr;
        }
        
        return expr;
    }
    
    // Number literal
    if (match(TokenType::NUMBER)) {
        auto token = getCurrentToken();
        advance();
        return std::make_shared<LiteralExpressionNode>(
            token->getValue(), DataType::INT, token->getLine(), token->getColumn()
        );
    }
    
    // String literal
    if (match(TokenType::STRING)) {
        auto token = getCurrentToken();
        advance();
        return std::make_shared<LiteralExpressionNode>(
            token->getValue(), DataType::STRING, token->getLine(), token->getColumn()
        );
    }
    
    // Character literal
    if (match(TokenType::CHARACTER)) {
        auto token = getCurrentToken();
        advance();
        return std::make_shared<LiteralExpressionNode>(
            token->getValue(), DataType::CHAR, token->getLine(), token->getColumn()
        );
    }
    
    // Identifier
    if (match(TokenType::IDENTIFIER)) {
        auto token = getCurrentToken();
        advance();
        
        // Check if it's a function call
        if (match(TokenType::LEFT_PAREN)) {
            return parseFunctionCall(token->getValue(), token->getLine(), token->getColumn());
        }
        
        // Check if symbol is declared
        if (!useSymbol(token->getValue())) {
            addErrorAtToken("Undefined identifier: " + token->getValue(), *token);
        }
        
        return std::make_shared<IdentifierExpressionNode>(
            token->getValue(), token->getLine(), token->getColumn()
        );
    }
    
    auto currentToken = getCurrentToken();
    if (currentToken) {
        addErrorAtToken("Unexpected token: " + currentToken->getValue(), *currentToken);
    } else {
        addError("Unexpected end of input");
    }
    
    return nullptr;
}

std::shared_ptr<ASTNode> Parser::parseFunctionCall(const std::string& functionName, int line, int column) {
    advance(); // consume '('
    
    auto callNode = std::make_shared<ASTNode>(NodeType::FUNCTION_CALL, functionName, line, column);
    
    // Parse arguments
    if (!match(TokenType::RIGHT_PAREN)) {
        auto arg = parseExpression();
        if (arg) {
            callNode->addChild(arg);
        }
        
        while (match(TokenType::COMMA)) {
            advance();
            arg = parseExpression();
            if (arg) {
                callNode->addChild(arg);
            }
        }
    }
    
    if (!consume(TokenType::RIGHT_PAREN, "Expected ')' after function arguments")) {
        return nullptr;
    }
    
    // Check if function is declared
    auto symbol = symbolTable->lookup(functionName);
    if (!symbol || !symbol->isFunction()) {
        addErrorAtToken("Undefined function: " + functionName, *getCurrentToken());
    }
    
    return callNode;
}

DataType Parser::tokenTypeToDataType(TokenType tokenType) {
    switch (tokenType) {
        case TokenType::KEYWORD_INT: return DataType::INT;
        case TokenType::KEYWORD_FLOAT: return DataType::FLOAT;
        case TokenType::KEYWORD_CHAR: return DataType::CHAR;
        case TokenType::KEYWORD_VOID: return DataType::VOID;
        default: return DataType::UNKNOWN;
    }
}

void Parser::synchronize() {
    advance();
    
    while (!isAtEnd()) {
        if (getCurrentToken()->getType() == TokenType::SEMICOLON) {
            advance();
            return;
        }
        
        TokenType type = getCurrentToken()->getType();
        if (type == TokenType::KEYWORD_INT || type == TokenType::KEYWORD_FLOAT ||
            type == TokenType::KEYWORD_CHAR || type == TokenType::KEYWORD_VOID ||
            type == TokenType::KEYWORD_IF || type == TokenType::KEYWORD_WHILE ||
            type == TokenType::KEYWORD_FOR || type == TokenType::KEYWORD_RETURN) {
            return;
        }
        
        advance();
    }
}

void Parser::enterScope(const std::string& scopeName) {
    auto token = getCurrentToken();
    int line = token ? token->getLine() : 0;
    symbolTable->enterScope(scopeName, line);
}

void Parser::exitScope() {
    auto token = getCurrentToken();
    int line = token ? token->getLine() : 0;
    symbolTable->exitScope(line);
}

bool Parser::declareSymbol(const std::string& name, SymbolKind kind, DataType dataType) {
    auto token = getCurrentToken();
    int line = token ? token->getLine() : 0;
    int column = token ? token->getColumn() : 0;
    
    return symbolTable->addSymbol(name, kind, dataType, line, column);
}

bool Parser::useSymbol(const std::string& name) {
    if (symbolTable->markAsUsed(name)) {
        return true;
    }
    return symbolTable->isDeclared(name);
}

void Parser::printAST(std::shared_ptr<ASTNode> root) const {
    if (root) {
        root->printTree();
    } else {
        std::cout << "No AST to print\n";
    }
}

void Parser::printErrors() const {
    std::cout << "=== Parser Errors ===\n";
    if (errors.empty()) {
        std::cout << "No errors\n";
    } else {
        for (const auto& error : errors) {
            std::cout << error << std::endl;
        }
    }
}

void Parser::printWarnings() const {
    std::cout << "=== Parser Warnings ===\n";
    if (warnings.empty()) {
        std::cout << "No warnings\n";
    } else {
        for (const auto& warning : warnings) {
            std::cout << warning << std::endl;
        }
    }
}

std::string Parser::nodeTypeToString(NodeType type) {
    return ASTNode::nodeTypeToString(type);
}
