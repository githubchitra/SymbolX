#ifndef LEXER_H
#define LEXER_H

#include "token.h"
#include <string>
#include <vector>
#include <memory>

class Lexer {
private:
    std::string source;
    int position;
    int line;
    int column;
    char currentChar;
    
    // Error tracking
    std::vector<std::string> errors;
    
    // Private helper methods
    void advance();
    void skipWhitespace();
    void skipComment();
    char peek(int offset = 0);
    bool match(char expected);
    std::string readNumber();
    std::string readIdentifier();
    std::string readString();
    std::string readCharacter();
    
    // Token creation
    std::unique_ptr<Token> createToken(TokenType type, const std::string& value);
    std::unique_ptr<Token> createToken(TokenType type);
    void addError(const std::string& message);
    
    // Keyword identification
    TokenType getKeywordType(const std::string& identifier);
    bool isKeyword(const std::string& identifier);
    
    // Character classification
    bool isDigit(char c);
    bool isLetter(char c);
    bool isAlphaNumeric(char c);
    bool isWhitespace(char c);

public:
    explicit Lexer(const std::string& source);
    
    // Main lexing method
    std::unique_ptr<Token> getNextToken();
    std::vector<std::unique_ptr<Token>> tokenizeAll();
    
    // Error handling
    const std::vector<std::string>& getErrors() const { return errors; }
    bool hasErrors() const { return !errors.empty(); }
    void clearErrors();
    
    // Utility methods
    void reset();
    int getCurrentLine() const { return line; }
    int getCurrentColumn() const { return column; }
    bool isAtEnd() const;
    
    // Debugging
    void printTokens(const std::vector<std::unique_ptr<Token>>& tokens);
};

#endif // LEXER_H
