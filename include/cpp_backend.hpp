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
    // Реализация visitor методов для AST узлов
    std::string visitTerminal(const Terminal* node) override;
    std::string visitNonTerminal(const NonTerminal* node) override;
    std::string visitCharRange(const CharRange* node) override;
    std::string visitAlternative(const Alternative* node) override;
    std::string visitSequence(const Sequence* node) override;
    std::string visitGroup(const Group* node) override;
    std::string visitOptional(const Optional* node) override;
    std::string visitZeroOrMore(const ZeroOrMore* node) override;
    std::string visitOneOrMore(const OneOrMore* node) override;
    
    // Генерация функций для правил
    std::string generateRuleFunction(const ProductionRule& rule) override;

private:
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
    std::string generateInlineNode(const ASTNode* node, const std::string& success_var, const std::string& indent);
    
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
    std::string visitNode(const ASTNode* node);
};

} // namespace bnf_parser_generator

