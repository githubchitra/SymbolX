#include "lexer.h"
#include <cctype>
#include <iostream>
#include <sstream>

Lexer::Lexer(const std::string& source) 
    : source(source), position(0), line(1), column(1), currentChar(source[0]) {}

void Lexer::advance() {
    if (position >= source.length()) {
        currentChar = '\0';
        return;
    }
    
    if (currentChar == '\n') {
        line++;
        column = 1;
    } else {
        column++;
    }
    
    position++;
    currentChar = position < source.length() ? source[position] : '\0';
}

void Lexer::skipWhitespace() {
    while (isWhitespace(currentChar)) {
        advance();
    }
}

void Lexer::skipComment() {
    if (currentChar == '/') {
        if (peek() == '/') {
            // Single-line comment
            advance(); // Skip '/'
            advance(); // Skip '/'
            while (currentChar != '\n' && currentChar != '\0') {
                advance();
            }
        } else if (peek() == '*') {
            // Multi-line comment
            advance(); // Skip '/'
            advance(); // Skip '*'
            while (currentChar != '\0' && !(currentChar == '*' && peek() == '/')) {
                advance();
            }
            if (currentChar == '*') {
                advance(); // Skip '*'
                advance(); // Skip '/'
            } else {
                addError("Unterminated multi-line comment");
            }
        }
    }
}

char Lexer::peek(int offset) {
    int peekPos = position + offset;
    if (peekPos >= source.length()) {
        return '\0';
    }
    return source[peekPos];
}

bool Lexer::match(char expected) {
    if (currentChar == expected) {
        advance();
        return true;
    }
    return false;
}

std::string Lexer::readNumber() {
    std::string number;
    while (isDigit(currentChar)) {
        number += currentChar;
        advance();
    }
    
    // Handle floating point numbers
    if (currentChar == '.') {
        number += currentChar;
        advance();
        while (isDigit(currentChar)) {
            number += currentChar;
            advance();
        }
    }
    
    return number;
}

std::string Lexer::readIdentifier() {
    std::string identifier;
    while (isAlphaNumeric(currentChar)) {
        identifier += currentChar;
        advance();
    }
    return identifier;
}

std::string Lexer::readString() {
    std::string string;
    advance(); // Skip opening quote
    
    while (currentChar != '"' && currentChar != '\0') {
        if (currentChar == '\\') {
            advance(); // Skip escape character
            if (currentChar != '\0') {
                string += currentChar;
                advance();
            }
        } else {
            string += currentChar;
            advance();
        }
    }
    
    if (currentChar == '"') {
        advance(); // Skip closing quote
    } else {
        addError("Unterminated string literal");
    }
    
    return string;
}

std::string Lexer::readCharacter() {
    std::string character;
    advance(); // Skip opening quote
    
    if (currentChar != '\0') {
        if (currentChar == '\\') {
            advance(); // Skip escape character
            if (currentChar != '\0') {
                character += currentChar;
                advance();
            }
        } else {
            character += currentChar;
            advance();
        }
    }
    
    if (currentChar == '\'') {
        advance(); // Skip closing quote
    } else {
        addError("Unterminated character literal");
    }
    
    return character;
}

std::unique_ptr<Token> Lexer::createToken(TokenType type, const std::string& value) {
    return std::make_unique<Token>(type, value, line, column - value.length());
}

std::unique_ptr<Token> Lexer::createToken(TokenType type) {
    return std::make_unique<Token>(type, "", line, column);
}

void Lexer::addError(const std::string& message) {
    std::ostringstream oss;
    oss << "Line " << line << ", Column " << column << ": " << message;
    errors.push_back(oss.str());
}

TokenType Lexer::getKeywordType(const std::string& identifier) {
    if (identifier == "int") return TokenType::KEYWORD_INT;
    if (identifier == "float") return TokenType::KEYWORD_FLOAT;
    if (identifier == "char") return TokenType::KEYWORD_CHAR;
    if (identifier == "void") return TokenType::KEYWORD_VOID;
    if (identifier == "if") return TokenType::KEYWORD_IF;
    if (identifier == "else") return TokenType::KEYWORD_ELSE;
    if (identifier == "while") return TokenType::KEYWORD_WHILE;
    if (identifier == "for") return TokenType::KEYWORD_FOR;
    if (identifier == "return") return TokenType::KEYWORD_RETURN;
    return TokenType::IDENTIFIER;
}

