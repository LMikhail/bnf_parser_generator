#include "utf8_utils.hpp"

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

