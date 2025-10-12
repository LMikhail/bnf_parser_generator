#include "bnf_ast.hpp"
#include "utf8_utils.hpp"
#include <sstream>
#include <iomanip>

namespace bnf_parser_generator {

std::string CharRange::toString() const {
    auto codepointToString = [](uint32_t cp) -> std::string {
        if (cp <= 0x7F && cp >= 0x20 && cp != '\\' && cp != '\'') {
            // Printable ASCII (кроме спец. символов)
            return "'" + utf8::codepointToUtf8(cp) + "'";
        } else if (cp <= 0xFFFF) {
            // 4-значная Unicode escape-последовательность
            std::ostringstream oss;
            oss << "'\\u" << std::hex << std::setw(4) << std::setfill('0') << cp << "'";
            return oss.str();
        } else {
            // 8-значная Unicode escape-последовательность
            std::ostringstream oss;
            oss << "'\\U" << std::hex << std::setw(8) << std::setfill('0') << cp << "'";
            return oss.str();
        }
    };
    
    return codepointToString(start) + ".." + codepointToString(end);
}

} // namespace bnf_parser_generator

