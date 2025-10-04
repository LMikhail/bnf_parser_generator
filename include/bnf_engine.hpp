#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

// Подключаем новые компоненты
#include "bnf_ast.hpp"
#include "bnf_parser.hpp"
#include "grammar_tokenizer.hpp"

namespace bnf_parser {

// Tokenizer определён в grammar_tokenizer.hpp

/**
 * Парсер BNF/EBNF - главный интерфейс для работы с грамматиками
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

} // namespace bnf_parser
