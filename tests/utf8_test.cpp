#include "utf8_utils.hpp"
#include <iostream>
#include <cassert>

int main() {
    std::cout << "=== UTF-8 Utils Tests ===" << std::endl;
    
    using namespace bnf_parser_generator::utf8;
    
    try {
        // Тест 1: ASCII символы
        std::string ascii = "Hello";
        assert(length(ascii) == 5);
        std::cout << "✓ ASCII length" << std::endl;
        
        // Тест 2: UTF-8 символы (русский текст)
        std::string russian = "Привет";  // 6 символов
        assert(length(russian) == 6);
        std::cout << "✓ UTF-8 length (Russian)" << std::endl;
        
        // Тест 3: Смешанный текст
        std::string mixed = "Hello Мир";  // 9 символов (5 + 1 + 3)
        assert(length(mixed) == 9);
        std::cout << "✓ Mixed ASCII/UTF-8 length" << std::endl;
        
        // Тест 4: Извлечение символов
        size_t charLen = 0;
        std::string firstChar = extractChar(russian, 0, charLen);
        assert(charLen == 2);  // 'П' занимает 2 байта
        assert(firstChar.length() == 2);
        std::cout << "✓ Extract UTF-8 character" << std::endl;
        
        // Тест 5: Извлечение ASCII
        std::string asciiChar = extractChar(ascii, 0, charLen);
        assert(charLen == 1);
        assert(asciiChar == "H");
        std::cout << "✓ Extract ASCII character" << std::endl;
        
        // Тест 6: Итератор
        size_t count = 0;
        for (Utf8Iterator it(russian); !it.atEnd(); it.next()) {
            std::string ch = it.current();
            assert(!ch.empty());
            assert(it.charIndex() == count);
            count++;
        }
        assert(count == 6);
        std::cout << "✓ Iterator" << std::endl;
        
        // Тест 7: Whitespace detection
        assert(isWhitespace(" "));
        assert(isWhitespace("\t"));
        assert(isWhitespace("\n"));
        assert(isWhitespace("\r"));
        assert(!isWhitespace("A"));
        assert(!isWhitespace("П"));
        std::cout << "✓ Whitespace detection" << std::endl;
        
        // Тест 8: charLength function
        assert(charLength(static_cast<unsigned char>('A')) == 1);  // ASCII
        assert(charLength(static_cast<unsigned char>(0xD0)) == 2);  // Russian letter (2 bytes)
        assert(charLength(static_cast<unsigned char>(0xE0)) == 3);  // 3-byte UTF-8
        assert(charLength(static_cast<unsigned char>(0xF0)) == 4);  // 4-byte UTF-8
        std::cout << "✓ Character length detection" << std::endl;
        
        // Тест 9: Пустая строка
        assert(length("") == 0);
        std::cout << "✓ Empty string" << std::endl;
        
        // Тест 10: Граничные случаи
        size_t len;
        std::string empty = extractChar("", 0, len);
        assert(len == 0);
        assert(empty.empty());
        std::cout << "✓ Edge cases" << std::endl;
        
        std::cout << "\nВсе тесты прошли успешно" << std::endl;
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
        return 1;
    }
}

