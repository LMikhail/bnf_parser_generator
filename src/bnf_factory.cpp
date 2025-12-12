#include "bnf_parser.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace bnf_parser_generator {

std::unique_ptr<Grammar> BNFGrammarFactory::fromString(const std::string& bnfText) {
    BNFLexer lexer(bnfText);
    auto tokens = lexer.tokenize();
    
    BNFParser parser(tokens);
    auto grammar = parser.parseGrammar();
    
    if (!grammar) {
        throw std::runtime_error("Failed to parse BNF grammar: " + parser.getError());
    }
    
    // Валидация грамматики
    auto validation = BNFParser::validateGrammar(*grammar);
    if (!validation.isValid) {
        std::string errorMsg = "Grammar validation failed:\n";
        for (const auto& error : validation.errors) {
            errorMsg += "  Error: " + error + "\n";
        }
        for (const auto& warning : validation.warnings) {
            errorMsg += "  Warning: " + warning + "\n";
        }
        throw std::runtime_error(errorMsg);
    }
    
    return grammar;
}

std::unique_ptr<Grammar> BNFGrammarFactory::fromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open grammar file: " + filename);
    }
    
    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    
    return fromString(content);
}

std::unique_ptr<Grammar> BNFGrammarFactory::createJSONGrammar() {
    const std::string jsonBNF = R"(
# JSON Grammar (RFC 7159 compliant)
json ::= value

value ::= object | array | string | number | boolean | null

object ::= '{' [member (',' member)*] '}'
member ::= string ':' value

array ::= '[' [value (',' value)*] ']'

string ::= '"' char* '"'
char ::= unescaped | escaped
unescaped ::= 'a'..'z' | 'A'..'Z' | '0'..'9' | ' ' | '!' | '#'..'[' | ']'..'~'
escaped ::= '\' ('"' | '\' | '/' | 'b' | 'f' | 'n' | 'r' | 't' | unicode)
unicode ::= 'u' hex hex hex hex
hex ::= '0'..'9' | 'a'..'f' | 'A'..'F'

number ::= ['-'] int ['.' digit+] [('e' | 'E') ['+' | '-'] digit+]
int ::= '0' | ('1'..'9' digit*)
digit ::= '0'..'9'

boolean ::= 'true' | 'false'
null ::= 'null'
)";
    
    return fromString(jsonBNF);
}

std::unique_ptr<Grammar> BNFGrammarFactory::createPrologGrammar() {
    const std::string prologBNF = R"(
# Prolog Grammar
program ::= clause*

clause ::= fact | rule | query
fact ::= term '.'
rule ::= term ':-' body '.'
query ::= '?-' body '.'

body ::= term (',' term)*

term ::= atom | variable | number | string | compound | list
compound ::= atom '(' args ')'
args ::= term (',' term)*

list ::= '[' [list_elements] ']'
list_elements ::= term (',' term)* ['|' term]

atom ::= lowercase (alphanumeric | '_')*
variable ::= (uppercase | '_') (alphanumeric | '_')*
number ::= ['-'] digit+ ['.' digit+]
string ::= '"' char* '"'
char ::= 'a'..'z' | 'A'..'Z' | '0'..'9' | ' ' | '!' | '#'..'~'

lowercase ::= 'a'..'z'
uppercase ::= 'A'..'Z'
digit ::= '0'..'9'
alphanumeric ::= lowercase | uppercase | digit
)";
    
    return fromString(prologBNF);
}

std::unique_ptr<Grammar> BNFGrammarFactory::createClojureGrammar() {
    const std::string clojureBNF = R"(
# Clojure (EDN) Grammar
program ::= form*

form ::= literal | collection | tagged_form | reader_macro

literal ::= number | string | character | keyword | symbol | boolean | nil

collection ::= list | vector | map | set
list ::= '(' form* ')'
vector ::= '[' form* ']'
map ::= '{' (form form)* '}'
set ::= '#{' form* '}'

tagged_form ::= '#' symbol form

reader_macro ::= quote_form | syntax_quote_form | unquote_form | deref_form
quote_form ::= "'" form
syntax_quote_form ::= '`' form
unquote_form ::= ('~' | '~@') form
deref_form ::= '@' form

symbol ::= symbol_start symbol_char*
keyword ::= ':' symbol_char+
number ::= ['-'] (integer | decimal | ratio | scientific)
string ::= '"' string_char* '"'
character ::= '\' (named_char | unicode_char | any_char)
boolean ::= 'true' | 'false'
nil ::= 'nil'

symbol_start ::= letter | symbol_special
symbol_char ::= letter | digit | symbol_special
letter ::= 'a'..'z' | 'A'..'Z'
digit ::= '0'..'9'
symbol_special ::= '*' | '+' | '!' | '-' | '_' | '?' | '$' | '%' | '&' | '=' | '<' | '>' | '/' | '.'

integer ::= digit+
decimal ::= digit+ '.' digit+
ratio ::= integer '/' integer
scientific ::= (integer | decimal) ('e' | 'E') ['+' | '-'] integer

string_char ::= 'a'..'z' | 'A'..'Z' | '0'..'9' | ' ' | '!' | '#'..'~'
named_char ::= 'newline' | 'space' | 'tab' | 'return'
unicode_char ::= 'u' hex_digit hex_digit hex_digit hex_digit
any_char ::= 'a'..'z' | 'A'..'Z' | '0'..'9'
hex_digit ::= digit | 'a'..'f' | 'A'..'F'
)";
    
    return fromString(clojureBNF);
}

std::unique_ptr<Grammar> BNFGrammarFactory::createArithmeticGrammar() {
    return fromString(R"(
        expr   ::= term { ('+' | '-') term };
        term   ::= factor { ('*' | '/') factor };
        factor ::= NUMBER | '(' expr ')';
        NUMBER ::= ('0'..'9')+;
    )");
}

std::unique_ptr<Grammar> BNFGrammarFactory::createIdentifierGrammar() {
    const std::string identifierBNF = R"(
# Identifier Grammar
identifier ::= letter (letter | digit | '_')*
letter ::= 'a'..'z' | 'A'..'Z'
digit ::= '0'..'9'
)";
    
    return fromString(identifierBNF);
}

} // namespace bnf_parser_generator
