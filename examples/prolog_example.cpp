#include "bnf_parser.hpp"
#include <iostream>

int main() {
    std::cout << "=== BNF Parser - Prolog Example ===" << std::endl;
    
    try {
        // Создаём парсер и грамматику
        auto parser = bnf_parser::Parser::create();
        auto grammar = bnf_parser::Grammar::prolog();
        auto tokenizer = parser->createTokenizer(*grammar);
        
        // Тестируем различные Prolog конструкции
        std::vector<std::string> test_cases = {
            "person(john).",
            "parent(X, Y) :- father(X, Y).",
            "X = john",
            "X is 2 + 3",
            "[1, 2, 3]",
            "member(X, [H|T])"
        };
        
        for (const auto& test : test_cases) {
            std::cout << "\nВход: " << test << std::endl;
            
            auto tokens = tokenizer->tokenize(test);
            
            if (!tokenizer->getLastError().empty()) {
                std::cout << "Ошибка: " << tokenizer->getLastError() << std::endl;
                continue;
            }
            
            std::cout << "Токены (" << tokens.size() << "):" << std::endl;
            for (const auto& token : tokens) {
                if (token.type != "EOF") {
                    std::cout << "  " << token.type << ": '" << token.value << "'" << std::endl;
                }
            }
            
            // Статистика
            auto stats = bnf_engine::utils::analyzeTokens(tokens);
            std::cout << "Статистика: " << stats.total_tokens << " токенов, " 
                      << stats.unique_types << " типов" << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
