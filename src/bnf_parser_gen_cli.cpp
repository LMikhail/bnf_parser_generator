#include "bnf_parser.hpp"
#include "code_generator.hpp"
#include "version.hpp"
#include <iostream>
#include <fstream>
#include <cstring>
#include <cctype>

using namespace bnf_parser_generator;

struct CliOptions {
    std::string input_file;
    std::string output_file;
    std::string output_dir = ".";
    std::string language = "cpp";
    std::string parser_name = "GeneratedParser";
    std::string namespace_name;
    std::string format = "source-only"; // source-only, library-static, library-shared, executable, all
    bool verbose = false;
    bool debug_mode = false;
    bool show_help = false;
    bool show_version = false;
    bool generate_executable = false;
    bool compile = false;
};

void printHelp(const char* program_name) {
    std::cout << "BNF Parser Generator - Generate standalone parsers from BNF/EBNF grammars\n";
    std::cout << "\nUsage: " << program_name << " [options]\n";
    std::cout << "\nOptions:\n";
    std::cout << "  -i, --input FILE       Input BNF/EBNF grammar file (required)\n";
    std::cout << "  -o, --output FILE      Output parser file name (without path)\n";
    std::cout << "  --output-dir DIR       Output directory for generated files\n";
    std::cout << "                         Default: generated/<bnf_name>/<format>/\n";
    std::cout << "                         For executables: generated/<bnf_name>/exec/<debug|release>/\n";
    std::cout << "  -l, --language LANG    Target language: cpp, dart, java, clojure (default: cpp)\n";
    std::cout << "  -n, --name NAME        Parser class name (default: GeneratedParser)\n";
    std::cout << "  --namespace NAME       Namespace/package name (optional)\n";
    std::cout << "  -f, --format FORMAT    Output format (default: source-only)\n";
    std::cout << "                         source-only    - Only source code files\n";
    std::cout << "                         library-static - Compile as static library (.a)\n";
    std::cout << "                         library-shared - Compile as shared library (.so)\n";
    std::cout << "                         executable     - Compile as standalone executable\n";
    std::cout << "                         all            - Generate source + all binaries\n";
    std::cout << "  -c, --compile          Auto-compile generated code (implies format selection)\n";
    std::cout << "  -v, --verbose          Verbose output\n";
    std::cout << "  -d, --debug            Generate debug code\n";
    std::cout << "  -e, --executable       Generate standalone executable (with main.cpp)\n";
    std::cout << "  -h, --help             Show this help message\n";
    std::cout << "  --version              Show version information\n";
    std::cout << "\nExamples:\n";
    std::cout << "  # Generate source code only:\n";
    std::cout << "  " << program_name << " -i json.bnf -o JsonParser.cpp\n\n";
    std::cout << "  # Generate with standalone executable:\n";
    std::cout << "  " << program_name << " -i json.bnf -o JsonParser.cpp --executable\n\n";
    std::cout << "  # Compile generated parser as executable:\n";
    std::cout << "  g++ -std=c++20 -o json_parser JsonParser_main.cpp JsonParser.cpp\n\n";
    std::cout << "  # Compile as shared library:\n";
    std::cout << "  g++ -std=c++20 -shared -fPIC -o libJsonParser.so JsonParser.cpp\n\n";
    std::cout << "  # Compile as static library:\n";
    std::cout << "  g++ -std=c++20 -c JsonParser.cpp -o JsonParser.o\n";
    std::cout << "  ar rcs libJsonParser.a JsonParser.o\n";
    std::cout << "  " << program_name << " -i calc.bnf -o calculator.cpp --namespace calc --verbose\n";
    std::cout << "  " << program_name << " -i json.bnf --executable --name JsonParser\n";
    std::cout << "\nSupported Languages:\n";
    auto langs = CodeGeneratorFactory::getSupportedLanguages();
    for (const auto& lang : langs) {
        std::cout << "  - " << lang << "\n";
    }
}

void printVersion() {
    std::cout << version::getFullVersionInfo() << "\n";
}

