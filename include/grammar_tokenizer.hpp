#pragma once

#include "bnf_ast.hpp"
#include <string>
#include <vector>
#include <memory>
#include <regex>
#include <unordered_map>

namespace bnf_parser {

// Forward declarations для избежания циклических зависимостей
struct Token {
    std::string type;
    std::string value;
    size_t line;
    size_t column;
    size_t position;
    
    Token(const std::string& t, const std::string& v, size_t l, size_t c, size_t p)
        : type(t), value(v), line(l), column(c), position(p) {}
};

class Tokenizer {
public:
    virtual ~Tokenizer() = default;
    virtual std::vector<Token> tokenize(const std::string& input) = 0;
    virtual std::string getLastError() const = 0;
    virtual void setSkipWhitespace(bool skip) = 0;
    virtual void setSkipComments(bool skip) = 0;
};

/**
 * Токенизатор, генерируемый из BNF/EBNF грамматики
 * Реализует настоящее распознавание токенов согласно правилам грамматики
 */
class GrammarBasedTokenizer : public Tokenizer {
private:
    const Grammar& grammar_;
    std::string lastError_;
    bool skipWhitespace_ = true;
    bool skipComments_ = true;
    
    // Кэш скомпилированных регулярных выражений для терминалов
    mutable std::unordered_map<std::string, std::regex> terminalRegexCache_;
    
    // Кэш правил для нетерминалов
    mutable std::unordered_map<std::string, std::string> nonTerminalPatterns_;

public:
    explicit GrammarBasedTokenizer(const Grammar& grammar);
    
    std::vector<Token> tokenize(const std::string& input) override;
    std::string getLastError() const override { return lastError_; }
    void setSkipWhitespace(bool skip) override { skipWhitespace_ = skip; }
    void setSkipComments(bool skip) override { skipComments_ = skip; }

private:
    // Генерация регулярных выражений из AST узлов
    std::string generateRegex(const ASTNode* node) const;
    std::string generateRegexWithDepth(const ASTNode* node, int depth) const;
    std::string generateTerminalRegex(const Terminal* terminal) const;
    std::string generateCharRangeRegex(const CharRange* range) const;
    std::string generateAlternativeRegex(const Alternative* alt) const;
    std::string generateSequenceRegex(const Sequence* seq) const;
    std::string generateOptionalRegex(const Optional* opt) const;
    std::string generateZeroOrMoreRegex(const ZeroOrMore* zeroMore) const;
    std::string generateOneOrMoreRegex(const OneOrMore* oneMore) const;
    std::string generateGroupRegex(const Group* group) const;
    
    // Попытка сопоставления токена
    struct MatchResult {
        bool matched;
        std::string tokenType;
        std::string value;
        size_t length;
        
        MatchResult() : matched(false), length(0) {}
        MatchResult(const std::string& type, const std::string& val, size_t len)
            : matched(true), tokenType(type), value(val), length(len) {}
    };
    
    MatchResult tryMatchRule(const std::string& input, size_t pos, const std::string& ruleName) const;
    MatchResult tryMatchNode(const std::string& input, size_t pos, const ASTNode* node, const std::string& ruleName) const;
    
    // Утилиты
    std::string escapeRegex(const std::string& str) const;
    bool isWhitespace(char c) const;
    void skipWhitespaceAndComments(const std::string& input, size_t& pos, size_t& line, size_t& column) const;
    
    // Определение типов токенов
    std::vector<std::string> getTokenTypes() const;
    bool isTerminalRule(const std::string& ruleName) const;
    bool isTerminalNode(const ASTNode* node) const;
};

/**
 * Фабрика для создания токенизаторов из грамматик
 */
class TokenizerFactory {
public:
    // Создать токенизатор из грамматики
    static std::unique_ptr<Tokenizer> createFromGrammar(const Grammar& grammar);
    
    // Создать токенизатор из BNF-файла
    static std::unique_ptr<Tokenizer> createFromFile(const std::string& filename);
    
    // Создать токенизатор из BNF-строки
    static std::unique_ptr<Tokenizer> createFromString(const std::string& bnfText);
};

} // namespace bnf_parser
