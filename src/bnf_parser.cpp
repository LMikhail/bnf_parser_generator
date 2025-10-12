#include "bnf_parser.hpp"
#include "utf8_utils.hpp"
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <unordered_set>

namespace bnf_parser_generator {

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
        
        // Определяем стартовый символ после добавления всех правил
        grammar->determineStartSymbol();
        
        return grammar;
        
    } catch (const std::exception& e) {
        error_ = e.what();
        return nullptr;
    }
}

std::unique_ptr<ProductionRule> BNFParser::parseRule() {
    // Ожидаем: IDENTIFIER [param1, param2:type] ::= expression
    
    if (!check(TokenType::IDENTIFIER)) {
        error("Expected rule name (identifier)");
        return nullptr;
    }
    
    BNFToken nameToken = advance();
    std::string ruleName = nameToken.value;
    
    // Парсим параметры правила (опционально)
    std::vector<RuleParameter> parameters;
    if (check(TokenType::LEFT_BRACKET)) {
        parameters = parseRuleParameters();
    }
    
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
    
    if (parameters.empty()) {
        return std::make_unique<ProductionRule>(ruleName, std::move(expression));
    } else {
        return std::make_unique<ProductionRule>(ruleName, parameters, std::move(expression));
    }
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
    
    return alternative;
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
    
    return sequence;
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
    // primary ::= IDENTIFIER [params] | TERMINAL | '(' expression ')' | '[' expression ']' | '{' expression '}' | char_range | context_action
    
    // Параметризованные нетерминалы и контекстные действия
    if (check(TokenType::IDENTIFIER)) {
        return parseParameterizedNonTerminal();
    }
    
    // Контекстные действия: {store(name, value)}
    if (check(TokenType::LEFT_BRACE)) {
        // Проверяем, это контекстное действие или повторение
        size_t saved = current_;
        advance(); // пропускаем {
        
        if (check(TokenType::IDENTIFIER)) {
            // Проверяем, следует ли за идентификатором открывающая скобка
            size_t saved2 = current_;
            advance(); // пропускаем идентификатор
            
            if (check(TokenType::LEFT_PAREN)) {
                // Это контекстное действие
                current_ = saved; // возвращаемся к началу
                return parseContextAction();
            }
            
            current_ = saved2; // возвращаемся к идентификатору
        }
        
        current_ = saved; // возвращаемся к {
        
        // Это обычное повторение {expression}
        match(TokenType::LEFT_BRACE);
        auto expr = parseExpression();
        if (!expr) return nullptr;
        
        if (!match(TokenType::RIGHT_BRACE)) {
            error("Expected '}' after repetition expression");
            return nullptr;
        }
        
        return std::make_unique<ZeroOrMore>(std::move(expr));
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
    
    // Диапазон символов: 'a'..'z'
    if (check(TokenType::TERMINAL)) {
        size_t saved = current_;
        BNFToken start = advance();
        
        if (match(TokenType::DOT_DOT) && check(TokenType::TERMINAL)) {
            BNFToken end = advance();
            
            // Используем UTF-8 утилиты для определения количества символов
            if (utf8::length(start.value) == 1 && utf8::length(end.value) == 1) {
                // Извлекаем Unicode codepoints
                uint32_t start_cp = utf8::utf8ToCodepoint(start.value);
                uint32_t end_cp = utf8::utf8ToCodepoint(end.value);
                
                if (start_cp == 0 || end_cp == 0) {
                    error("Invalid UTF-8 character in range");
                    return nullptr;
                }
                
                // CharRange теперь работает с Unicode codepoints
                return std::make_unique<CharRange>(start_cp, end_cp);
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
        (void)opt; // Подавляем предупреждение о неиспользуемой переменной
        return true; // Опциональные элементы всегда продуктивны (могут быть пустыми)
    } else if (const auto* zeroMore = dynamic_cast<const ZeroOrMore*>(node)) {
        (void)zeroMore; // Подавляем предупреждение о неиспользуемой переменной
        return true; // Повторение 0+ всегда продуктивно (может быть пустым)
    } else if (const auto* oneMore = dynamic_cast<const OneOrMore*>(node)) {
        return isProductive(oneMore->content.get(), productive);
    }
    
    return false;
}

// Extended BNF методы для контекстно-зависимых грамматик

std::vector<RuleParameter> BNFParser::parseRuleParameters() {
    // [param1, param2:type, param3:enum{val1,val2}]
    std::vector<RuleParameter> parameters;
    
    if (!match(TokenType::LEFT_BRACKET)) {
        error("Expected '[' to start parameter list");
        return parameters;
    }
    
    // Парсим первый параметр
    if (!check(TokenType::RIGHT_BRACKET)) {
        parameters.push_back(parseRuleParameter());
        
        // Парсим остальные параметры
        while (match(TokenType::COMMA)) {
            parameters.push_back(parseRuleParameter());
        }
    }
    
    if (!match(TokenType::RIGHT_BRACKET)) {
        error("Expected ']' to end parameter list");
    }
    
    return parameters;
}

RuleParameter BNFParser::parseRuleParameter() {
    // param:type или param:enum{val1,val2} или просто param
    if (!check(TokenType::IDENTIFIER)) {
        error("Expected parameter name");
        return RuleParameter("", ParameterType::STRING);
    }
    
    std::string paramName = advance().value;
    
    // Если есть двоеточие, парсим тип
    if (match(TokenType::COLON)) {
        ParameterType type = parseParameterType();
        
        if (type == ParameterType::ENUM) {
            // Для enum нужно распарсить значения
            std::vector<std::string> enumValues = parseEnumValues();
            return RuleParameter(paramName, enumValues);
        } else {
            return RuleParameter(paramName, type);
        }
    }
    
    // По умолчанию - строковый тип
    return RuleParameter(paramName, ParameterType::STRING);
}

ParameterType BNFParser::parseParameterType() {
    // int | string | bool | enum{...}
    if (!check(TokenType::IDENTIFIER)) {
        error("Expected parameter type");
        return ParameterType::STRING;
    }
    
    std::string typeName = advance().value;
    
    if (typeName == "int" || typeName == "integer") {
        return ParameterType::INTEGER;
    } else if (typeName == "string" || typeName == "str") {
        return ParameterType::STRING;
    } else if (typeName == "bool" || typeName == "boolean") {
        return ParameterType::BOOLEAN;
    } else if (typeName == "enum") {
        return ParameterType::ENUM;
    } else {
        error("Unknown parameter type: " + typeName);
        return ParameterType::STRING;
    }
}

std::vector<std::string> BNFParser::parseEnumValues() {
    // {val1, val2, val3}
    std::vector<std::string> values;
    
    if (!match(TokenType::LEFT_BRACE)) {
        error("Expected '{' to start enum values");
        return values;
    }
    
    // Парсим первое значение
    if (!check(TokenType::RIGHT_BRACE)) {
        if (!check(TokenType::IDENTIFIER)) {
            error("Expected enum value");
            return values;
        }
        values.push_back(advance().value);
        
        // Парсим остальные значения
        while (match(TokenType::COMMA)) {
            if (!check(TokenType::IDENTIFIER)) {
                error("Expected enum value after ','");
                break;
            }
            values.push_back(advance().value);
        }
    }
    
    if (!match(TokenType::RIGHT_BRACE)) {
        error("Expected '}' to end enum values");
    }
    
    return values;
}

std::vector<std::string> BNFParser::parseParameterValues() {
    // [val1, val2] для вызовов нетерминалов
    std::vector<std::string> values;
    
    if (!match(TokenType::LEFT_BRACKET)) {
        error("Expected '[' to start parameter values");
        return values;
    }
    
    // Парсим первое значение
    if (!check(TokenType::RIGHT_BRACKET)) {
        if (!check(TokenType::IDENTIFIER)) {
            error("Expected parameter value");
            return values;
        }
        values.push_back(advance().value);
        
        // Парсим остальные значения
        while (match(TokenType::COMMA)) {
            if (!check(TokenType::IDENTIFIER)) {
                error("Expected parameter value after ','");
                break;
            }
            values.push_back(advance().value);
        }
    }
    
    if (!match(TokenType::RIGHT_BRACKET)) {
        error("Expected ']' to end parameter values");
    }
    
    return values;
}

std::unique_ptr<ContextAction> BNFParser::parseContextAction() {
    // {store(name, value)} | {lookup(name)} | {check(condition)}
    if (!match(TokenType::LEFT_BRACE)) {
        error("Expected '{' to start context action");
        return nullptr;
    }
    
    if (!check(TokenType::IDENTIFIER)) {
        error("Expected action name");
        return nullptr;
    }
    
    std::string actionName = advance().value;
    std::vector<std::string> arguments;
    
    if (!match(TokenType::LEFT_PAREN)) {
        error("Expected '(' after action name");
        return nullptr;
    }
    
    // Парсим аргументы
    if (!check(TokenType::RIGHT_PAREN)) {
        if (!check(TokenType::IDENTIFIER)) {
            error("Expected argument");
            return nullptr;
        }
        arguments.push_back(advance().value);
        
        while (match(TokenType::COMMA)) {
            if (!check(TokenType::IDENTIFIER)) {
                error("Expected argument after ','");
                break;
            }
            arguments.push_back(advance().value);
        }
    }
    
    if (!match(TokenType::RIGHT_PAREN)) {
        error("Expected ')' after arguments");
        return nullptr;
    }
    
    if (!match(TokenType::RIGHT_BRACE)) {
        error("Expected '}' to end context action");
        return nullptr;
    }
    
    // Определяем тип действия
    ContextAction::ActionType actionType;
    if (actionName == "store") {
        actionType = ContextAction::ActionType::STORE;
    } else if (actionName == "lookup") {
        actionType = ContextAction::ActionType::LOOKUP;
    } else if (actionName == "check") {
        actionType = ContextAction::ActionType::CHECK;
    } else {
        error("Unknown action type: " + actionName);
        return nullptr;
    }
    
    return std::make_unique<ContextAction>(actionType, arguments);
}

std::unique_ptr<NonTerminal> BNFParser::parseParameterizedNonTerminal() {
    // <rule>[param1, param2] или просто <rule>
    if (!check(TokenType::IDENTIFIER)) {
        error("Expected non-terminal name");
        return nullptr;
    }
    
    std::string name = advance().value;
    
    // Проверяем, есть ли параметры
    if (check(TokenType::LEFT_BRACKET)) {
        std::vector<std::string> paramValues = parseParameterValues();
        return std::make_unique<NonTerminal>(name, paramValues);
    } else {
        return std::make_unique<NonTerminal>(name);
    }
}

} // namespace bnf_parser_generator