bool parseArgs(int argc, char* argv[], CliOptions& options) {
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "-h" || arg == "--help") {
            options.show_help = true;
            return true;
        } else if (arg == "--version") {
            options.show_version = true;
            return true;
        } else if ((arg == "-i" || arg == "--input") && i + 1 < argc) {
            options.input_file = argv[++i];
        } else if ((arg == "-o" || arg == "--output") && i + 1 < argc) {
            options.output_file = argv[++i];
        } else if (arg == "--output-dir" && i + 1 < argc) {
            options.output_dir = argv[++i];
        } else if ((arg == "-l" || arg == "--language") && i + 1 < argc) {
            options.language = argv[++i];
        } else if ((arg == "-n" || arg == "--name") && i + 1 < argc) {
            options.parser_name = argv[++i];
        } else if (arg == "--namespace" && i + 1 < argc) {
            options.namespace_name = argv[++i];
        } else if ((arg == "-f" || arg == "--format") && i + 1 < argc) {
            options.format = argv[++i];
        } else if (arg == "-c" || arg == "--compile") {
            options.compile = true;
        } else if (arg == "-v" || arg == "--verbose") {
            options.verbose = true;
        } else if (arg == "-d" || arg == "--debug") {
            options.debug_mode = true;
        } else if (arg == "-e" || arg == "--executable") {
            options.generate_executable = true;
        } else {
            std::cerr << "Unknown option: " << arg << "\n";
            return false;
        }
    }
    
    return true;
}

bool validateOptions(const CliOptions& options) {
    if (options.input_file.empty()) {
        std::cerr << "Error: Input file is required\n";
        std::cerr << "Use --help for usage information\n";
        return false;
    }
    
    if (!CodeGeneratorFactory::isLanguageSupported(options.language)) {
        std::cerr << "Error: Unsupported language: " << options.language << "\n";
        std::cerr << "Supported languages: ";
        auto langs = CodeGeneratorFactory::getSupportedLanguages();
        for (size_t i = 0; i < langs.size(); ++i) {
            if (i > 0) std::cerr << ", ";
            std::cerr << langs[i];
        }
        std::cerr << "\n";
        return false;
    }
    
    return true;
}