bool Lexer::isKeyword(const std::string& identifier) {
    return getKeywordType(identifier) != TokenType::IDENTIFIER;
}

bool Lexer::isDigit(char c) {
    return std::isdigit(c);
}

bool Lexer::isLetter(char c) {
    return std::isalpha(c) || c == '_';
}

bool Lexer::isAlphaNumeric(char c) {
    return isLetter(c) || isDigit(c);
}

bool Lexer::isWhitespace(char c) {
    return std::isspace(c);
}

std::unique_ptr<Token> Lexer::getNextToken() {
    while (currentChar != '\0') {
        // Skip whitespace and comments
        skipWhitespace();
        if (currentChar == '/') {
            skipComment();
            continue;
        }
        
        // Numbers
        if (isDigit(currentChar)) {
            return createToken(TokenType::NUMBER, readNumber());
        }
        
        // Identifiers and keywords
        if (isLetter(currentChar)) {
            std::string identifier = readIdentifier();
            TokenType type = getKeywordType(identifier);
            return createToken(type, identifier);
        }
        
        // String literals
        if (currentChar == '"') {
            return createToken(TokenType::STRING, readString());
        }
        
        // Character literals
        if (currentChar == '\'') {
            return createToken(TokenType::CHARACTER, readCharacter());
        }
        
        // Two-character operators
        if (currentChar == '=' && peek() == '=') {
            advance();
            advance();
            return createToken(TokenType::EQUAL, "==");
        }
        
        if (currentChar == '!' && peek() == '=') {
            advance();
            advance();
            return createToken(TokenType::NOT_EQUAL, "!=");
        }
        
        if (currentChar == '<' && peek() == '=') {
            advance();
            advance();
            return createToken(TokenType::LESS_EQUAL, "<=");
        }
        
        if (currentChar == '>' && peek() == '=') {
            advance();
            advance();
            return createToken(TokenType::GREATER_EQUAL, ">=");
        }
        
        // Single-character tokens
        switch (currentChar) {
            case '+': advance(); return createToken(TokenType::PLUS, "+");
            case '-': advance(); return createToken(TokenType::MINUS, "-");
            case '*': advance(); return createToken(TokenType::MULTIPLY, "*");
            case '/': advance(); return createToken(TokenType::DIVIDE, "/");
            case '=': advance(); return createToken(TokenType::ASSIGN, "=");
            case '<': advance(); return createToken(TokenType::LESS_THAN, "<");
            case '>': advance(); return createToken(TokenType::GREATER_THAN, ">");
            case '(': advance(); return createToken(TokenType::LEFT_PAREN, "(");
            case ')': advance(); return createToken(TokenType::RIGHT_PAREN, ")");
            case '{': advance(); return createToken(TokenType::LEFT_BRACE, "{");
            case '}': advance(); return createToken(TokenType::RIGHT_BRACE, "}");
            case '[': advance(); return createToken(TokenType::LEFT_BRACKET, "[");
            case ']': advance(); return createToken(TokenType::RIGHT_BRACKET, "]");
            case ';': advance(); return createToken(TokenType::SEMICOLON, ";");
            case ',': advance(); return createToken(TokenType::COMMA, ",");
            default:
                std::ostringstream oss;
                oss << "Unexpected character: '" << currentChar << "'";
                addError(oss.str());
                advance();
                return createToken(TokenType::UNKNOWN, std::string(1, currentChar));
        }
    }
    
    return createToken(TokenType::END_OF_FILE, "");
}

std::vector<std::unique_ptr<Token>> Lexer::tokenizeAll() {
    std::vector<std::unique_ptr<Token>> tokens;
    
    while (true) {
        auto token = getNextToken();
        tokens.push_back(std::move(token));
        
        if (tokens.back()->getType() == TokenType::END_OF_FILE) {
            break;
        }
    }
    
    return tokens;
}

void Lexer::clearErrors() {
    errors.clear();
}

void Lexer::reset() {
    position = 0;
    line = 1;
    column = 1;
    currentChar = source.empty() ? '\0' : source[0];
    errors.clear();
}

bool Lexer::isAtEnd() const {
    return position >= source.length();
}

void Lexer::printTokens(const std::vector<std::unique_ptr<Token>>& tokens) {
    std::cout << "Tokens:\n";
    std::cout << "-------\n";
    for (const auto& token : tokens) {
        token->print();
    }
    std::cout << "-------\n";
    std::cout << "Total tokens: " << tokens.size() << "\n";
}
