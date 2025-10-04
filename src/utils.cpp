#include "bnf_engine.hpp"
#include <sstream>

namespace bnf_engine {
namespace utils {

std::string tokensToString(const std::vector<Token>& tokens) {
    std::ostringstream oss;
    for (size_t i = 0; i < tokens.size(); ++i) {
        if (i > 0) oss << " ";
        oss << tokens[i].value;
    }
    return oss.str();
}

std::vector<Token> findTokensByType(const std::vector<Token>& tokens, const std::string& type) {
    std::vector<Token> result;
    for (const auto& token : tokens) {
        if (token.type == type) {
            result.push_back(token);
        }
    }
    return result;
}

TokenStats analyzeTokens(const std::vector<Token>& tokens) {
    TokenStats stats;
    stats.total_tokens = tokens.size();
    
    for (const auto& token : tokens) {
        stats.type_counts[token.type]++;
    }
    
    stats.unique_types = stats.type_counts.size();
    return stats;
}

} // namespace utils
} // namespace bnf_engine
