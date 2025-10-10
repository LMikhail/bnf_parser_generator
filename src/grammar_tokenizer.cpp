#include "grammar_tokenizer.hpp"
#include "bnf_parser.hpp"
#include <algorithm>
#include <cctype>
#include <sstream>

namespace bnf_parser {

GrammarBasedTokenizer::GrammarBasedTokenizer(const Grammar& grammar) 
    : grammar_(grammar) {}

std::vector<Token> GrammarBasedTokenizer::tokenize(const std::string& input) {
    std::vector<Token> tokens;
    size_t pos = 0;
    size_t line = 1;
    size_t column = 1;
    lastError_.clear();
    
    while (pos < input.length()) {
        size_t startPos = pos;
        size_t startColumn = column;
        
        // Пропускаем пробелы и комментарии
        if (skipWhitespace_ || skipComments_) {
            skipWhitespaceAndComments(input, pos, line, column);
            if (pos >= input.length()) break;
        }
        
        // Пытаемся сопоставить токены согласно грамматике
        bool matched = false;
        MatchResult bestMatch;
        
        // Проверяем все правила грамматики, начиная с терминальных
        auto tokenTypes = getTokenTypes();
        
        for (const std::string& tokenType : tokenTypes) {
            MatchResult match = tryMatchRule(input, pos, tokenType);
            if (match.matched && match.length > bestMatch.length) {
                bestMatch = match;
            }
        }
        
        if (bestMatch.matched) {
            tokens.emplace_back(bestMatch.tokenType, bestMatch.value, line, startColumn, startPos);
            
            // Обновляем позицию
            for (size_t i = 0; i < bestMatch.length; ++i) {
                if (input[pos] == '\n') {
                    line++;
                    column = 1;
                } else {
                    column++;
                }
                pos++;
            }
            matched = true;
        }
        
        if (!matched) {
            lastError_ = "Unexpected character '" + std::string(1, input[pos]) + 
                        "' at line " + std::to_string(line) + 
                        ", column " + std::to_string(column);
            break;
        }
    }
    
    // Добавляем EOF токен
    tokens.emplace_back("EOF", "", line, column, pos);
    return tokens;
}

std::string GrammarBasedTokenizer::generateRegex(const ASTNode* node) const {
    return generateRegexWithDepth(node, 0);
}

std::string GrammarBasedTokenizer::generateRegexWithDepth(const ASTNode* node, int depth) const {
    // Защита от бесконечной рекурсии
    if (depth > 100) {
        return ""; // Слишком глубокая рекурсия
    }
    
    if (const auto* terminal = dynamic_cast<const Terminal*>(node)) {
        return generateTerminalRegex(terminal);
    } else if (const auto* charRange = dynamic_cast<const CharRange*>(node)) {
        return generateCharRangeRegex(charRange);
    } else if (const auto* alt = dynamic_cast<const Alternative*>(node)) {
        return generateAlternativeRegex(alt);
    } else if (const auto* seq = dynamic_cast<const Sequence*>(node)) {
        return generateSequenceRegex(seq);
    } else if (const auto* opt = dynamic_cast<const Optional*>(node)) {
        return generateOptionalRegex(opt);
    } else if (const auto* zeroMore = dynamic_cast<const ZeroOrMore*>(node)) {
        return generateZeroOrMoreRegex(zeroMore);
    } else if (const auto* oneMore = dynamic_cast<const OneOrMore*>(node)) {
        return generateOneOrMoreRegex(oneMore);
    } else if (const auto* group = dynamic_cast<const Group*>(node)) {
        return generateGroupRegex(group);
    } else if (const auto* nonTerminal = dynamic_cast<const NonTerminal*>(node)) {
        // Рекурсивно разворачиваем нетерминал с ограничением глубины
        const ProductionRule* rule = grammar_.findRule(nonTerminal->name);
        if (rule) {
            return generateRegexWithDepth(rule->rightSide.get(), depth + 1);
        }
        return ""; // Неопределённый нетерминал
    }
    
    return "";
}

std::string GrammarBasedTokenizer::generateTerminalRegex(const Terminal* terminal) const {
    return escapeRegex(terminal->value);
}

std::string GrammarBasedTokenizer::generateCharRangeRegex(const CharRange* range) const {
    std::string result = "[";
    if (range->start <= range->end) {
        result += range->start;
        result += "-";
        result += range->end;
    }
    result += "]";
    return result;
}

std::string GrammarBasedTokenizer::generateAlternativeRegex(const Alternative* alt) const {
    std::string result = "(";
    for (size_t i = 0; i < alt->choices.size(); ++i) {
        if (i > 0) result += "|";
        result += generateRegex(alt->choices[i].get());
    }
    result += ")";
    return result;
}

std::string GrammarBasedTokenizer::generateSequenceRegex(const Sequence* seq) const {
    std::string result = "(";
    for (const auto& element : seq->elements) {
        result += generateRegex(element.get());
    }
    result += ")";
    return result;
}

std::string GrammarBasedTokenizer::generateOptionalRegex(const Optional* opt) const {
    return "(" + generateRegex(opt->content.get()) + ")?";
}

std::string GrammarBasedTokenizer::generateZeroOrMoreRegex(const ZeroOrMore* zeroMore) const {
    return "(" + generateRegex(zeroMore->content.get()) + ")*";
}

std::string GrammarBasedTokenizer::generateOneOrMoreRegex(const OneOrMore* oneMore) const {
    return "(" + generateRegex(oneMore->content.get()) + ")+";
}

std::string GrammarBasedTokenizer::generateGroupRegex(const Group* group) const {
    return "(" + generateRegex(group->content.get()) + ")";
}

GrammarBasedTokenizer::MatchResult 
GrammarBasedTokenizer::tryMatchRule(const std::string& input, size_t pos, const std::string& ruleName) const {
    const ProductionRule* rule = grammar_.findRule(ruleName);
    if (!rule) {
        return MatchResult();
    }
    
    return tryMatchNode(input, pos, rule->rightSide.get(), ruleName);
}

GrammarBasedTokenizer::MatchResult 
GrammarBasedTokenizer::tryMatchNode(const std::string& input, size_t pos, const ASTNode* node, const std::string& ruleName) const {
    // Генерируем регулярное выражение для узла
    std::string pattern = generateRegex(node);
    if (pattern.empty()) {
        return MatchResult();
    }
    
    // Кэшируем скомпилированные регексы
    auto it = terminalRegexCache_.find(pattern);
    if (it == terminalRegexCache_.end()) {
        try {
            terminalRegexCache_[pattern] = std::regex("^" + pattern);
            it = terminalRegexCache_.find(pattern);
        } catch (const std::regex_error&) {
            return MatchResult(); // Некорректное регулярное выражение
        }
    }
    
    // Пытаемся сопоставить с начала текущей позиции
    std::string substr = input.substr(pos);
    std::smatch match;
    
    if (std::regex_search(substr, match, it->second) && match.position() == 0) {
        std::string matchedText = match.str();
        return MatchResult(ruleName, matchedText, matchedText.length());
    }
    
    return MatchResult();
}

std::string GrammarBasedTokenizer::escapeRegex(const std::string& str) const {
    std::string result;
    for (char c : str) {
        switch (c) {
            case '.': case '^': case '$': case '*': case '+': case '?':
            case '(': case ')': case '[': case ']': case '{': case '}':
            case '|': case '\\':
                result += '\\';
                result += c;
                break;
            default:
                result += c;
                break;
        }
    }
    return result;
}

bool GrammarBasedTokenizer::isWhitespace(char c) const {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

void GrammarBasedTokenizer::skipWhitespaceAndComments(const std::string& input, size_t& pos, size_t& line, size_t& column) const {
    while (pos < input.length()) {
        char c = input[pos];
        
        // Пропускаем пробелы
        if (skipWhitespace_ && isWhitespace(c)) {
            if (c == '\n') {
                line++;
                column = 1;
            } else {
                column++;
            }
            pos++;
            continue;
        }
        
        // Пропускаем комментарии
        if (skipComments_ && c == '#') {
            while (pos < input.length() && input[pos] != '\n') {
                pos++;
            }
            continue;
        }
        
        break;
    }
}

std::vector<std::string> GrammarBasedTokenizer::getTokenTypes() const {
    std::vector<std::string> types;
    
    // Сначала добавляем терминальные правила (они имеют приоритет)
    for (const auto& rule : grammar_.rules) {
        if (isTerminalRule(rule->leftSide)) {
            types.push_back(rule->leftSide);
        }
    }
    
    // Затем добавляем остальные правила
    for (const auto& rule : grammar_.rules) {
        if (!isTerminalRule(rule->leftSide)) {
            types.push_back(rule->leftSide);
        }
    }
    
    return types;
}

bool GrammarBasedTokenizer::isTerminalRule(const std::string& ruleName) const {
    // Эвристика: правила, состоящие только из терминалов и символьных диапазонов,
    // считаются терминальными правилами (лексемами)
    const ProductionRule* rule = grammar_.findRule(ruleName);
    if (!rule) return false;
    
    return isTerminalNode(rule->rightSide.get());
}

bool GrammarBasedTokenizer::isTerminalNode(const ASTNode* node) const {
    if (dynamic_cast<const Terminal*>(node) || dynamic_cast<const CharRange*>(node)) {
        return true;
    } else if (const auto* alt = dynamic_cast<const Alternative*>(node)) {
        for (const auto& choice : alt->choices) {
            if (!isTerminalNode(choice.get())) {
                return false;
            }
        }
        return true;
    } else if (const auto* seq = dynamic_cast<const Sequence*>(node)) {
        for (const auto& element : seq->elements) {
            if (!isTerminalNode(element.get())) {
                return false;
            }
        }
        return true;
    } else if (const auto* opt = dynamic_cast<const Optional*>(node)) {
        return isTerminalNode(opt->content.get());
    } else if (const auto* zeroMore = dynamic_cast<const ZeroOrMore*>(node)) {
        return isTerminalNode(zeroMore->content.get());
    } else if (const auto* oneMore = dynamic_cast<const OneOrMore*>(node)) {
        return isTerminalNode(oneMore->content.get());
    } else if (const auto* group = dynamic_cast<const Group*>(node)) {
        return isTerminalNode(group->content.get());
    } else if (const auto* nonTerminal = dynamic_cast<const NonTerminal*>(node)) {
        // Рекурсивно проверяем нетерминал
        const ProductionRule* rule = grammar_.findRule(nonTerminal->name);
        if (rule) {
            return isTerminalNode(rule->rightSide.get());
        }
    }
    
    return false;
}

// Фабрика токенизаторов
std::unique_ptr<Tokenizer> TokenizerFactory::createFromGrammar(const Grammar& grammar) {
    return std::make_unique<GrammarBasedTokenizer>(grammar);
}

std::unique_ptr<Tokenizer> TokenizerFactory::createFromFile(const std::string& filename) {
    auto grammar = BNFGrammarFactory::fromFile(filename);
    return createFromGrammar(*grammar);
}

std::unique_ptr<Tokenizer> TokenizerFactory::createFromString(const std::string& bnfText) {
    auto grammar = BNFGrammarFactory::fromString(bnfText);
    return createFromGrammar(*grammar);
}

} // namespace bnf_parser
