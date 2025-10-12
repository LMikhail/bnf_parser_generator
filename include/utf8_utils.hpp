#pragma once

#include <string>
#include <cstddef>
#include <cstdint>

namespace bnf_parser_generator {
namespace utf8 {

/**
 * @brief Определяет длину UTF-8 символа в байтах по первому байту
 * @param firstByte Первый байт UTF-8 последовательности
 * @return Длина символа в байтах (1-4), или 1 для некорректных последовательностей
 */
size_t charLength(unsigned char firstByte);

/**
 * @brief Проверяет валидность UTF-8 последовательности
 * @param input Строка для проверки
 * @param pos Позиция начала последовательности
 * @param length Ожидаемая длина последовательности
 * @return true если последовательность валидна
 */
bool isValidSequence(const std::string& input, size_t pos, size_t length);

/**
 * @brief Извлекает следующий UTF-8 символ из строки
 * @param input Входная строка
 * @param pos Позиция начала символа
 * @param charLength Выходной параметр - длина извлеченного символа в байтах
 * @return UTF-8 символ как строка
 * 
 * Если встречена невалидная последовательность, возвращает один байт.
 */
std::string extractChar(const std::string& input, size_t pos, size_t& charLength);

/**
 * @brief Проверяет, является ли UTF-8 символ пробельным (ASCII whitespace)
 * @param utf8Char UTF-8 символ для проверки
 * @return true если символ - пробел, табуляция, перевод строки или возврат каретки
 * 
 * Примечание: В будущем можно расширить для Unicode whitespace (U+00A0, U+2000-U+200B и т.д.)
 */
bool isWhitespace(const std::string& utf8Char);

/**
 * @brief Подсчитывает количество UTF-8 символов (не байтов) в строке
 * @param str Строка для анализа
 * @return Количество UTF-8 символов
 */
size_t length(const std::string& str);

/**
 * @brief Преобразует Unicode codepoint в UTF-8 строку
 * @param codepoint Unicode codepoint (U+0000 - U+10FFFF)
 * @return UTF-8 представление символа
 * @throws std::runtime_error если codepoint вне допустимого диапазона
 */
std::string codepointToUtf8(uint32_t codepoint);

/**
 * @brief Извлекает первый Unicode codepoint из UTF-8 строки
 * @param utf8Str UTF-8 строка
 * @return Unicode codepoint первого символа, или 0 если строка пуста или невалидна
 */
uint32_t utf8ToCodepoint(const std::string& utf8Str);

/**
 * @brief Класс для итерации по UTF-8 символам в строке
 * 
 * Пример использования:
 * @code
 * std::string text = "Привет мир";
 * for (Utf8Iterator it(text); !it.atEnd(); it.next()) {
 *     std::string ch = it.current();
 *     size_t pos = it.position();
 *     // обработка символа
 * }
 * @endcode
 */
class Utf8Iterator {
public:
    explicit Utf8Iterator(const std::string& str);
    
    bool atEnd() const;
    std::string current() const;
    size_t position() const;  // Байтовая позиция
    size_t charIndex() const; // Номер символа
    void next();
    
private:
    const std::string& str_;
    size_t pos_;
    size_t charIdx_;
};

} // namespace utf8
} // namespace bnf_parser_generator

