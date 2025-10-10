#pragma once

#include "bnf_ast.hpp"
#include "grammar_tokenizer.hpp"
#include <string>
#include <memory>
#include <vector>
#include <unordered_set>

namespace bnf_parser {

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

// Forward declarations
class Grammar;
class Tokenizer;

/**
 * Интерфейс парсера BNF/EBNF
 */
class Parser {
public:
    virtual ~Parser() = default;
    
    // Создать экземпляр парсера
    static std::unique_ptr<Parser> create();
    
    // Парсинг грамматик
    virtual std::unique_ptr<Grammar> parseGrammarFromString(const std::string& bnfText) = 0;
    virtual std::unique_ptr<Grammar> parseGrammarFromFile(const std::string& filename) = 0;
    
    // Создание токенизаторов
    virtual std::unique_ptr<Tokenizer> createTokenizer(const Grammar& grammar) = 0;
    virtual std::unique_ptr<Tokenizer> createTokenizerFromString(const std::string& bnfText) = 0;
    virtual std::unique_ptr<Tokenizer> createTokenizerFromFile(const std::string& filename) = 0;
    
    // Предустановленные грамматики
    virtual std::unique_ptr<Grammar> createJSONGrammar() = 0;
    virtual std::unique_ptr<Grammar> createPrologGrammar() = 0;
    virtual std::unique_ptr<Grammar> createClojureGrammar() = 0;
    virtual std::unique_ptr<Grammar> createArithmeticGrammar() = 0;
    
    // Валидация грамматик
    struct ValidationResult {
        bool isValid;
        std::vector<std::string> errors;
        std::vector<std::string> warnings;
    };
    
    virtual ValidationResult validateGrammar(const Grammar& grammar) = 0;
    
    // Информация о парсере
    virtual std::string getVersion() const = 0;
    virtual std::vector<std::string> getSupportedFeatures() const = 0;
};

/**
 * Утилиты для работы с токенами
 */
namespace utils {
    // Преобразовать токены обратно в текст
    std::string tokensToString(const std::vector<Token>& tokens);
    
    // Найти токены определённого типа
    std::vector<Token> findTokensByType(const std::vector<Token>& tokens, const std::string& type);
    
    // Статистика токенизации
    struct TokenStats {
        size_t total_tokens;
        size_t unique_types;
        std::unordered_map<std::string, size_t> type_counts;
    };
    
    TokenStats analyzeTokens(const std::vector<Token>& tokens);
    
    // Форматирование грамматик
    std::string formatGrammar(const Grammar& grammar);
    
    // Экспорт грамматик в различные форматы
    std::string exportToEBNF(const Grammar& grammar);
    std::string exportToBNF(const Grammar& grammar);
    std::string exportToRegex(const Grammar& grammar, const std::string& ruleName);
}

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
};

} // namespace bnf_parser
