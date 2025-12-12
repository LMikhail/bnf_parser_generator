#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <variant>
#include <cstdint>
#include <cctype>

namespace bnf_parser_generator {

/**
 * Абстрактное синтаксическое дерево для BNF/EBNF грамматик
 * Поддерживает Extended BNF для контекстно-зависимых грамматик
 */

// Типы параметров для Extended BNF
enum class ParameterType {
    ENUM,      // enum class Number { Sing, Plur }
    INTEGER,   // int (для indentation levels)
    STRING,    // std::string (для identifiers)
    BOOLEAN    // bool (для flags)
};

// Параметр правила в Extended BNF
struct RuleParameter {
    std::string name;           // Имя параметра (N, indent, etc.)
    ParameterType type;         // Тип параметра
    std::vector<std::string> enumValues;  // Для ENUM типа: {"sing", "plur"}
    std::string defaultValue;   // Значение по умолчанию (опционально)
    
    RuleParameter(const std::string& n, ParameterType t) 
        : name(n), type(t) {}
    
    RuleParameter(const std::string& n, const std::vector<std::string>& values)
        : name(n), type(ParameterType::ENUM), enumValues(values) {}
    
    std::string toString() const {
        switch (type) {
            case ParameterType::ENUM:
                return name + ":{" + joinStrings(enumValues, ",") + "}";
            case ParameterType::INTEGER:
                return name + ":int";
            case ParameterType::STRING:
                return name + ":string";
            case ParameterType::BOOLEAN:
                return name + ":bool";
        }
        return name;
    }
    
private:
    std::string joinStrings(const std::vector<std::string>& strings, const std::string& delimiter) const {
        std::string result;
        for (std::size_t i = 0; i < strings.size(); ++i) {
            if (i > 0) result += delimiter;
            result += strings[i];
        }
        return result;
    }
};

// Базовый класс для всех узлов AST
class ASTNode {
public:
    virtual ~ASTNode() = default;
    virtual std::string toString(int indent = 0) const = 0;

    // RTTI-хелперы для безопасного приведения типов
    template<typename T>
    bool is() const {
        return dynamic_cast<const T*>(this) != nullptr;
    }

    template<typename T>
    const T* as() const {
        return dynamic_cast<const T*>(this);
    }

    template<typename T>
    T* as() {
        return dynamic_cast<T*>(this);
    }
};

// Контекстное действие: {store(name, value)} или {lookup(name)}
class ContextAction : public ASTNode {
public:
    enum class ActionType {
        STORE,   // {store(name, value)}
        LOOKUP,  // {lookup(name)}
        CHECK    // {check(condition)}
    };
    
    ActionType actionType;
    std::vector<std::string> arguments;
    
    ContextAction(ActionType type, const std::vector<std::string>& args)
        : actionType(type), arguments(args) {}
    
    std::string toString(int indent = 0) const override {
        (void)indent;
        std::string result = "{";
        switch (actionType) {
            case ActionType::STORE:
                result += "store(" + arguments[0] + ", " + arguments[1] + ")";
                break;
            case ActionType::LOOKUP:
                result += "lookup(" + arguments[0] + ")";
                break;
            case ActionType::CHECK:
                result += "check(" + arguments[0] + ")";
                break;
        }
        result += "}";
        return result;
    }
};

// Терминальный символ (в кавычках)
class Terminal : public ASTNode {
public:
    std::string value;
    
    explicit Terminal(const std::string& val) : value(val) {}
    std::string toString(int indent = 0) const override {
        (void)indent;
        return "\"" + value + "\"";
    }
};

// Нетерминальный символ с поддержкой параметров
class NonTerminal : public ASTNode {
public:
    std::string name;
    std::vector<std::string> parameterValues;  // Значения параметров при вызове
    
    explicit NonTerminal(const std::string& n) : name(n) {}
    
    NonTerminal(const std::string& n, const std::vector<std::string>& params)
        : name(n), parameterValues(params) {}
    
    std::string toString(int indent = 0) const override { 
        (void)indent;
        std::string result = "<" + name;
        if (!parameterValues.empty()) {
            result += "[";
            for (std::size_t i = 0; i < parameterValues.size(); ++i) {
                if (i > 0) result += ", ";
                result += parameterValues[i];
            }
            result += "]";
        }
        result += ">";
        return result;
    }
    
    bool hasParameters() const { return !parameterValues.empty(); }
};

// Диапазон символов (EBNF): 'a'..'z' или '\u0000'..'\U0010FFFF'
class CharRange : public ASTNode {
public:
    uint32_t start;  // Unicode codepoint
    uint32_t end;    // Unicode codepoint
    
    CharRange(uint32_t s, uint32_t e) : start(s), end(e) {}
    std::string toString(int indent = 0) const override;
};

// Альтернативы: A | B | C
class Alternative : public ASTNode {
public:
    std::vector<std::unique_ptr<ASTNode>> choices;
    
    void addChoice(std::unique_ptr<ASTNode> choice) {
        choices.push_back(std::move(choice));
    }
    