int main(int argc, char* argv[]) {
    CliOptions options;
    
    if (!parseArgs(argc, argv, options)) {
        return 1;
    }
    
    if (options.show_help) {
        printHelp(argv[0]);
        return 0;
    }
    
    if (options.show_version) {
        printVersion();
        return 0;
    }
    
    if (!validateOptions(options)) {
        return 1;
    }
    
    try {
        if (options.verbose) {
            std::cout << "BNF Parser Generator v" << version::getVersionString() << "\n";
            std::cout << "Input:  " << options.input_file << "\n";
            std::cout << "Language: " << options.language << "\n";
        }
        
        // Парсинг BNF грамматики
        if (options.verbose) {
            std::cout << "\n[1/3] Parsing BNF grammar...\n";
        }
        
        auto grammar = BNFGrammarFactory::fromFile(options.input_file);
        if (!grammar) {
            std::cerr << "Error: Failed to parse grammar file: " << options.input_file << "\n";
            return 1;
        }
        
        if (options.verbose) {
            std::cout << "  ✓ Parsed " << grammar->rules.size() << " rules\n";
            std::cout << "  ✓ Start symbol: " << grammar->startSymbol << "\n";
        }
        
        // Валидация грамматики
        if (options.verbose) {
            std::cout << "\n[2/3] Validating grammar...\n";
        }
        
        auto validation = BNFParser::validateGrammar(*grammar);
        if (!validation.isValid) {
            std::cerr << "Error: Grammar validation failed\n";
            for (const auto& error : validation.errors) {
                std::cerr << "  - " << error << "\n";
            }
            return 1;
        }
        
        if (options.verbose) {
            std::cout << "  ✓ Grammar is valid\n";
            if (!validation.warnings.empty()) {
                std::cout << "  Warnings:\n";
                for (const auto& warning : validation.warnings) {
                    std::cout << "    - " << warning << "\n";
                }
            }
        }
        
        // Генерация кода
        if (options.verbose) {
            std::cout << "\n[3/3] Generating parser code...\n";
        }
        
        auto generator = CodeGeneratorFactory::create(options.language);
        if (!generator) {
            std::cerr << "Error: Failed to create code generator for: " << options.language << "\n";
            return 1;
        }
        
        GeneratorOptions gen_options;
        gen_options.target_language = options.language;
        
        // Определяем имя парсера
        std::string parser_name;
        
        // 1. Если указан --name, используем его
        if (options.parser_name != "GeneratedParser") {
            parser_name = options.parser_name;
        }
        // 2. Если указан --output, извлекаем из него
        else if (!options.output_file.empty()) {
            parser_name = options.output_file;
            // Убираем путь
            size_t last_slash = parser_name.find_last_of("/\\");
            if (last_slash != std::string::npos) {
                parser_name = parser_name.substr(last_slash + 1);
            }
            // Убираем расширение
            size_t last_dot = parser_name.find_last_of(".");
            if (last_dot != std::string::npos) {
                parser_name = parser_name.substr(0, last_dot);
            }
        }
        // 3. Иначе извлекаем из имени входного BNF файла
        else {
            parser_name = options.input_file;
            // Убираем путь
            size_t last_slash = parser_name.find_last_of("/\\");
            if (last_slash != std::string::npos) {
                parser_name = parser_name.substr(last_slash + 1);
            }
            // Убираем расширение .bnf
            size_t last_dot = parser_name.find_last_of(".");
            if (last_dot != std::string::npos) {
                parser_name = parser_name.substr(0, last_dot);
            }
            
            // Преобразуем в CamelCase для имени класса: json -> JsonParser
            if (!parser_name.empty()) {
                // Первая буква в верхний регистр
                parser_name[0] = std::toupper(parser_name[0]);
                // Добавляем суффикс Parser
                parser_name += "Parser";
            }
        }
        
        gen_options.parser_name = parser_name;
        
        gen_options.namespace_name = options.namespace_name;
        gen_options.debug_mode = options.debug_mode;
        gen_options.generate_executable = options.generate_executable;
        
        auto result = generator->generate(*grammar, gen_options);
        
        if (!result.success) {
            std::cerr << "Error: Code generation failed: " << result.error_message << "\n";
            return 1;
        }
        
        // Определение выходной директории
        std::string output_dir = options.output_dir;
        if (output_dir == ".") {
            // Извлекаем имя BNF файла без расширения
            std::string bnf_name = options.input_file;
            size_t last_slash = bnf_name.find_last_of("/\\");
            if (last_slash != std::string::npos) {
                bnf_name = bnf_name.substr(last_slash + 1);
            }
            size_t last_dot = bnf_name.find_last_of(".");
            if (last_dot != std::string::npos) {
                bnf_name = bnf_name.substr(0, last_dot);
            }
            
            // Формируем путь: generated/${bnf_name}/${format}/
            std::string target_type = options.format;
            if (target_type == "source-only") {
                target_type = "source";
            }
            
            // Для executable добавляем debug/release
            if (options.format == "executable" || options.generate_executable) {
                std::string build_type = options.debug_mode ? "debug" : "release";
                output_dir = "generated/" + bnf_name + "/exec/" + build_type;
            } else {
                output_dir = "generated/" + bnf_name + "/" + target_type;
            }
        }
        
        // Создаем выходную директорию
        std::string mkdir_cmd = "mkdir -p " + output_dir;
        if (system(mkdir_cmd.c_str()) != 0) {
            std::cerr << "Warning: Failed to create directory: " << output_dir << "\n";
        }
        
        // Определение имени выходного файла
        std::string output_file;
        if (!options.output_file.empty()) {
            output_file = output_dir + "/" + options.output_file;
        } else {
            output_file = output_dir + "/" + result.parser_filename;
        }
        
        // Запись сгенерированного кода
        std::ofstream out(output_file);
        if (!out) {
            std::cerr << "Error: Cannot write to file: " << output_file << "\n";
            return 1;
        }
        
        out << result.parser_code;
        out.close();
        
        if (options.verbose) {
            std::cout << "  ✓ Generated " << result.parser_code.size() << " bytes\n";
            std::cout << "  ✓ Output: " << output_file << "\n";
            
            if (!result.messages.empty()) {
                std::cout << "\n  Messages:\n";
                for (const auto& msg : result.messages) {
                    std::cout << "    " << msg << "\n";
                }
            }
        }
        
        // Запись дополнительных файлов (заголовочные файлы и т.д.)
        for (const auto& [filename, content] : result.additional_files) {
            std::ofstream additional_out(filename);
            if (additional_out) {
                additional_out << content;
                additional_out.close();
                if (options.verbose) {
                    std::cout << "  ✓ Additional file: " << filename << "\n";
                }
            }
        }
        
        // Запись main.cpp если сгенерирован исполняемый файл
        if (!result.main_code.empty()) {
            std::string main_output_file = output_dir + "/" + result.main_filename;
            std::ofstream main_out(main_output_file);
            if (!main_out) {
                std::cerr << "Error: Cannot write to file: " << main_output_file << "\n";
                return 1;
            }
            main_out << result.main_code;
            main_out.close();
            
            if (options.verbose) {
                std::cout << "  ✓ Generated main.cpp: " << main_output_file << "\n";
            }
        }
        
        if (!options.verbose) {
            std::cout << "Generated in " << output_dir << ": " << result.parser_filename;
            if (!result.main_filename.empty()) {
                std::cout << ", " << result.main_filename;
            }
            std::cout << "\n";
        } else {
            std::cout << "\n✅ Success! Files generated in: " << output_dir << "\n";
        }
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}

