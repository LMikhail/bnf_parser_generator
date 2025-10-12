#include "bnf_parser.hpp"
#include "code_generator.hpp"
#include <iostream>
#include <cassert>

using namespace bnf_parser_generator;

int main() {
    std::cout << "=== Extended BNF Tests ===" << std::endl;
    
    try {
        // Тест 1: Парсинг параметризованного правила
        {
            std::string extended_bnf = R"(
                agreement[N:enum{sing,plur}] ::= noun[N] verb[N]
                noun[sing] ::= "cat" | "dog"
                noun[plur] ::= "cats" | "dogs"
                verb[sing] ::= "runs" | "jumps"
                verb[plur] ::= "run" | "jump"
            )";
            
            auto grammar = BNFGrammarFactory::fromString(extended_bnf);
            assert(grammar != nullptr);
            assert(grammar->rules.size() == 5);
            assert(grammar->isContextSensitive());
            
            // Проверяем первое правило с параметрами
            auto* agreement_rule = grammar->findRule("agreement");
            assert(agreement_rule != nullptr);
            assert(agreement_rule->hasParameters());
            assert(agreement_rule->parameters.size() == 1);
            assert(agreement_rule->parameters[0].name == "N");
            assert(agreement_rule->parameters[0].type == ParameterType::ENUM);
            assert(agreement_rule->parameters[0].enumValues.size() == 2);
            assert(agreement_rule->parameters[0].enumValues[0] == "sing");
            assert(agreement_rule->parameters[0].enumValues[1] == "plur");
            
            std::cout << "✓ Parameterized rule parsing" << std::endl;
        }
        
        // Тест 2: Генерация C++ кода для параметризованных правил
        {
            std::string extended_bnf = R"(
                agreement[N:enum{sing,plur}] ::= noun[N] verb[N]
                noun[sing] ::= "cat"
                noun[plur] ::= "cats"
                verb[sing] ::= "runs"
                verb[plur] ::= "run"
            )";
            
            auto grammar = BNFGrammarFactory::fromString(extended_bnf);
            auto generator = CodeGeneratorFactory::create("cpp");
            
            GeneratorOptions options;
            options.parser_name = "AgreementParser";
            
            auto result = generator->generate(*grammar, options);
            assert(result.success);
            assert(!result.parser_code.empty());
            
            // Проверяем наличие enum в сгенерированном коде
            assert(result.parser_code.find("enum class NEnum") != std::string::npos);
            assert(result.parser_code.find("sing") != std::string::npos);
            assert(result.parser_code.find("plur") != std::string::npos);
            
            // Проверяем наличие параметризованных функций
            assert(result.parser_code.find("parse_agreement(NEnum N)") != std::string::npos);
            assert(result.parser_code.find("parse_noun(NEnum N)") != std::string::npos);
            assert(result.parser_code.find("parse_verb(NEnum N)") != std::string::npos);
            
            std::cout << "✓ Parameterized code generation" << std::endl;
        }
        
        // Тест 3: Контекстные действия
        {
            std::string context_bnf = R"(
                document ::= (anchor | reference)*
                anchor ::= "&" name content {store(name, content)}
                reference ::= "*" name {lookup(name)}
                name ::= 'a'..'z'+
                content ::= 'A'..'Z'+
            )";
            
            auto grammar = BNFGrammarFactory::fromString(context_bnf);
            assert(grammar != nullptr);
            assert(grammar->isContextSensitive());
            
            // Проверяем, что грамматика контекстно-зависимая
            std::cout << "✓ Context actions parsing" << std::endl;
        }
        
        // Тест 4: Генерация кода с контекстными действиями
        {
            std::string context_bnf = R"(
                document ::= anchor reference
                anchor ::= "&name" "value" {store(name, value)}
                reference ::= "*name" {lookup(name)}
            )";
            
            auto grammar = BNFGrammarFactory::fromString(context_bnf);
            auto generator = CodeGeneratorFactory::create("cpp");
            
            GeneratorOptions options;
            options.parser_name = "ContextParser";
            
            auto result = generator->generate(*grammar, options);
            assert(result.success);
            
            // Проверяем наличие контекстного хранилища
            assert(result.parser_code.find("context_storage_") != std::string::npos);
            assert(result.parser_code.find("std::unordered_map") != std::string::npos);
            
            // Проверяем генерацию store и lookup действий
            assert(result.parser_code.find("context_storage_[") != std::string::npos);
            assert(result.parser_code.find("context_storage_.find") != std::string::npos);
            
            std::cout << "✓ Context actions code generation" << std::endl;
        }
        
        // Тест 5: Indentation грамматика (Python-style)
        {
            std::string indent_bnf = R"(
                block[indent:int] ::= line[indent] (newline line[indent])*
                line[indent] ::= spaces[indent] statement
                spaces[0] ::= ""
                spaces[n:int] ::= "    " spaces[n-1]
                statement ::= "print" "hello"
                newline ::= "\n"
            )";
            
            auto grammar = BNFGrammarFactory::fromString(indent_bnf);
            assert(grammar != nullptr);
            assert(grammar->isContextSensitive());
            
            // Проверяем параметр типа int
            auto* block_rule = grammar->findRule("block");
            assert(block_rule != nullptr);
            assert(block_rule->hasParameters());
            assert(block_rule->parameters[0].type == ParameterType::INTEGER);
            
            std::cout << "✓ Indentation grammar parsing" << std::endl;
        }
        
        // Тест 6: Валидация Extended BNF грамматик
        {
            std::string invalid_bnf = R"(
                rule[param] ::= other[undefined_param]
            )";
            
            try {
                auto grammar = BNFGrammarFactory::fromString(invalid_bnf);
                // Должна быть ошибка валидации
                assert(false && "Expected validation error");
            } catch (const std::runtime_error& e) {
                std::string error_msg = e.what();
                assert(error_msg.find("undefined_param") != std::string::npos || 
                       error_msg.find("validation") != std::string::npos);
                std::cout << "✓ Extended BNF validation" << std::endl;
            }
        }
        
        // Тест 7: Смешанные параметризованные и обычные правила
        {
            std::string mixed_bnf = R"(
                program ::= agreement statement
                agreement[N:enum{sing,plur}] ::= noun[N] verb[N]
                noun[sing] ::= "cat"
                noun[plur] ::= "cats"
                verb[sing] ::= "runs"
                verb[plur] ::= "run"
                statement ::= "end"
            )";
            
            auto grammar = BNFGrammarFactory::fromString(mixed_bnf);
            assert(grammar != nullptr);
            assert(grammar->isContextSensitive());
            
            // Проверяем наличие как параметризованных, так и обычных правил
            auto parameterized_rules = grammar->getParameterizedRules();
            assert(parameterized_rules.size() == 3); // agreement, noun, verb
            
            auto* program_rule = grammar->findRule("program");
            assert(program_rule != nullptr);
            assert(!program_rule->hasParameters()); // обычное правило
            
            std::cout << "✓ Mixed parameterized and regular rules" << std::endl;
        }
        
        // Тест 8: Генерация enum типов
        {
            std::string enum_bnf = R"(
                test[T:enum{type1,type2,type3}] ::= "value"
            )";
            
            auto grammar = BNFGrammarFactory::fromString(enum_bnf);
            auto generator = CodeGeneratorFactory::create("cpp");
            
            GeneratorOptions options;
            options.parser_name = "EnumTestParser";
            
            auto result = generator->generate(*grammar, options);
            assert(result.success);
            
            // Проверяем правильную генерацию enum
            assert(result.parser_code.find("enum class TEnum") != std::string::npos);
            assert(result.parser_code.find("type1") != std::string::npos);
            assert(result.parser_code.find("type2") != std::string::npos);
            assert(result.parser_code.find("type3") != std::string::npos);
            
            std::cout << "✓ Enum type generation" << std::endl;
        }
        
        std::cout << "\n✅ Все Extended BNF тесты прошли успешно" << std::endl;
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Ошибка: " << e.what() << std::endl;
        return 1;
    }
}

// Вспомогательная функция для поиска контекстных действий
bool hasContextActionsInRule(const ASTNode* node) {
    if (dynamic_cast<const ContextAction*>(node)) {
        return true;
    }
    
    if (const auto* seq = dynamic_cast<const Sequence*>(node)) {
        for (const auto& elem : seq->elements) {
            if (hasContextActionsInRule(elem.get())) return true;
        }
    }
    
    if (const auto* alt = dynamic_cast<const Alternative*>(node)) {
        for (const auto& choice : alt->choices) {
            if (hasContextActionsInRule(choice.get())) return true;
        }
    }
    
    if (const auto* grp = dynamic_cast<const Group*>(node)) {
        return hasContextActionsInRule(grp->content.get());
    }
    
    if (const auto* opt = dynamic_cast<const Optional*>(node)) {
        return hasContextActionsInRule(opt->content.get());
    }
    
    if (const auto* zom = dynamic_cast<const ZeroOrMore*>(node)) {
        return hasContextActionsInRule(zom->content.get());
    }
    
    if (const auto* oom = dynamic_cast<const OneOrMore*>(node)) {
        return hasContextActionsInRule(oom->content.get());
    }
    
    return false;
}
