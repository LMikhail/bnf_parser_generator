#pragma once

#include "bnf_ast.hpp"
#include <string>
#include <memory>
#include <vector>

namespace bnf_parser_generator {

/**
 * Настройки генерации кода
 */
struct GeneratorOptions {
    // Целевой язык
    std::string target_language = "cpp";
    
    // Имя генерируемого парсера
    std::string parser_name = "GeneratedParser";
    
    // Namespace/package для сгенерированного кода
    std::string namespace_name = "";
    
    // Включить отладочный вывод в сгенерированный код
    bool debug_mode = false;
    
    // Генерировать методы печати AST
    bool generate_ast_printer = true;
    
    // Генерировать методы обхода AST (visitor pattern)
    bool generate_ast_visitor = false;
    
    // Стиль отступов (пробелы или табы)
    std::string indent_style = "    "; // 4 пробела по умолчанию
    
    // Максимальная глубина рекурсии (защита от переполнения стека)
    size_t max_recursion_depth = 1000;
    
    // Генерировать проверки ошибок
    bool generate_error_handling = true;
    
    // Включить позиционную информацию в AST узлы
    bool track_positions = true;
    
    // Генерировать исполняемый файл (с main())
    bool generate_executable = false;
    
    // Имя входного файла для парсинга (используется в main)
    std::string default_input_file = "";
};

/**
 * Результат генерации кода
 */
struct GeneratedCode {
    // Основной сгенерированный файл (парсер)
    std::string parser_code;
    
    // Имя файла парсера
    std::string parser_filename;
    
    // Дополнительные файлы (например, заголовочные файлы, main.cpp)
    std::vector<std::pair<std::string, std::string>> additional_files; // (filename, content)
    
    // Сгенерированный main.cpp (если generate_executable == true)
    std::string main_code;
    std::string main_filename;
    
    // Сообщения и предупреждения
    std::vector<std::string> messages;
    std::vector<std::string> warnings;
    
    // Успешность генерации
    bool success = true;
    std::string error_message;
};

/**
 * Базовый интерфейс для генераторов кода
 * Реализует паттерн Visitor для обхода BNF AST
 */
class CodeGenerator {
public:
    virtual ~CodeGenerator() = default;
    
    /**
     * Сгенерировать парсер из грамматики
     * @param grammar BNF/EBNF грамматика
     * @param options Настройки генерации
     * @return Результат генерации с кодом и метаданными
     */
    virtual GeneratedCode generate(const Grammar& grammar, const GeneratorOptions& options) = 0;
    
    /**
     * Получить имя целевого языка (cpp, dart, java, clojure)
     */
    virtual std::string getTargetLanguage() const = 0;
    
    /**
     * Получить расширение файлов для целевого языка
     */
    virtual std::string getFileExtension() const = 0;
    
    /**
     * Получить поддерживаемые возможности генератора
     */
    virtual std::vector<std::string> getSupportedFeatures() const = 0;

protected:
    // Вспомогательные методы для обхода AST
    virtual std::string visitTerminal(const Terminal* node) = 0;
    virtual std::string visitNonTerminal(const NonTerminal* node) = 0;
    virtual std::string visitCharRange(const CharRange* node) = 0;
    virtual std::string visitAlternative(const Alternative* node) = 0;
    virtual std::string visitSequence(const Sequence* node) = 0;
    virtual std::string visitGroup(const Group* node) = 0;
    virtual std::string visitOptional(const Optional* node) = 0;
    virtual std::string visitZeroOrMore(const ZeroOrMore* node) = 0;
    virtual std::string visitOneOrMore(const OneOrMore* node) = 0;
    
    // Генерация кода для правил
    virtual std::string generateRuleFunction(const ProductionRule& rule) = 0;
    
    // Утилиты для форматирования
    std::string indent(const std::string& code, size_t level = 1) const;
    std::string escapeString(const std::string& str) const;
    std::string makeIdentifier(const std::string& name) const;
    std::string camelToSnake(const std::string& name) const;
};

/**
 * Фабрика для создания генераторов кода
 */
class CodeGeneratorFactory {
public:
    /**
     * Создать генератор для указанного языка
     * @param language Целевой язык (cpp, dart, java, clojure)
     * @return Уникальный указатель на генератор или nullptr если язык не поддерживается
     */
    static std::unique_ptr<CodeGenerator> create(const std::string& language);
    
    /**
     * Получить список поддерживаемых языков
     */
    static std::vector<std::string> getSupportedLanguages();
    
    /**
     * Проверить, поддерживается ли язык
     */
    static bool isLanguageSupported(const std::string& language);
};

} // namespace bnf_parser_generator

