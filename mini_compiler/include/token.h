#ifndef TOKEN_H
#define TOKEN_H

#include <string>
#include <iostream>

enum class TokenType {
    // Keywords
    KEYWORD_INT,
    KEYWORD_FLOAT,
    KEYWORD_CHAR,
    KEYWORD_VOID,
    KEYWORD_IF,
    KEYWORD_ELSE,
    KEYWORD_WHILE,
    KEYWORD_FOR,
    KEYWORD_RETURN,
    
    // Identifiers and Literals
    IDENTIFIER,
    NUMBER,
    STRING,
    CHARACTER,
    
    // Operators
    PLUS,
    MINUS,
    MULTIPLY,
    DIVIDE,
    ASSIGN,
    EQUAL,
    NOT_EQUAL,
    LESS_THAN,
    GREATER_THAN,
    LESS_EQUAL,
    GREATER_EQUAL,
    
    // Delimiters
    LEFT_PAREN,
    RIGHT_PAREN,
    LEFT_BRACE,
    RIGHT_BRACE,
    LEFT_BRACKET,
    RIGHT_BRACKET,
    SEMICOLON,
    COMMA,
    
    // Special
    END_OF_FILE,
    UNKNOWN
};

class Token {
private:
    TokenType type;
    std::string value;
    int line;
    int column;

public:
    Token(TokenType type, const std::string& value, int line, int column);
    
    // Getters
    TokenType getType() const { return type; }
    const std::string& getValue() const { return value; }
    int getLine() const { return line; }
    int getColumn() const { return column; }
    
    // Utility methods
    bool isKeyword() const;
    bool isOperator() const;
    bool isDelimiter() const;
    bool isLiteral() const;
    
    // Display
    std::string toString() const;
    void print() const;
    
    // Static utility methods
    static std::string tokenTypeToString(TokenType type);
};

#endif // TOKEN_H
