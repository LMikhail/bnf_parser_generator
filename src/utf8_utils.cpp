#include "utf8_utils.hpp"
#include <stdexcept>

namespace bnf_parser_generator {
namespace utf8 {

size_t charLength(unsigned char firstByte) {
    if ((firstByte & 0x80) == 0) {
        return 1; // 0xxxxxxx - ASCII (1 байт)
    } else if ((firstByte & 0xE0) == 0xC0) {
        return 2; // 110xxxxx - 2 байта
    } else if ((firstByte & 0xF0) == 0xE0) {
        return 3; // 1110xxxx - 3 байта
    } else if ((firstByte & 0xF8) == 0xF0) {
        return 4; // 11110xxx - 4 байта
    }
    return 1; // Некорректный UTF-8, обрабатываем как 1 байт
}

bool isValidSequence(const std::string& input, size_t pos, size_t length) {
    if (pos + length > input.length()) {
        return false; // Выходим за границы строки
    }
    
    if (length == 1) {
        // ASCII символ
        return (static_cast<unsigned char>(input[pos]) & 0x80) == 0;
    }
    
    // Проверяем continuation bytes (10xxxxxx)
    for (size_t i = 1; i < length; ++i) {
        if ((static_cast<unsigned char>(input[pos + i]) & 0xC0) != 0x80) {
            return false;
        }
    }
    
    return true;
}

std::string extractChar(const std::string& input, size_t pos, size_t& charLength) {
    if (pos >= input.length()) {
        charLength = 0;
        return "";
    }
    
    unsigned char firstByte = static_cast<unsigned char>(input[pos]);
    charLength = utf8::charLength(firstByte);
    
    // Проверяем валидность последовательности
    if (!isValidSequence(input, pos, charLength)) {
        // Если невалидная последовательность, берем только 1 байт
        charLength = 1;
        return std::string(1, input[pos]);
    }
    
    return input.substr(pos, charLength);
}

bool isWhitespace(const std::string& utf8Char) {
    if (utf8Char.length() == 1) {
        char c = utf8Char[0];
        return c == ' ' || c == '\t' || c == '\n' || c == '\r';
    }
    // Можно расширить для Unicode whitespace (U+00A0, U+2000-U+200B и т.д.)
    // Например:
    // - U+00A0 (NO-BREAK SPACE): 0xC2 0xA0
    // - U+2000-U+200B (различные пробелы): 0xE2 0x80 0x80-0x8B
    return false;
}

size_t length(const std::string& str) {
    size_t count = 0;
    size_t pos = 0;
    
    while (pos < str.length()) {
        size_t charLen;
        extractChar(str, pos, charLen);
        if (charLen == 0) break;
        pos += charLen;
        count++;
    }
    
    return count;
}

std::string codepointToUtf8(uint32_t codepoint) {
    std::string result;
    
    if (codepoint > 0x10FFFF) {
        throw std::runtime_error("Invalid Unicode codepoint: exceeds U+10FFFF");
    }
    
    // Surrogate pairs (U+D800 - U+DFFF) are invalid in UTF-8
    if (codepoint >= 0xD800 && codepoint <= 0xDFFF) {
        throw std::runtime_error("Invalid Unicode codepoint: surrogate pair range");
    }
    
    if (codepoint <= 0x7F) {
        // 1-byte sequence: 0xxxxxxx
        result += static_cast<char>(codepoint);
    } else if (codepoint <= 0x7FF) {
        // 2-byte sequence: 110xxxxx 10xxxxxx
        result += static_cast<char>(0xC0 | (codepoint >> 6));
        result += static_cast<char>(0x80 | (codepoint & 0x3F));
    } else if (codepoint <= 0xFFFF) {
        // 3-byte sequence: 1110xxxx 10xxxxxx 10xxxxxx
        result += static_cast<char>(0xE0 | (codepoint >> 12));
        result += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
        result += static_cast<char>(0x80 | (codepoint & 0x3F));
    } else {
        // 4-byte sequence: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
        result += static_cast<char>(0xF0 | (codepoint >> 18));
        result += static_cast<char>(0x80 | ((codepoint >> 12) & 0x3F));
        result += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
        result += static_cast<char>(0x80 | (codepoint & 0x3F));
    }
    
    return result;
}

uint32_t utf8ToCodepoint(const std::string& utf8Str) {
    if (utf8Str.empty()) {
        return 0;
    }
    
    unsigned char firstByte = static_cast<unsigned char>(utf8Str[0]);
    size_t len = charLength(firstByte);
    
    if (utf8Str.length() < len) {
        return 0; // Невалидная последовательность
    }
    
    uint32_t codepoint = 0;
    
    if (len == 1) {
        // ASCII: 0xxxxxxx
        codepoint = firstByte;
    } else if (len == 2) {
        // 2-byte: 110xxxxx 10xxxxxx
        codepoint = (firstByte & 0x1F) << 6;
        codepoint |= (static_cast<unsigned char>(utf8Str[1]) & 0x3F);
    } else if (len == 3) {
        // 3-byte: 1110xxxx 10xxxxxx 10xxxxxx
        codepoint = (firstByte & 0x0F) << 12;
        codepoint |= (static_cast<unsigned char>(utf8Str[1]) & 0x3F) << 6;
        codepoint |= (static_cast<unsigned char>(utf8Str[2]) & 0x3F);
    } else if (len == 4) {
        // 4-byte: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
        codepoint = (firstByte & 0x07) << 18;
        codepoint |= (static_cast<unsigned char>(utf8Str[1]) & 0x3F) << 12;
        codepoint |= (static_cast<unsigned char>(utf8Str[2]) & 0x3F) << 6;
        codepoint |= (static_cast<unsigned char>(utf8Str[3]) & 0x3F);
    }
    
    return codepoint;
}

// Utf8Iterator implementation

Utf8Iterator::Utf8Iterator(const std::string& str)
    : str_(str), pos_(0), charIdx_(0) {}

bool Utf8Iterator::atEnd() const {
    return pos_ >= str_.length();
}

std::string Utf8Iterator::current() const {
    if (atEnd()) return "";
    size_t charLen;
    return extractChar(str_, pos_, charLen);
}

size_t Utf8Iterator::position() const {
    return pos_;
}

size_t Utf8Iterator::charIndex() const {
    return charIdx_;
}

void Utf8Iterator::next() {
    if (atEnd()) return;
    
    size_t charLen;
    extractChar(str_, pos_, charLen);
    pos_ += charLen;
    charIdx_++;
}

} // namespace utf8
} // namespace bnf_parser_generator

