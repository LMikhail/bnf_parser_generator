#include "bnf_parser.hpp"
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <unordered_set>

namespace bnf_engine {

BNFParser::BNFParser(const std::vector<BNFToken>& tokens) 
    : tokens_(tokens), current_(0) {}

std::unique_ptr<Grammar> BNFParser::parseGrammar() {
    auto grammar = std::make_unique<Grammar>();
    error_.clear();
    
    try {
        // Пропускаем комментарии и переводы строк в начале
        while (!isAtEnd() && (check(TokenType::COMMENT) || check(TokenType::NEWLINE))) {
            advance();
        }
        
        // Парсим правила продукции
        while (!isAtEnd()) {
            if (check(TokenType::COMMENT) || check(TokenType::NEWLINE)) {
                advance();
                continue;
            }
            
            auto rule = parseRule();
            if (rule) {
                grammar->addRule(std::move(rule));
            } else {
                break; // Ошибка парсинга
            }
        }
        
        if (!error_.empty()) {
            return nullptr;
        }
        
        return grammar;
        
    } catch (const std::exception& e) {
        error_ = e.what();
        return nullptr;
    }
}

std::unique_ptr<ProductionRule> BNFParser::parseRule() {
    // Ожидаем: IDENTIFIER ::= expression
    
    if (!check(TokenType::IDENTIFIER)) {
        error("Expected rule name (identifier)");
        return nullptr;
    }
    
    BNFToken nameToken = advance();
    std::string ruleName = nameToken.value;
    
    if (!match(TokenType::DEFINE)) {
        error("Expected '::=' after rule name");
        return nullptr;
    }
    
    auto expression = parseExpression();
    if (!expression) {
        return nullptr;
    }
    
    // Пропускаем опциональные переводы строк после правила
    while (check(TokenType::NEWLINE)) {
        advance();
    }
    
    return std::make_unique<ProductionRule>(ruleName, std::move(expression));
}

std::unique_ptr<ASTNode> BNFParser::parseExpression() {
    // expression ::= alternative
    return parseAlternative();
}

std::unique_ptr<ASTNode> BNFParser::parseAlternative() {
    // alternative ::= sequence ('|' sequence)*
    
    auto left = parseSequence();
    if (!left) return nullptr;
    
    if (!check(TokenType::ALTERNATIVE)) {
        return left; // Только одна альтернатива
    }
    
    // Множественные альтернативы
    auto alternative = std::make_unique<Alternative>();
    alternative->addChoice(std::move(left));
    
    while (match(TokenType::ALTERNATIVE)) {
        auto right = parseSequence();
        if (!right) return nullptr;
        alternative->addChoice(std::move(right));
    }
    
    return std::move(alternative);
}

std::unique_ptr<ASTNode> BNFParser::parseSequence() {
    // sequence ::= factor+
    
    std::vector<std::unique_ptr<ASTNode>> elements;
    
    auto first = parseFactor();
    if (!first) return nullptr;
    
    elements.push_back(std::move(first));
    
    // Продолжаем парсить элементы последовательности
    while (!isAtEnd() && 
           !check(TokenType::ALTERNATIVE) && 
           !check(TokenType::RIGHT_PAREN) && 
           !check(TokenType::RIGHT_BRACKET) && 
           !check(TokenType::RIGHT_BRACE) &&
           !check(TokenType::NEWLINE) &&
           !check(TokenType::EOF_TOKEN)) {
        
        auto element = parseFactor();
        if (!element) break;
        elements.push_back(std::move(element));
    }
    
    if (elements.size() == 1) {
        return std::move(elements[0]);
    }
    
    auto sequence = std::make_unique<Sequence>();
    for (auto& element : elements) {
        sequence->addElement(std::move(element));
    }
    
    return std::move(sequence);
}

std::unique_ptr<ASTNode> BNFParser::parseFactor() {
    // factor ::= primary ('+' | '*' | '?')?
    
    auto primary = parsePrimary();
    if (!primary) return nullptr;
    
    // EBNF постфиксные операторы
    if (match(TokenType::PLUS)) {
        return std::make_unique<OneOrMore>(std::move(primary));
    } else if (match(TokenType::STAR)) {
        return std::make_unique<ZeroOrMore>(std::move(primary));
    } else if (match(TokenType::QUESTION)) {
        return std::make_unique<Optional>(std::move(primary));
    }
    
    return primary;
}

std::unique_ptr<ASTNode> BNFParser::parsePrimary() {
    // primary ::= IDENTIFIER | TERMINAL | '(' expression ')' | '[' expression ']' | '{' expression '}' | char_range
    
    if (check(TokenType::IDENTIFIER)) {
        BNFToken token = advance();
        return std::make_unique<NonTerminal>(token.value);
    }
    
    // Обработка терминалов перенесена ниже для поддержки диапазонов
    
    if (match(TokenType::LEFT_PAREN)) {
        auto expr = parseExpression();
        if (!expr) return nullptr;
        
        if (!match(TokenType::RIGHT_PAREN)) {
            error("Expected ')' after grouped expression");
            return nullptr;
        }
        
        return std::make_unique<Group>(std::move(expr));
    }
    
    if (match(TokenType::LEFT_BRACKET)) {
        auto expr = parseExpression();
        if (!expr) return nullptr;
        
        if (!match(TokenType::RIGHT_BRACKET)) {
            error("Expected ']' after optional expression");
            return nullptr;
        }
        
        return std::make_unique<Optional>(std::move(expr));
    }
    
    if (match(TokenType::LEFT_BRACE)) {
        auto expr = parseExpression();
        if (!expr) return nullptr;
        
        if (!match(TokenType::RIGHT_BRACE)) {
            error("Expected '}' after repetition expression");
            return nullptr;
        }
        
        return std::make_unique<ZeroOrMore>(std::move(expr));
    }
    
    // Диапазон символов: 'a'..'z'
    if (check(TokenType::TERMINAL)) {
        size_t saved = current_;
        BNFToken start = advance();
        
        if (match(TokenType::DOT_DOT) && check(TokenType::TERMINAL)) {
            BNFToken end = advance();
            
            if (start.value.length() == 1 && end.value.length() == 1) {
                return std::make_unique<CharRange>(start.value[0], end.value[0]);
            } else {
                error("Character ranges must be single characters");
                return nullptr;
            }
        } else {
            // Откат - это был обычный терминал
            current_ = saved;
            BNFToken token = advance();
            return std::make_unique<Terminal>(token.value);
        }
    }
    
    error("Expected identifier, terminal, or grouped expression");
    return nullptr;
}

bool BNFParser::match(TokenType type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

bool BNFParser::check(TokenType type) const {
    if (isAtEnd()) return false;
    return peek().type == type;
}

BNFToken BNFParser::advance() {
    if (!isAtEnd()) current_++;
    return tokens_[current_ - 1];
}

BNFToken BNFParser::peek() const {
    return tokens_[current_];
}

bool BNFParser::isAtEnd() const {
    return current_ >= tokens_.size() || peek().type == TokenType::EOF_TOKEN;
}

void BNFParser::error(const std::string& message) {
    BNFToken token = peek();
    error_ = "Parse error at line " + std::to_string(token.line) + 
             ", column " + std::to_string(token.column) + ": " + message;
}

// Валидация грамматики согласно классическим правилам BNF
BNFParser::ValidationResult BNFParser::validateGrammar(const Grammar& grammar) {
    ValidationResult result;
    result.isValid = true;
    
    if (grammar.rules.empty()) {
        result.errors.push_back("Grammar is empty");
        result.isValid = false;
        return result;
    }
    
    // Собираем все нетерминалы и терминалы
    std::unordered_set<std::string> definedNonTerminals;
    std::unordered_set<std::string> usedNonTerminals;
    std::unordered_set<std::string> terminals;
    
    for (const auto& rule : grammar.rules) {
        definedNonTerminals.insert(rule->leftSide);
        collectSymbols(rule->rightSide.get(), usedNonTerminals, terminals);
    }
    
    // Проверка 1: Все используемые нетерминалы должны быть определены
    for (const std::string& nonTerminal : usedNonTerminals) {
        if (definedNonTerminals.find(nonTerminal) == definedNonTerminals.end()) {
            result.errors.push_back("Undefined non-terminal: " + nonTerminal);
            result.isValid = false;
        }
    }
    
    // Проверка 2: Все определённые нетерминалы должны быть достижимы из стартового символа
    std::unordered_set<std::string> reachable;
    std::vector<std::string> toProcess = {grammar.startSymbol};
    
    while (!toProcess.empty()) {
        std::string current = toProcess.back();
        toProcess.pop_back();
        
        if (reachable.find(current) != reachable.end()) continue;
        reachable.insert(current);
        
        const ProductionRule* rule = grammar.findRule(current);
        if (rule) {
            std::unordered_set<std::string> used, terms;
            collectSymbols(rule->rightSide.get(), used, terms);
            for (const std::string& nt : used) {
                if (reachable.find(nt) == reachable.end()) {
                    toProcess.push_back(nt);
                }
            }
        }
    }
    
    for (const std::string& nonTerminal : definedNonTerminals) {
        if (reachable.find(nonTerminal) == reachable.end()) {
            result.warnings.push_back("Unreachable non-terminal: " + nonTerminal);
        }
    }
    
    // Проверка 3: Все нетерминалы должны быть продуктивными (выводить терминальные строки)
    std::unordered_set<std::string> productive;
    bool changed = true;
    
    while (changed) {
        changed = false;
        for (const auto& rule : grammar.rules) {
            if (productive.find(rule->leftSide) == productive.end()) {
                if (isProductive(rule->rightSide.get(), productive)) {
                    productive.insert(rule->leftSide);
                    changed = true;
                }
            }
        }
    }
    
    for (const std::string& nonTerminal : definedNonTerminals) {
        if (productive.find(nonTerminal) == productive.end()) {
            result.errors.push_back("Non-productive non-terminal: " + nonTerminal);
            result.isValid = false;
        }
    }
    
    return result;
}

void BNFParser::collectSymbols(const ASTNode* node, 
                               std::unordered_set<std::string>& nonTerminals,
                               std::unordered_set<std::string>& terminals) {
    if (const auto* nt = dynamic_cast<const NonTerminal*>(node)) {
        nonTerminals.insert(nt->name);
    } else if (const auto* t = dynamic_cast<const Terminal*>(node)) {
        terminals.insert(t->value);
    } else if (const auto* alt = dynamic_cast<const Alternative*>(node)) {
        for (const auto& choice : alt->choices) {
            collectSymbols(choice.get(), nonTerminals, terminals);
        }
    } else if (const auto* seq = dynamic_cast<const Sequence*>(node)) {
        for (const auto& element : seq->elements) {
            collectSymbols(element.get(), nonTerminals, terminals);
        }
    } else if (const auto* group = dynamic_cast<const Group*>(node)) {
        collectSymbols(group->content.get(), nonTerminals, terminals);
    } else if (const auto* opt = dynamic_cast<const Optional*>(node)) {
        collectSymbols(opt->content.get(), nonTerminals, terminals);
    } else if (const auto* zeroMore = dynamic_cast<const ZeroOrMore*>(node)) {
        collectSymbols(zeroMore->content.get(), nonTerminals, terminals);
    } else if (const auto* oneMore = dynamic_cast<const OneOrMore*>(node)) {
        collectSymbols(oneMore->content.get(), nonTerminals, terminals);
    }
}

bool BNFParser::isProductive(const ASTNode* node, 
                            const std::unordered_set<std::string>& productive) {
    if (const auto* nt = dynamic_cast<const NonTerminal*>(node)) {
        return productive.find(nt->name) != productive.end();
    } else if (dynamic_cast<const Terminal*>(node) || dynamic_cast<const CharRange*>(node)) {
        return true; // Терминалы всегда продуктивны
    } else if (const auto* alt = dynamic_cast<const Alternative*>(node)) {
        // Альтернатива продуктивна, если хотя бы один выбор продуктивен
        for (const auto& choice : alt->choices) {
            if (isProductive(choice.get(), productive)) {
                return true;
            }
        }
        return false;
    } else if (const auto* seq = dynamic_cast<const Sequence*>(node)) {
        // Последовательность продуктивна, если все элементы продуктивны
        for (const auto& element : seq->elements) {
            if (!isProductive(element.get(), productive)) {
                return false;
            }
        }
        return true;
    } else if (const auto* group = dynamic_cast<const Group*>(node)) {
        return isProductive(group->content.get(), productive);
    } else if (const auto* opt = dynamic_cast<const Optional*>(node)) {
        return true; // Опциональные элементы всегда продуктивны (могут быть пустыми)
    } else if (const auto* zeroMore = dynamic_cast<const ZeroOrMore*>(node)) {
        return true; // Повторение 0+ всегда продуктивно (может быть пустым)
    } else if (const auto* oneMore = dynamic_cast<const OneOrMore*>(node)) {
        return isProductive(oneMore->content.get(), productive);
    }
    
    return false;
}

} // namespace bnf_engine
