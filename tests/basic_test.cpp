#include "bnf_parser.hpp"
#include <iostream>
#include <cassert>

int main() {
    std::cout << "=== BNF Parser Basic Tests ===" << std::endl;
    
    try {
        // Тест создания парсера
        auto parser = bnf_parser::Parser::create();
        assert(parser != nullptr);
        std::cout << "✓ Parser creation" << std::endl;
        
        // Тест Prolog грамматики
        auto prolog_grammar = parser->createPrologGrammar();
        assert(prolog_grammar != nullptr);
        auto validation = parser->validateGrammar(*prolog_grammar);
        assert(validation.isValid);
        std::cout << "✓ Prolog grammar" << std::endl;
        
        // Тест токенизатора
        auto tokenizer = parser->createTokenizer(*prolog_grammar);
        assert(tokenizer != nullptr);
        std::cout << "✓ Tokenizer creation" << std::endl;
        
        // Тест простой токенизации
        auto tokens = tokenizer->tokenize("person(john).");
        assert(!tokens.empty());
        assert(tokenizer->getLastError().empty());
        std::cout << "✓ Basic tokenization (" << tokens.size() << " tokens)" << std::endl;
        
        // Тест JSON
        auto json_grammar = parser->createJSONGrammar();
        auto json_tokenizer = parser->createTokenizer(*json_grammar);
        auto json_tokens = json_tokenizer->tokenize(R"({"key": "value"})");
        assert(!json_tokens.empty());
        std::cout << "✓ JSON tokenization (" << json_tokens.size() << " tokens)" << std::endl;
        
        // Тест Clojure
        auto clj_grammar = parser->createClojureGrammar();
        auto clj_tokenizer = parser->createTokenizer(*clj_grammar);
        auto clj_tokens = clj_tokenizer->tokenize("(+ 1 2)");
        assert(!clj_tokens.empty());
        std::cout << "✓ Clojure tokenization (" << clj_tokens.size() << " tokens)" << std::endl;
        
        std::cout << "\nВсе тесты прошли успешно" << std::endl;
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
        return 1;
    }
}
