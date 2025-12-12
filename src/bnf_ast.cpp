#include "bnf_ast.hpp"
#include "utf8_utils.hpp"
#include <sstream>
#include <iomanip>

namespace bnf_parser_generator {

using namespace bnf_parser_generator;

std::string CharRange::toString(int indent) const {
    (void)indent;
    return "'" + utf8::codepointToUtf8(start) + "'..'" + utf8::codepointToUtf8(end) + "'";
}

} // namespace bnf_parser_generator

