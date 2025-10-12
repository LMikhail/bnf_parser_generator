#include "code_generator.hpp"
#include "cpp_backend.hpp"
#include <algorithm>
#include <cctype>

namespace bnf_parser_generator {

// Реализация базовых утилит CodeGenerator

std::string CodeGenerator::indent(const std::string& code, size_t level) const {
    std::string indent_str;
    for (size_t i = 0; i < level; ++i) {
        indent_str += "    "; // 4 пробела на уровень
    }
    
    std::string result;
    std::istringstream stream(code);
    std::string line;
    
    while (std::getline(stream, line)) {
        if (!line.empty()) {
            result += indent_str + line + "\n";
        } else {
            result += "\n";
        }
    }
    
    return result;
}

std::string CodeGenerator::escapeString(const std::string& str) const {
    std::string result;
    for (char c : str) {
        switch (c) {
            case '\\': result += "\\\\"; break;
            case '\"': result += "\\\""; break;
            case '\n': result += "\\n"; break;
            case '\r': result += "\\r"; break;
            case '\t': result += "\\t"; break;
            default: result += c; break;
        }
    }
    return result;
}

std::string CodeGenerator::makeIdentifier(const std::string& name) const {
    std::string result;
    for (char c : name) {
        if (std::isalnum(c) || c == '_') {
            result += c;
        } else {
            result += '_';
        }
    }
    
    // Убедимся, что идентификатор начинается с буквы или подчёркивания
    if (!result.empty() && std::isdigit(result[0])) {
        result = "_" + result;
    }
    
    return result;
}

// Реализация фабрики генераторов

std::unique_ptr<CodeGenerator> CodeGeneratorFactory::create(const std::string& language) {
    std::string lang_lower = language;
    std::transform(lang_lower.begin(), lang_lower.end(), lang_lower.begin(), ::tolower);
    
    if (lang_lower == "cpp" || lang_lower == "c++" || lang_lower == "cxx") {
        return std::make_unique<CppCodeGenerator>();
    }
    // Будущие языки:
    // else if (lang_lower == "dart") {
    //     return std::make_unique<DartCodeGenerator>();
    // }
    // else if (lang_lower == "java") {
    //     return std::make_unique<JavaCodeGenerator>();
    // }
    // else if (lang_lower == "clojure" || lang_lower == "clj") {
    //     return std::make_unique<ClojureCodeGenerator>();
    // }
    
    return nullptr;
}

std::vector<std::string> CodeGeneratorFactory::getSupportedLanguages() {
    return {
        "cpp",
        // Будущие языки:
        // "dart",
        // "java",
        // "clojure"
    };
}

bool CodeGeneratorFactory::isLanguageSupported(const std::string& language) {
    auto supported = getSupportedLanguages();
    std::string lang_lower = language;
    std::transform(lang_lower.begin(), lang_lower.end(), lang_lower.begin(), ::tolower);
    
    return std::find(supported.begin(), supported.end(), lang_lower) != supported.end();
}

} // namespace bnf_parser_generator

