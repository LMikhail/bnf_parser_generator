#include "bnf_parser.hpp"
#include "code_generator.hpp"
#include <iostream>
#include <cassert>

using namespace bnf_parser_generator;

int main() {
    std::cout << "=== Code Generator Tests ===" << std::endl;
    
    try {
        // Тест 1: Создание генератора для C++
        {
            auto generator = CodeGeneratorFactory::create("cpp");
            assert(generator != nullptr);
            assert(generator->getTargetLanguage() == "cpp");
            assert(generator->getFileExtension() == ".cpp");
            std::cout << "✓ C++ generator creation" << std::endl;
        }
        
        // Тест 2: Проверка поддерживаемых языков
        {
            auto languages = CodeGeneratorFactory::getSupportedLanguages();
            assert(!languages.empty());
            assert(CodeGeneratorFactory::isLanguageSupported("cpp"));
            std::cout << "✓ Supported languages check" << std::endl;
        }
        
        // Тест 3: Генерация кода из простой грамматики
        {
            std::string simple_bnf = R"(
                expr ::= term '+' term;
                term ::= 'x' | 'y';
            )";
            
            auto grammar = BNFGrammarFactory::fromString(simple_bnf);
            assert(grammar != nullptr);
            
            auto generator = CodeGeneratorFactory::create("cpp");
            GeneratorOptions options;
            options.parser_name = "SimpleParser";
            
            auto result = generator->generate(*grammar, options);
            assert(result.success);
            assert(!result.parser_code.empty());
            assert(result.parser_filename == "simple_parser.cpp");
            std::cout << "✓ Simple grammar code generation" << std::endl;
        }
        
        // Тест 4: Генерация с namespace
        {
            std::string bnf = R"(
                start ::= 'hello' 'world';
            )";
            
            auto grammar = BNFGrammarFactory::fromString(bnf);
            auto generator = CodeGeneratorFactory::create("cpp");
            
            GeneratorOptions options;
            options.parser_name = "HelloParser";
            options.namespace_name = "hello";
            
            auto result = generator->generate(*grammar, options);
            assert(result.success);
            assert(result.parser_code.find("namespace hello") != std::string::npos);
            std::cout << "✓ Namespace generation" << std::endl;
        }
        
        // Тест 5: EBNF конструкции
        {
            std::string ebnf = R"(
                list ::= '[' [ element { ',' element } ] ']';
                element ::= 'a'..'z'+;
            )";
            
            auto grammar = BNFGrammarFactory::fromString(ebnf);
            auto generator = CodeGeneratorFactory::create("cpp");
            
            GeneratorOptions options;
            options.parser_name = "ListParser";
            
            auto result = generator->generate(*grammar, options);
            assert(result.success);
            assert(!result.parser_code.empty());
            
            // Проверим наличие ключевых компонентов в сгенерированном коде
            assert(result.parser_code.find("class ListParser") != std::string::npos);
            assert(result.parser_code.find("parse_list") != std::string::npos);
            assert(result.parser_code.find("parse_element") != std::string::npos);
            std::cout << "✓ EBNF constructs code generation" << std::endl;
        }
        
        // Тест 6: Генерация с отладочным режимом
        {
            std::string bnf = R"(
                expr ::= 'x';
            )";
            
            auto grammar = BNFGrammarFactory::fromString(bnf);
            auto generator = CodeGeneratorFactory::create("cpp");
            
            GeneratorOptions options;
            options.debug_mode = true;
            
            auto result = generator->generate(*grammar, options);
            assert(result.success);
            std::cout << "✓ Debug mode generation" << std::endl;
        }
        
        // Тест 7: Проверка AST классов в сгенерированном коде
        {
            std::string bnf = R"(
                start ::= 'test';
            )";
            
            auto grammar = BNFGrammarFactory::fromString(bnf);
            auto generator = CodeGeneratorFactory::create("cpp");
            
            GeneratorOptions options;
            
            auto result = generator->generate(*grammar, options);
            assert(result.success);
            
            // Проверим наличие базового класса ASTNode
            assert(result.parser_code.find("class ASTNode") != std::string::npos);
            assert(result.parser_code.find("virtual ~ASTNode()") != std::string::npos);
            std::cout << "✓ AST class generation" << std::endl;
        }
        
        // Тест 8: Обработка ошибок валидации
        {
            std::string invalid_bnf = R"(
                start ::= undefined_rule;
            )";
            
            // Фабрика бросает исключение при невалидной грамматике
            [[maybe_unused]] bool caught_error = false;
            try {
                auto grammar = BNFGrammarFactory::fromString(invalid_bnf);
                (void)grammar; // Подавляем предупреждение
            } catch (const std::runtime_error&) {
                caught_error = true;
            }
            assert(caught_error && "Expected to catch error for invalid grammar");
            std::cout << "✓ Invalid grammar handling" << std::endl;
        }
        
        // Тест 9: Предустановленные грамматики
        {
            auto arith_grammar = BNFGrammarFactory::createArithmeticGrammar();
            auto generator = CodeGeneratorFactory::create("cpp");
            
            GeneratorOptions options;
            options.parser_name = "ArithmeticParser";
            
            auto result = generator->generate(*arith_grammar, options);
            assert(result.success);
            assert(!result.parser_code.empty());
            std::cout << "✓ Arithmetic grammar code generation" << std::endl;
        }
        
        // Тест 10: Проверка сообщений и предупреждений
        {
            std::string bnf = R"(
                expr ::= 'x' | 'y';
            )";
            
            auto grammar = BNFGrammarFactory::fromString(bnf);
            auto generator = CodeGeneratorFactory::create("cpp");
            
            GeneratorOptions options;
            
            auto result = generator->generate(*grammar, options);
            assert(result.success);
            assert(!result.messages.empty());
            std::cout << "✓ Messages and warnings" << std::endl;
        }
        
        std::cout << "\n✅ Все тесты генератора прошли успешно" << std::endl;
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Ошибка: " << e.what() << std::endl;
        return 1;
    }
}

