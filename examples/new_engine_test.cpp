#include "bnf_parser.hpp"
#include <iostream>

int main() {
    std::cout << "=== Новый BNF Parser 2.0 - Тест ===" << std::endl;
    
    try {
        // Создаём новый парсер
        auto parser = bnf_parser::Parser::create();
        
        std::cout << "Версия парсера: " << parser->getVersion() << std::endl;
        std::cout << "Поддерживаемые функции:" << std::endl;
        
        auto features = parser->getSupportedFeatures();
        for (const auto& feature : features) {
            std::cout << "  - " << feature << std::endl;
        }
        
        // Тестируем простую арифметическую грамматику
        std::cout << "\n=== Тест арифметической грамматики ===" << std::endl;
        
        auto grammar = bnf_parser::BNFGrammar::arithmetic();
        std::cout << "Грамматика загружена: " << grammar->toString() << std::endl;
        
        // Валидация грамматики
        auto validation = grammar->validate();
        if (validation.isValid) {
            std::cout << "✅ Грамматика валидна!" << std::endl;
        } else {
            std::cout << "❌ Ошибки валидации:" << std::endl;
            for (const auto& error : validation.errors) {
                std::cout << "  " << error << std::endl;
            }
        }
        
        if (!validation.warnings.empty()) {
            std::cout << "⚠️ Предупреждения:" << std::endl;
            for (const auto& warning : validation.warnings) {
                std::cout << "  " << warning << std::endl;
            }
        }
        
        // Создаём токенизатор
        auto tokenizer = engine->createTokenizer(*grammar);
        
        // Тестируем токенизацию
        std::vector<std::string> testExpressions = {
            "2 + 3",
            "10 * (5 - 2)",
            "123 / 456"
        };
        
        for (const auto& expr : testExpressions) {
            std::cout << "\nВыражение: " << expr << std::endl;
            
            auto tokens = tokenizer->tokenize(expr);
            
            if (!tokenizer->getLastError().empty()) {
                std::cout << "Ошибка: " << tokenizer->getLastError() << std::endl;
                continue;
            }
            
            std::cout << "Токены:" << std::endl;
            for (const auto& token : tokens) {
                if (token.type != "EOF") {
                    std::cout << "  " << token.type << ": '" << token.value << "'" << std::endl;
                }
            }
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
