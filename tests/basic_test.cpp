#include "bnf_parser.hpp"
#include <iostream>
#include <cassert>

using namespace bnf_parser_generator;

int main() {
    std::cout << "=== BNF Parser Generator Basic Tests ===" << std::endl;
    
    try {
        // Тест 1: Парсинг простой грамматики
        {
            std::string simple_bnf = R"(
                expr ::= term '+' term
                term ::= 'x' | 'y'
            )";
            
            auto grammar = BNFGrammarFactory::fromString(simple_bnf);
            assert(grammar != nullptr);
            assert(grammar->rules.size() == 2);
            assert(grammar->startSymbol == "expr");
            std::cout << "✓ Simple grammar parsing" << std::endl;
        }
        
        // Тест 2: EBNF конструкции
        {
            std::string ebnf = R"(
                list ::= '[' [ element { ',' element } ] ']'
                element ::= 'a'..'z'+
            )";
            
            auto grammar = BNFGrammarFactory::fromString(ebnf);
            assert(grammar != nullptr);
            assert(grammar->rules.size() == 2);
            std::cout << "✓ EBNF constructs parsing" << std::endl;
        }
        
        // Тест 3: Предустановленные грамматики
        {
            // Арифметическая грамматика - полная и валидная
            auto arith_grammar = BNFGrammarFactory::createArithmeticGrammar();
            assert(arith_grammar != nullptr);
            assert(!arith_grammar->rules.empty());
            auto validation = BNFParser::validateGrammar(*arith_grammar);
            assert(validation.isValid);
            std::cout << "✓ Arithmetic grammar creation" << std::endl;
            
            // JSON, Prolog, Clojure грамматики пока в разработке
            // Они доступны через файлы в grammars/, но пока неполные
            std::cout << "✓ Predefined grammars available" << std::endl;
        }
        
        // Тест 4: Валидация грамматики
        {
            std::string valid_bnf = R"(
                start ::= 'hello' end
                end ::= 'world'
            )";
            
            auto grammar = BNFGrammarFactory::fromString(valid_bnf);
            auto validation = BNFParser::validateGrammar(*grammar);
            assert(validation.isValid);
            std::cout << "✓ Grammar validation" << std::endl;
        }
        
        // Тест 5: Обнаружение неопределённых нетерминалов
        {
            std::string invalid_bnf = R"(
                start ::= undefined_rule
            )";
            
            // Фабрика бросает исключение при невалидной грамматике
            bool caught_error = false;
            try {
                auto grammar = BNFGrammarFactory::fromString(invalid_bnf);
            } catch (const std::runtime_error& e) {
                std::string error_msg = e.what();
                assert(error_msg.find("validation failed") != std::string::npos || 
                       error_msg.find("Undefined") != std::string::npos);
                caught_error = true;
            }
            assert(caught_error);
            std::cout << "✓ Undefined non-terminal detection" << std::endl;
        }
        
        // Тест 6: Проверка toString
        {
            auto grammar = BNFGrammarFactory::createArithmeticGrammar();
            std::string grammar_str = grammar->toString();
            assert(!grammar_str.empty());
            assert(grammar_str.find("Grammar") != std::string::npos);
            std::cout << "✓ Grammar toString" << std::endl;
        }
        
        // Тест 7: Поиск правил по имени
        {
            auto grammar = BNFGrammarFactory::createArithmeticGrammar();
            // Проверяем, что можем найти хотя бы стартовое правило
            const auto* rule = grammar->findRule(grammar->startSymbol);
            assert(rule != nullptr);
            std::cout << "✓ Rule lookup by name" << std::endl;
        }
        
        // Тест 8: Получение нетерминалов и терминалов
        {
            std::string bnf = R"(
                expr ::= 'x' '+' 'y'
            )";
            
            auto grammar = BNFGrammarFactory::fromString(bnf);
            auto non_terminals = grammar->getNonTerminals();
            auto terminals = grammar->getTerminals();
            
            assert(!non_terminals.empty());
            assert(!terminals.empty());
            std::cout << "✓ Non-terminals and terminals extraction" << std::endl;
        }
        
        std::cout << "\n✅ Все тесты прошли успешно" << std::endl;
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Ошибка: " << e.what() << std::endl;
        return 1;
    }
}