    std::string toString(int indent = 0) const override {
        (void)indent;
        std::string result;
        for (std::size_t i = 0; i < choices.size(); ++i) {
            if (i > 0) result += " | ";
            result += choices[i]->toString(indent);
        }
        return result;
    }
};

// Последовательность: A B C
class Sequence : public ASTNode {
public:
    std::vector<std::unique_ptr<ASTNode>> elements;
    
    void addElement(std::unique_ptr<ASTNode> element) {
        elements.push_back(std::move(element));
    }
    
    std::string toString(int indent = 0) const override {
        (void)indent;
        std::string result;
        for (std::size_t i = 0; i < elements.size(); ++i) {
            if (i > 0) result += " ";
            result += elements[i]->toString(indent);
        }
        return result;
    }
};

// Группировка: (A | B)
class Group : public ASTNode {
public:
    std::unique_ptr<ASTNode> content;
    
    explicit Group(std::unique_ptr<ASTNode> c) : content(std::move(c)) {}
    std::string toString(int indent = 0) const override { 
        (void)indent;
        return "(" + content->toString(indent) + ")"; 
    }
};

// EBNF: Опциональность [A] или A?
class Optional : public ASTNode {
public:
    std::unique_ptr<ASTNode> content;
    
    explicit Optional(std::unique_ptr<ASTNode> c) : content(std::move(c)) {}
    std::string toString(int indent = 0) const override { 
        (void)indent;
        return "[" + content->toString(indent) + "]"; 
    }
};

// EBNF: Повторение 0 или более раз {A} или A*
class ZeroOrMore : public ASTNode {
public:
    std::unique_ptr<ASTNode> content;
    
    explicit ZeroOrMore(std::unique_ptr<ASTNode> c) : content(std::move(c)) {}
    std::string toString(int indent = 0) const override { 
        (void)indent;
        return "{" + content->toString(indent) + "}"; 
    }
};

// EBNF: Повторение 1 или более раз A+
class OneOrMore : public ASTNode {
public:
    std::unique_ptr<ASTNode> content;
    
    explicit OneOrMore(std::unique_ptr<ASTNode> c) : content(std::move(c)) {}
    std::string toString(int indent = 0) const override { 
        (void)indent;
        return content->toString(indent) + "+"; 
    }
};

// Правило продукции с поддержкой параметров: A[N] ::= B[N] C[N]
class ProductionRule {
public:
    std::string leftSide;  // Нетерминал слева
    std::vector<RuleParameter> parameters;  // Параметры правила
    std::unique_ptr<ASTNode> rightSide;  // Правая часть
    
    ProductionRule(const std::string& left, std::unique_ptr<ASTNode> right)
        : leftSide(left), rightSide(std::move(right)) {}
    
    ProductionRule(const std::string& left, const std::vector<RuleParameter>& params, std::unique_ptr<ASTNode> right)
        : leftSide(left), parameters(params), rightSide(std::move(right)) {}
    
    std::string toString() const {
        std::string result = leftSide;
        if (!parameters.empty()) {
            result += "[";
            for (std::size_t i = 0; i < parameters.size(); ++i) {
                if (i > 0) result += ", ";
                result += parameters[i].toString();
            }
            result += "]";
        }
        result += " ::= " + rightSide->toString();
        return result;
    }
    
    bool hasParameters() const { return !parameters.empty(); }
    
    // Найти параметр по имени
    const RuleParameter* findParameter(const std::string& name) const {
        for (const auto& param : parameters) {
            if (param.name == name) {
                return &param;
            }
        }
        return nullptr;
    }
};

// Полная грамматика
class Grammar {
public:
    std::vector<std::unique_ptr<ProductionRule>> rules;
    std::string startSymbol;  // Стартовый символ грамматики
    
    void addRule(std::unique_ptr<ProductionRule> rule) {
        rules.push_back(std::move(rule));
    }
    
    // Определяет стартовый символ грамматики после добавления всех правил
    void determineStartSymbol() {
        if (!startSymbol.empty() || rules.empty()) {
            return; // Уже установлен или нет правил
        }
        
        // 1. Проверяем специальные имена
        const std::vector<std::string> specialNames = {"json", "program", "start", "grammar", "root"};
        for (const auto& ruleName : specialNames) {
            for (const auto& r : rules) {
                if (r->leftSide == ruleName) {
                    startSymbol = ruleName;
                    return;
                }
            }
        }
        
        // 2. Берем первое правило, которое ссылается на non-terminals
        for (const auto& r : rules) {
            if (hasNonTerminalReferences(r->rightSide.get())) {
                startSymbol = r->leftSide;
                return;
            }
        }
        
        // 3. В крайнем случае, берем просто первое правило
        if (!rules.empty()) {
            startSymbol = rules[0]->leftSide;
        }
    }
    
private:
    // Проверяет, содержит ли узл ссылки на non-terminals
    bool hasNonTerminalReferences(const ASTNode* node) const {
        if (dynamic_cast<const NonTerminal*>(node)) {
            return true;
        }
        if (auto* seq = dynamic_cast<const Sequence*>(node)) {
            for (const auto& elem : seq->elements) {
                if (hasNonTerminalReferences(elem.get())) return true;
            }
        }
        if (auto* alt = dynamic_cast<const Alternative*>(node)) {
            for (const auto& choice : alt->choices) {
                if (hasNonTerminalReferences(choice.get())) return true;
            }
        }
        if (auto* grp = dynamic_cast<const Group*>(node)) {
            return hasNonTerminalReferences(grp->content.get());
        }
        if (auto* opt = dynamic_cast<const Optional*>(node)) {
            return hasNonTerminalReferences(opt->content.get());
        }
        if (auto* zom = dynamic_cast<const ZeroOrMore*>(node)) {
            return hasNonTerminalReferences(zom->content.get());
        }
        if (auto* oom = dynamic_cast<const OneOrMore*>(node)) {
            return hasNonTerminalReferences(oom->content.get());
        }
        return false;
    }
    
public:
    
