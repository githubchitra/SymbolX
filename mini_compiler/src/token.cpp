#include "token.h"
#include <sstream>

Token::Token(TokenType type, const std::string& value, int line, int column)
    : type(type), value(value), line(line), column(column) {}

bool Token::isKeyword() const {
    return type == TokenType::KEYWORD_INT || type == TokenType::KEYWORD_FLOAT ||
           type == TokenType::KEYWORD_CHAR || type == TokenType::KEYWORD_VOID ||
           type == TokenType::KEYWORD_IF || type == TokenType::KEYWORD_ELSE ||
           type == TokenType::KEYWORD_WHILE || type == TokenType::KEYWORD_FOR ||
           type == TokenType::KEYWORD_RETURN;
}

bool Token::isOperator() const {
    return type == TokenType::PLUS || type == TokenType::MINUS ||
           type == TokenType::MULTIPLY || type == TokenType::DIVIDE ||
           type == TokenType::ASSIGN || type == TokenType::EQUAL ||
           type == TokenType::NOT_EQUAL || type == TokenType::LESS_THAN ||
           type == TokenType::GREATER_THAN || type == TokenType::LESS_EQUAL ||
           type == TokenType::GREATER_EQUAL;
}

bool Token::isDelimiter() const {
    return type == TokenType::LEFT_PAREN || type == TokenType::RIGHT_PAREN ||
           type == TokenType::LEFT_BRACE || type == TokenType::RIGHT_BRACE ||
           type == TokenType::LEFT_BRACKET || type == TokenType::RIGHT_BRACKET ||
           type == TokenType::SEMICOLON || type == TokenType::COMMA;
}

bool Token::isLiteral() const {
    return type == TokenType::NUMBER || type == TokenType::STRING ||
           type == TokenType::CHARACTER;
}

std::string Token::toString() const {
    std::ostringstream oss;
    oss << "Token[" << tokenTypeToString(type) << ", '" << value 
        << "', line=" << line << ", col=" << column << "]";
    return oss.str();
}

void Token::print() const {
    std::cout << toString() << std::endl;
}

std::string Token::tokenTypeToString(TokenType type) {
    switch (type) {
        // Keywords
        case TokenType::KEYWORD_INT: return "INT";
        case TokenType::KEYWORD_FLOAT: return "FLOAT";
        case TokenType::KEYWORD_CHAR: return "CHAR";
        case TokenType::KEYWORD_VOID: return "VOID";
        case TokenType::KEYWORD_IF: return "IF";
        case TokenType::KEYWORD_ELSE: return "ELSE";
        case TokenType::KEYWORD_WHILE: return "WHILE";
        case TokenType::KEYWORD_FOR: return "FOR";
        case TokenType::KEYWORD_RETURN: return "RETURN";
        
        // Identifiers and Literals
        case TokenType::IDENTIFIER: return "IDENTIFIER";
        case TokenType::NUMBER: return "NUMBER";
        case TokenType::STRING: return "STRING";
        case TokenType::CHARACTER: return "CHARACTER";
        
        // Operators
        case TokenType::PLUS: return "PLUS";
        case TokenType::MINUS: return "MINUS";
        case TokenType::MULTIPLY: return "MULTIPLY";
        case TokenType::DIVIDE: return "DIVIDE";
        case TokenType::ASSIGN: return "ASSIGN";
        case TokenType::EQUAL: return "EQUAL";
        case TokenType::NOT_EQUAL: return "NOT_EQUAL";
        case TokenType::LESS_THAN: return "LESS_THAN";
        case TokenType::GREATER_THAN: return "GREATER_THAN";
        case TokenType::LESS_EQUAL: return "LESS_EQUAL";
        case TokenType::GREATER_EQUAL: return "GREATER_EQUAL";
        
        // Delimiters
        case TokenType::LEFT_PAREN: return "LEFT_PAREN";
        case TokenType::RIGHT_PAREN: return "RIGHT_PAREN";
        case TokenType::LEFT_BRACE: return "LEFT_BRACE";
        case TokenType::RIGHT_BRACE: return "RIGHT_BRACE";
        case TokenType::LEFT_BRACKET: return "LEFT_BRACKET";
        case TokenType::RIGHT_BRACKET: return "RIGHT_BRACKET";
        case TokenType::SEMICOLON: return "SEMICOLON";
        case TokenType::COMMA: return "COMMA";
        
        // Special
        case TokenType::END_OF_FILE: return "EOF";
        case TokenType::UNKNOWN: return "UNKNOWN";
        
        default: return "UNKNOWN";
    }
}
