#include "bnf_parser.hpp"
#include "utf8_utils.hpp"
#include <cctype>
#include <stdexcept>

namespace bnf_parser_generator {

BNFLexer::BNFLexer(const std::string& input) 
    : input_(input), pos_(0), line_(1), column_(1) {}

std::vector<BNFToken> BNFLexer::tokenize() {
    std::vector<BNFToken> tokens;
    
    while (pos_ < input_.length()) {
        skipWhitespace();
        
        if (pos_ >= input_.length()) break;
        
        char c = peek();
        
        // Комментарии
        if (c == '#') {
            tokens.push_back(readComment());
            continue;
        }
        
        // Переводы строк (важны для структуры грамматики)
        if (c == '\n') {
            tokens.emplace_back(TokenType::NEWLINE, "\\n", line_, column_);
            advance();
            continue;
        }
        
        // Строковые литералы
        if (c == '"' || c == '\'') {
            tokens.push_back(readString());
            continue;
        }
        
        // Проверяем двухсимвольные операторы
        if (c == ':' && peek(1) == ':' && peek(2) == '=') {
            tokens.emplace_back(TokenType::DEFINE, "::=", line_, column_);
            advance(); advance(); advance();
            continue;
        }
        
        if (c == '.' && peek(1) == '.') {
            tokens.emplace_back(TokenType::DOT_DOT, "..", line_, column_);
            advance(); advance();
            continue;
        }
        
        // Односимвольные операторы
        switch (c) {
            case '|':
                tokens.emplace_back(TokenType::ALTERNATIVE, "|", line_, column_);
                advance();
                break;
            case '(':
                tokens.emplace_back(TokenType::LEFT_PAREN, "(", line_, column_);
                advance();
                break;
            case ')':
                tokens.emplace_back(TokenType::RIGHT_PAREN, ")", line_, column_);
                advance();
                break;
            case '[':
                tokens.emplace_back(TokenType::LEFT_BRACKET, "[", line_, column_);
                advance();
                break;
            case ']':
                tokens.emplace_back(TokenType::RIGHT_BRACKET, "]", line_, column_);
                advance();
                break;
            case '{':
                tokens.emplace_back(TokenType::LEFT_BRACE, "{", line_, column_);
                advance();
                break;
            case '}':
                tokens.emplace_back(TokenType::RIGHT_BRACE, "}", line_, column_);
                advance();
                break;
            case '+':
                tokens.emplace_back(TokenType::PLUS, "+", line_, column_);
                advance();
                break;
            case '*':
                tokens.emplace_back(TokenType::STAR, "*", line_, column_);
                advance();
                break;
            case '?':
                tokens.emplace_back(TokenType::QUESTION, "?", line_, column_);
                advance();
                break;
            case ',':
                tokens.emplace_back(TokenType::COMMA, ",", line_, column_);
                advance();
                break;
            case ';':
                tokens.emplace_back(TokenType::SEMICOLON, ";", line_, column_);
                advance();
                break;
            case ':':
                tokens.emplace_back(TokenType::COLON, ":", line_, column_);
                advance();
                break;
            default:
                // Идентификаторы и нетерминалы
                if (isAlpha(c) || c == '_' || c == '<') {
                    tokens.push_back(readIdentifier());
                } else {
                    tokens.emplace_back(TokenType::UNKNOWN, std::string(1, c), line_, column_);
                    advance();
                }
                break;
        }
    }
    
    tokens.emplace_back(TokenType::EOF_TOKEN, "", line_, column_);
    return tokens;
}

char BNFLexer::peek(size_t offset) const {
    size_t pos = pos_ + offset;
    return (pos < input_.length()) ? input_[pos] : '\0';
}

char BNFLexer::advance() {
    if (pos_ >= input_.length()) return '\0';
    
    char c = input_[pos_++];
    if (c == '\n') {
        line_++;
        column_ = 1;
    } else {
        column_++;
    }
    return c;
}

void BNFLexer::skipWhitespace() {
    while (pos_ < input_.length()) {
        char c = peek();
        if (c == ' ' || c == '\t' || c == '\r') {
            advance();
        } else {
            break;
        }
    }
}

BNFToken BNFLexer::readString() {
    size_t startLine = line_;
    size_t startColumn = column_;
    
    char quote = advance(); // Пропускаем открывающую кавычку
    std::string value;
    
    while (pos_ < input_.length()) {
        char c = peek();
        
        if (c == quote) {
            advance(); // Пропускаем закрывающую кавычку
            break;
        }
        
        if (c == '\\') {
            advance(); // Пропускаем обратный слеш
            char escaped = peek();
            
            // Обработка Unicode escape-последовательностей \uXXXX или \UXXXXXXXX
            if (escaped == 'u' || escaped == 'U') {
                bool isExtended = (escaped == 'U');
                advance(); // Пропускаем 'u' или 'U'
                
                // Читаем 4 или 8 шестнадцатеричных цифр
                int hexDigits = isExtended ? 8 : 4;
                std::string hexCode;
                for (int i = 0; i < hexDigits; ++i) {
                    char hexChar = peek();
                    if ((hexChar >= '0' && hexChar <= '9') ||
                        (hexChar >= 'a' && hexChar <= 'f') ||
                        (hexChar >= 'A' && hexChar <= 'F')) {
                        hexCode += advance();
                    } else {
                        throw std::runtime_error("Invalid Unicode escape sequence at line " + 
                                                std::to_string(line_) + ", column " + 
                                                std::to_string(column_));
                    }
                }
                
                // Преобразуем hex в число
                uint32_t codepoint = 0;
                for (char hexChar : hexCode) {
                    codepoint = codepoint * 16;
                    if (hexChar >= '0' && hexChar <= '9') {
                        codepoint += (hexChar - '0');
                    } else if (hexChar >= 'a' && hexChar <= 'f') {
                        codepoint += (hexChar - 'a' + 10);
                    } else if (hexChar >= 'A' && hexChar <= 'F') {
                        codepoint += (hexChar - 'A' + 10);
                    }
                }
                
                // Используем utf8 утилиты для преобразования codepoint в UTF-8
                value += utf8::codepointToUtf8(codepoint);
            } else {
                advance();
                switch (escaped) {
                    case 'n': value += '\n'; break;
                    case 't': value += '\t'; break;
                    case 'r': value += '\r'; break;
                    case '\\': value += '\\'; break;
                    case '"': value += '"'; break;
                    case '\'': value += '\''; break;
                    default: 
                        value += '\\';
                        value += escaped;
                        break;
                }
            }
        } else {
            value += advance();
        }
    }
    
    return BNFToken(TokenType::TERMINAL, value, startLine, startColumn);
}

BNFToken BNFLexer::readIdentifier() {
    size_t startLine = line_;
    size_t startColumn = column_;
    std::string value;
    
    // Поддержка нетерминалов в угловых скобках: <identifier>
    bool inBrackets = false;
    if (peek() == '<') {
        inBrackets = true;
        advance(); // Пропускаем <
    }
    
    while (pos_ < input_.length()) {
        char c = peek();
        
        if (inBrackets && c == '>') {
            advance(); // Пропускаем >
            break;
        }
        
        if (!inBrackets && !isAlnum(c) && c != '_' && c != '-') {
            break;
        }
        
        if (inBrackets && (isAlnum(c) || c == '_' || c == '-' || c == ' ')) {
            value += advance();
        } else if (!inBrackets && (isAlnum(c) || c == '_' || c == '-')) {
            value += advance();
        } else {
            break;
        }
    }
    
    return BNFToken(TokenType::IDENTIFIER, value, startLine, startColumn);
}

BNFToken BNFLexer::readComment() {
    size_t startLine = line_;
    size_t startColumn = column_;
    std::string value;
    
    advance(); // Пропускаем #
    
    while (pos_ < input_.length() && peek() != '\n') {
        value += advance();
    }
    
    return BNFToken(TokenType::COMMENT, value, startLine, startColumn);
}

bool BNFLexer::isAlpha(char c) const {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool BNFLexer::isAlnum(char c) const {
    return isAlpha(c) || isDigit(c);
}

bool BNFLexer::isDigit(char c) const {
    return c >= '0' && c <= '9';
}

} // namespace bnf_parser_generator
