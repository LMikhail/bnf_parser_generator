#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <variant>

namespace bnf_engine {

/**
 * Абстрактное синтаксическое дерево для BNF/EBNF грамматик
 */

// Базовый класс для всех узлов AST
class ASTNode {
public:
    virtual ~ASTNode() = default;
    virtual std::string toString() const = 0;
};

// Терминальный символ (в кавычках)
class Terminal : public ASTNode {
public:
    std::string value;
    
    explicit Terminal(const std::string& val) : value(val) {}
    std::string toString() const override { return "\"" + value + "\""; }
};

// Нетерминальный символ
class NonTerminal : public ASTNode {
public:
    std::string name;
    
    explicit NonTerminal(const std::string& n) : name(n) {}
    std::string toString() const override { return "<" + name + ">"; }
};

// Диапазон символов (EBNF): 'a'..'z'
class CharRange : public ASTNode {
public:
    char start;
    char end;
    
    CharRange(char s, char e) : start(s), end(e) {}
    std::string toString() const override { 
        return "'" + std::string(1, start) + "'.." + "'" + std::string(1, end) + "'"; 
    }
};

// Альтернативы: A | B | C
class Alternative : public ASTNode {
public:
    std::vector<std::unique_ptr<ASTNode>> choices;
    
    void addChoice(std::unique_ptr<ASTNode> choice) {
        choices.push_back(std::move(choice));
    }
    
    std::string toString() const override {
        std::string result;
        for (size_t i = 0; i < choices.size(); ++i) {
            if (i > 0) result += " | ";
            result += choices[i]->toString();
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
    
    std::string toString() const override {
        std::string result;
        for (size_t i = 0; i < elements.size(); ++i) {
            if (i > 0) result += " ";
            result += elements[i]->toString();
        }
        return result;
    }
};

// Группировка: (A | B)
class Group : public ASTNode {
public:
    std::unique_ptr<ASTNode> content;
    
    explicit Group(std::unique_ptr<ASTNode> c) : content(std::move(c)) {}
    std::string toString() const override { 
        return "(" + content->toString() + ")"; 
    }
};

// EBNF: Опциональность [A] или A?
class Optional : public ASTNode {
public:
    std::unique_ptr<ASTNode> content;
    
    explicit Optional(std::unique_ptr<ASTNode> c) : content(std::move(c)) {}
    std::string toString() const override { 
        return "[" + content->toString() + "]"; 
    }
};

// EBNF: Повторение 0 или более раз {A} или A*
class ZeroOrMore : public ASTNode {
public:
    std::unique_ptr<ASTNode> content;
    
    explicit ZeroOrMore(std::unique_ptr<ASTNode> c) : content(std::move(c)) {}
    std::string toString() const override { 
        return "{" + content->toString() + "}"; 
    }
};

// EBNF: Повторение 1 или более раз A+
class OneOrMore : public ASTNode {
public:
    std::unique_ptr<ASTNode> content;
    
    explicit OneOrMore(std::unique_ptr<ASTNode> c) : content(std::move(c)) {}
    std::string toString() const override { 
        return content->toString() + "+"; 
    }
};

// Правило продукции: A ::= B | C
class ProductionRule {
public:
    std::string leftSide;  // Нетерминал слева
    std::unique_ptr<ASTNode> rightSide;  // Правая часть
    
    ProductionRule(const std::string& left, std::unique_ptr<ASTNode> right)
        : leftSide(left), rightSide(std::move(right)) {}
    
    std::string toString() const {
        return leftSide + " ::= " + rightSide->toString();
    }
};

// Полная грамматика
class Grammar {
public:
    std::vector<std::unique_ptr<ProductionRule>> rules;
    std::string startSymbol;  // Стартовый символ грамматики
    
    void addRule(std::unique_ptr<ProductionRule> rule) {
        if (startSymbol.empty()) {
            startSymbol = rule->leftSide;  // Первое правило - стартовый символ
        }
        rules.push_back(std::move(rule));
    }
    
    // Найти правило по имени нетерминала
    const ProductionRule* findRule(const std::string& nonTerminal) const {
        for (const auto& rule : rules) {
            if (rule->leftSide == nonTerminal) {
                return rule.get();
            }
        }
        return nullptr;
    }
    
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

} // namespace bnf_engine
