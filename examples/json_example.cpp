#include "bnf_parser.hpp"
#include <iostream>

int main() {
    std::cout << "=== BNF Parser - JSON Example ===" << std::endl;
    
    try {
        auto parser = bnf_parser::Parser::create();
        auto grammar = bnf_parser::Grammar::json();
        auto tokenizer = parser->createTokenizer(*grammar);
        
        std::vector<std::string> json_examples = {
            R"({"name": "John", "age": 30})",
            R"([1, 2, 3, "hello"])",
            R"({"nested": {"key": "value"}})",
            R"(true)",
            R"(null)",
            R"(42.5)"
        };
        
        for (const auto& json : json_examples) {
            std::cout << "\nJSON: " << json << std::endl;
            
            auto tokens = tokenizer->tokenize(json);
            
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
