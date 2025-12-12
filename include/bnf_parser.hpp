#pragma once

#include "bnf_ast.hpp"
#include <string>
#include <memory>
#include <vector>
#include <unordered_set>

namespace bnf_parser_generator {

/**
 * Лексер для BNF/EBNF грамматик
 */
enum class TokenType {
    IDENTIFIER,     // нетерминал или имя правила
    TERMINAL,       // строка в кавычках
    CHAR_RANGE,     // 'a'..'z'
    DEFINE,         // ::=
    ALTERNATIVE,    // |
    LEFT_PAREN,     // (
    RIGHT_PAREN,    // )
    LEFT_BRACKET,   // [
    RIGHT_BRACKET,  // ]
    LEFT_BRACE,     // {
    RIGHT_BRACE,    // }
    PLUS,           // +
    STAR,           // *
    QUESTION,       // ?
    DOT_DOT,        // ..
    COMMA,          // , (для параметров)
    COLON,          // : (для типов параметров)
    ACTION_OPEN,    // { (начало контекстного действия)
    ACTION_CLOSE,   // } (конец контекстного действия)
    SEMICOLON,      // ;
    COMMENT,        // # комментарий
    NEWLINE,        // \n
    EOF_TOKEN,      // конец файла
    UNKNOWN
};

struct BNFToken {
    TokenType type;
    std::string value;
    size_t line;
    size_t column;
    
    BNFToken(TokenType t, const std::string& v, size_t l, size_t c)
        : type(t), value(v), line(l), column(c) {}
};

class BNFLexer {
private:
    std::string input_;
    size_t pos_;
    size_t line_;
    size_t column_;
    
public:
    explicit BNFLexer(const std::string& input);
    
    std::vector<BNFToken> tokenize();
    
private:
    char peek(size_t offset = 0) const;
    char advance();
    void skipWhitespace();
    BNFToken readString();
    BNFToken readIdentifier();
    BNFToken readComment();
    BNFToken readCharRange();
    bool isAlpha(char c) const;
    bool isAlnum(char c) const;
    bool isDigit(char c) const;
};

/**
 * Парсер BNF/EBNF грамматик
 * Реализует рекурсивный спуск согласно классическим правилам
 */
class BNFParser {
private:
    std::vector<BNFToken> tokens_;
    size_t current_;
    std::string error_;
    
public:
    explicit BNFParser(const std::vector<BNFToken>& tokens);
    
    // Основной метод парсинга
    std::unique_ptr<Grammar> parseGrammar();
    
    // Получить последнюю ошибку
    const std::string& getError() const { return error_; }
    
    // Валидация грамматики согласно классическим правилам BNF
    struct ValidationResult {
        bool isValid;
        std::vector<std::string> errors;
        std::vector<std::string> warnings;
    };
    
    static ValidationResult validateGrammar(const Grammar& grammar);

private:
    // Вспомогательные методы для валидации
    static void collectSymbols(const ASTNode* node, 
                              std::unordered_set<std::string>& nonTerminals,
                              std::unordered_set<std::string>& terminals);
    static bool isProductive(const ASTNode* node, 
                            const std::unordered_set<std::string>& productive);
    // Методы парсинга по правилам грамматики BNF
    std::unique_ptr<ProductionRule> parseRule();
    std::unique_ptr<ASTNode> parseExpression();
    std::unique_ptr<ASTNode> parseAlternative();
    std::unique_ptr<ASTNode> parseSequence();
    std::unique_ptr<ASTNode> parseFactor();
    std::unique_ptr<ASTNode> parsePrimary();
    std::unique_ptr<ASTNode> parseTerminalOrCharRange();
    
    // Утилиты
    bool match(TokenType type);
    bool check(TokenType type) const;
    BNFToken advance();
    BNFToken peek() const;
    bool isAtEnd() const;
    void error(const std::string& message);
    
    // EBNF конструкции
    std::unique_ptr<ASTNode> parseOptional(std::unique_ptr<ASTNode> content);
    std::unique_ptr<ASTNode> parseZeroOrMore(std::unique_ptr<ASTNode> content);
    std::unique_ptr<ASTNode> parseOneOrMore(std::unique_ptr<ASTNode> content);
    std::unique_ptr<ASTNode> parseGroup();
    
    // Специальные конструкции
    std::unique_ptr<ASTNode> parseCharRange();
    
    // Extended BNF для контекстно-зависимых грамматик
    std::vector<RuleParameter> parseRuleParameters();  // [param1, param2:type]
    RuleParameter parseRuleParameter();                 // param:type
    ParameterType parseParameterType();                 // enum{val1,val2} | int | string | bool
    std::vector<std::string> parseEnumValues();        // {val1, val2, val3}
    std::vector<std::string> parseParameterValues();   // [val1, val2] для вызовов
    std::unique_ptr<ContextAction> parseContextAction(); // {store(name, value)}
    std::unique_ptr<NonTerminal> parseParameterizedNonTerminal(); // <rule>[param1, param2]
};

/**
 * Фабрика для создания парсеров из различных источников
 */
class BNFGrammarFactory {
public:
    // Парсинг из строки
    static std::unique_ptr<Grammar> fromString(const std::string& bnfText);
    
    // Парсинг из файла
    static std::unique_ptr<Grammar> fromFile(const std::string& filename);
    
    // Предустановленные грамматики (переписанные для нового парсера)
    static std::unique_ptr<Grammar> createJSONGrammar();
    static std::unique_ptr<Grammar> createPrologGrammar();
    static std::unique_ptr<Grammar> createClojureGrammar();
    
    // Создание простых грамматик для тестирования
    static std::unique_ptr<Grammar> createArithmeticGrammar();
    static std::unique_ptr<Grammar> createIdentifierGrammar();
    
    // Extended BNF примеры (контекстно-зависимые грамматики)
    static std::unique_ptr<Grammar> createAgreementGrammar();      // Согласование: noun[N] verb[N]
    static std::unique_ptr<Grammar> createIndentationGrammar();    // Python-style indentation
    static std::unique_ptr<Grammar> createYamlAnchorsGrammar();    // YAML anchors and references
};

} // namespace bnf_parser_generator
