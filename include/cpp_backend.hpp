#pragma once

#include "code_generator.hpp"
#include <sstream>
#include <unordered_set>

namespace bnf_parser_generator {

/**
 * Генератор C++ кода для парсеров
 * Создаёт standalone парсер с рекурсивным спуском и естественным backtracking
 */
class CppCodeGenerator : public CodeGenerator {
private:
    GeneratorOptions options_;
    std::ostringstream header_includes_;
    std::ostringstream type_definitions_;
    std::ostringstream parser_functions_;
    std::ostringstream helper_functions_;
    
    // Отслеживание сгенерированных функций для избежания дубликатов
    std::unordered_set<std::string> generated_functions_;
    
    // Текущий уровень отступа
    size_t current_indent_level_ = 0;
    
    // Счетчик для уникальных имен переменных
    mutable size_t variable_counter_ = 0;

public:
    CppCodeGenerator() = default;
    
    GeneratedCode generate(const Grammar& grammar, const GeneratorOptions& options) override;
    
    std::string getTargetLanguage() const override { return "cpp"; }
    std::string getFileExtension() const override { return ".cpp"; }
    
    std::vector<std::string> getSupportedFeatures() const override {
        return {
            "recursive_descent",
            "natural_backtracking",
            "ast_construction",
            "error_reporting",
            "position_tracking",
            "utf8_support",
            "standalone_code"
        };
    }

protected:
    // Реализация visitor методов для AST узлов (с действием при ошибке)
    std::string visitTerminal(const Terminal* node, const std::string& on_failure_action);
    std::string visitNonTerminal(const NonTerminal* node, const std::string& on_failure_action);
    std::string visitCharRange(const CharRange* node, const std::string& on_failure_action);
    std::string visitAlternative(const Alternative* node, const std::string& on_failure_action);
    std::string visitSequence(const Sequence* node, const std::string& on_failure_action);
    std::string visitGroup(const Group* node, const std::string& on_failure_action);
    std::string visitOptional(const Optional* node, const std::string& on_failure_action);
    std::string visitZeroOrMore(const ZeroOrMore* node, const std::string& on_failure_action);
    std::string visitOneOrMore(const OneOrMore* node, const std::string& on_failure_action);
    
    // Extended BNF visitor методы
    std::string visitContextAction(const ContextAction* node, const std::string& on_failure_action);
    
    // Генерация функций для правил
    std::string generateRuleFunction(const ProductionRule& rule) override;

private:
    // Реализация чисто виртуальных методов из CodeGenerator
    std::string visitTerminal(const Terminal* node) override { 
        return visitTerminal(node, "return nullptr;"); 
    }
    std::string visitNonTerminal(const NonTerminal* node) override {
        return visitNonTerminal(node, "return nullptr;");
    }
    std::string visitCharRange(const CharRange* node) override {
        return visitCharRange(node, "return nullptr;");
    }
    std::string visitAlternative(const Alternative* node) override {
        return visitAlternative(node, "return nullptr;");
    }
    std::string visitSequence(const Sequence* node) override {
        return visitSequence(node, "return nullptr;");
    }
    std::string visitGroup(const Group* node) override {
        return visitGroup(node, "return nullptr;");
    }
    std::string visitOptional(const Optional* node) override {
        return visitOptional(node, "return nullptr;");
    }
    std::string visitZeroOrMore(const ZeroOrMore* node) override {
        return visitZeroOrMore(node, "return nullptr;");
    }
    std::string visitOneOrMore(const OneOrMore* node) override {
        return visitOneOrMore(node, "return nullptr;");
    }

    // Генерация различных частей парсера
    std::string generateHeader();
    std::string generateIncludes();
    std::string generateASTNodeClasses(const Grammar& grammar);
    std::string generateParserClass(const Grammar& grammar);
    std::string generateParserState();
    std::string generateHelperMethods();
    std::string generateMainParseMethod(const Grammar& grammar);
    std::string generateFooter();
    std::string generateMainCpp(const Grammar& grammar);
    
    // Вспомогательные методы
    std::string getIndent(size_t level) const;
    std::string generateComment(const std::string& comment) const;
    std::string generateFunctionSignature(const std::string& rule_name) const;
    std::string generateTokenMatchCode(const std::string& token_value) const;
    std::string generateErrorHandling(const std::string& expected) const;
    
    // AST генерация
    std::string generateASTNodeType(const std::string& rule_name) const;
    std::string generateASTConstruction(const std::string& rule_name) const;
    
    // Обработка особых случаев
    bool needsHelper(const ASTNode* node) const;
    std::string generateHelperFunction(const ASTNode* node, const std::string& name);
    
    // Оптимизации
    bool canInline(const ASTNode* node) const;
    std::string generateInlineCode(const ASTNode* node);
    
    // Обобщённый метод визитации узлов
    std::string visitNode(const ASTNode* node, const std::string& on_failure_action);
    
    // Extended BNF методы
    std::string generateParameterizedFunction(const ProductionRule& rule);
    std::string generateParameterTypes(const std::vector<RuleParameter>& params);
    std::string generateParameterDeclarations(const std::vector<RuleParameter>& params);
    std::string generateParameterPassing(const std::vector<std::string>& paramValues);
    std::string generateEnumDeclaration(const RuleParameter& param);
    std::string generateContextStorage();  // Для YAML anchors и т.д.
    std::string generateContextActions(const ContextAction* action);
    
    // Проверка поддерживаемых возможностей
    bool isExtendedBNF(const Grammar& grammar) const;
    bool hasParameterizedRules(const Grammar& grammar) const;
    bool hasContextActions(const Grammar& grammar) const;
    bool hasContextActionsInNode(const ASTNode* node) const;
};

} // namespace bnf_parser_generator

