#include "bnf_parser.hpp"
#include <iostream>

int main() {
    std::cout << "=== BNF Parser - Clojure (EDN) Example ===" << std::endl;
    
    try {
        auto parser = bnf_parser::Parser::create();
        auto grammar = bnf_parser::Grammar::clojure();
        auto tokenizer = parser->createTokenizer(*grammar);
        
        std::vector<std::string> clojure_examples = {
            "(+ 1 2 3)",
            "[1 2 3 :keyword]",
            "{:name \"John\" :age 30}",
            "#{1 2 3}",
            "'(quoted list)",
            "@atom-ref",
            "^{:meta true} symbol",
            "true false nil"
        };
        
        for (const auto& clj : clojure_examples) {
            std::cout << "\nClojure: " << clj << std::endl;
            
            auto tokens = tokenizer->tokenize(clj);
            
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
