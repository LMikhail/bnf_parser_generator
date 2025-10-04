#include "bnf_parser.hpp"
#include <iostream>
#include <iomanip>

void printSeparator(const std::string& title) {
    std::cout << "\n" << std::string(60, '=') << std::endl;
    std::cout << "  " << title << std::endl;
    std::cout << std::string(60, '=') << std::endl;
}

void printTokens(const std::vector<bnf_parser::Token>& tokens) {
    std::cout << std::left << std::setw(15) << "Type" 
              << std::setw(20) << "Value" 
              << std::setw(8) << "Line" 
              << "Column" << std::endl;
    std::cout << std::string(50, '-') << std::endl;
    
    for (const auto& token : tokens) {
        if (token.type != "EOF") {
            std::cout << std::left << std::setw(15) << token.type
                      << std::setw(20) << ("'" + token.value + "'")
                      << std::setw(8) << token.line
                      << token.column << std::endl;
        }
    }
}

int main() {
    printSeparator("BNF Parser 2.0 - Полная поддержка BNF/EBNF");
    
    try {
        // Создаём парсер
        auto parser = bnf_parser::Parser::create();
        
        std::cout << "Версия: " << parser->getVersion() << std::endl;
        std::cout << "Поддерживаемые возможности: " << parser->getSupportedFeatures().size() << std::endl;
        
        // Демонстрация 1: Простая арифметическая грамматика
        printSeparator("Демонстрация 1: Арифметические выражения");
        
        auto arithmeticGrammar = parser->createArithmeticGrammar();
        std::cout << "Грамматика:\n" << bnf_parser::utils::formatGrammar(*arithmeticGrammar) << std::endl;
        
        // Валидация
        auto validation = parser->validateGrammar(*arithmeticGrammar);
        if (validation.isValid) {
            std::cout << "✅ Грамматика валидна!" << std::endl;
        } else {
            std::cout << "❌ Ошибки валидации:" << std::endl;
            for (const auto& error : validation.errors) {
                std::cout << "  " << error << std::endl;
            }
        }
        
        // Создаём токенизатор
        auto tokenizer = parser->createTokenizer(*arithmeticGrammar);
        
        // Тестируем различные выражения
        std::vector<std::string> expressions = {
            "2 + 3 * 4",
            "(10 - 5) / 2",
            "123 + 456 * 789"
        };
        
        for (const auto& expr : expressions) {
            std::cout << "\nВыражение: " << expr << std::endl;
            auto tokens = tokenizer->tokenize(expr);
            
            if (!tokenizer->getLastError().empty()) {
                std::cout << "❌ Ошибка: " << tokenizer->getLastError() << std::endl;
                continue;
            }
            
            printTokens(tokens);
            
            // Статистика
            auto stats = bnf_parser::utils::analyzeTokens(tokens);
            std::cout << "Статистика: " << stats.total_tokens << " токенов, " 
                      << stats.unique_types << " типов" << std::endl;
        }
        
        // Демонстрация 2: Пользовательская BNF грамматика
        printSeparator("Демонстрация 2: Пользовательская грамматика");
        
        const std::string customBNF = R"(
# Простая грамматика для переменных и присваиваний
program ::= statement+
statement ::= assignment | expression
assignment ::= identifier '=' expression
expression ::= term (('+'|'-') term)*
term ::= factor (('*'|'/') factor)*
factor ::= number | identifier | '(' expression ')'
identifier ::= letter (letter | digit)*
number ::= digit+
letter ::= 'a'..'z' | 'A'..'Z'
digit ::= '0'..'9'
)";
        
        std::cout << "Пользовательская BNF грамматика:" << std::endl;
        std::cout << customBNF << std::endl;
        
        auto customGrammar = parser->parseGrammarFromString(customBNF);
        auto customValidation = parser->validateGrammar(*customGrammar);
        
        if (customValidation.isValid) {
            std::cout << "✅ Пользовательская грамматика валидна!" << std::endl;
            
            auto customTokenizer = parser->createTokenizer(*customGrammar);
            
            std::vector<std::string> testPrograms = {
                "x = 10 + 20",
                "result = (a + b) * c",
                "value = 123"
            };
            
            for (const auto& program : testPrograms) {
                std::cout << "\nПрограмма: " << program << std::endl;
                auto tokens = customTokenizer->tokenize(program);
                
                if (!customTokenizer->getLastError().empty()) {
                    std::cout << "❌ Ошибка: " << customTokenizer->getLastError() << std::endl;
                    continue;
                }
                
                printTokens(tokens);
            }
        } else {
            std::cout << "❌ Ошибки в пользовательской грамматике:" << std::endl;
            for (const auto& error : customValidation.errors) {
                std::cout << "  " << error << std::endl;
            }
        }
        
        // Демонстрация 3: Экспорт грамматик
        printSeparator("Демонстрация 3: Экспорт грамматик");
        
        std::cout << "Экспорт в EBNF:" << std::endl;
        std::cout << bnf_parser::utils::exportToEBNF(*arithmeticGrammar) << std::endl;
        
        std::cout << "Экспорт в классическую BNF:" << std::endl;
        std::cout << bnf_parser::utils::exportToBNF(*arithmeticGrammar) << std::endl;
        
        // Демонстрация 4: Предустановленные грамматики
        printSeparator("Демонстрация 4: Предустановленные грамматики");
        
        auto jsonGrammar = parser->createJSONGrammar();
        std::cout << "JSON грамматика загружена: " 
                  << jsonGrammar->rules.size() << " правил" << std::endl;
        
        auto jsonTokenizer = parser->createTokenizer(*jsonGrammar);
        
        std::string jsonExample = R"({"name": "test", "value": 42})";
        std::cout << "\nJSON пример: " << jsonExample << std::endl;
        
        auto jsonTokens = jsonTokenizer->tokenize(jsonExample);
        if (!jsonTokenizer->getLastError().empty()) {
            std::cout << "❌ Ошибка: " << jsonTokenizer->getLastError() << std::endl;
        } else {
            printTokens(jsonTokens);
        }
        
        printSeparator("Демонстрация завершена успешно!");
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Критическая ошибка: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