    // Найти правило по имени нетерминала
    const ProductionRule* findRule(const std::string& nonTerminal) const {
        for (const auto& rule : rules) {
            if (rule->leftSide == nonTerminal) {
                return rule.get();
            }
        }
        return nullptr;
    }
    
    // Найти все правила с параметрами
    std::vector<const ProductionRule*> getParameterizedRules() const {
        std::vector<const ProductionRule*> result;
        for (const auto& rule : rules) {
            if (rule->hasParameters()) {
                result.push_back(rule.get());
            }
        }
        return result;
    }
    
    // Проверить, является ли грамматика контекстно-зависимой
    bool isContextSensitive() const {
        // Грамматика контекстно-зависимая, если содержит параметризованные правила
        for (const auto& rule : rules) {
            if (rule->hasParameters()) {
                return true;
            }
        }
        
        // Или содержит контекстные действия
        for (const auto& rule : rules) {
            if (hasContextActions(rule->rightSide.get())) {
                return true;
            }
        }
        
        return false;
    }
    
private:
    // Проверяет, содержит ли узел контекстные действия
    bool hasContextActions(const ASTNode* node) const {
        if (dynamic_cast<const ContextAction*>(node)) {
            return true;
        }
        if (auto* seq = dynamic_cast<const Sequence*>(node)) {
            for (const auto& elem : seq->elements) {
                if (hasContextActions(elem.get())) return true;
            }
        }
        if (auto* alt = dynamic_cast<const Alternative*>(node)) {
            for (const auto& choice : alt->choices) {
                if (hasContextActions(choice.get())) return true;
            }
        }
        if (auto* grp = dynamic_cast<const Group*>(node)) {
            return hasContextActions(grp->content.get());
        }
        if (auto* opt = dynamic_cast<const Optional*>(node)) {
            return hasContextActions(opt->content.get());
        }
        if (auto* zom = dynamic_cast<const ZeroOrMore*>(node)) {
            return hasContextActions(zom->content.get());
        }
        if (auto* oom = dynamic_cast<const OneOrMore*>(node)) {
            return hasContextActions(oom->content.get());
        }
        return false;
    }
    
public:
    
    // Получить все нетерминалы
    std::vector<std::string> getNonTerminals() const {
        std::vector<std::string> result;
        for (const auto& rule : rules) {
            result.push_back(rule->leftSide);
        }
        return result;
    }
    
    // Получить все терминалы из грамматики
    std::vector<std::string> getTerminals() const {
        std::vector<std::string> terminals;
        for (const auto& rule : rules) {
            collectTerminals(rule->rightSide.get(), terminals);
        }
        return terminals;
    }
    
    std::string toString() const {
        std::string result = "# Grammar (start: " + startSymbol + ")\n";
        for (const auto& rule : rules) {
            result += rule->toString() + "\n";
        }
        return result;
    }

private:
    void collectTerminals(const ASTNode* node, std::vector<std::string>& terminals) const {
        if (const auto* terminal = dynamic_cast<const Terminal*>(node)) {
            terminals.push_back(terminal->value);
        } else if (const auto* alt = dynamic_cast<const Alternative*>(node)) {
            for (const auto& choice : alt->choices) {
                collectTerminals(choice.get(), terminals);
            }
        } else if (const auto* seq = dynamic_cast<const Sequence*>(node)) {
            for (const auto& element : seq->elements) {
                collectTerminals(element.get(), terminals);
            }
        } else if (const auto* group = dynamic_cast<const Group*>(node)) {
            collectTerminals(group->content.get(), terminals);
        } else if (const auto* opt = dynamic_cast<const Optional*>(node)) {
            collectTerminals(opt->content.get(), terminals);
        } else if (const auto* zeroMore = dynamic_cast<const ZeroOrMore*>(node)) {
            collectTerminals(zeroMore->content.get(), terminals);
        } else if (const auto* oneMore = dynamic_cast<const OneOrMore*>(node)) {
            collectTerminals(oneMore->content.get(), terminals);
        }
    }
};

} // namespace bnf_parser_generator
