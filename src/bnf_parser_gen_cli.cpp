#include "bnf_parser.hpp"
#include "code_generator.hpp"
#include "version.hpp"
#include <iostream>
#include <fstream>
#include <cstring>

using namespace bnf_parser_generator;

struct CliOptions {
    std::string input_file;
    std::string output_file;
    std::string language = "cpp";
    std::string parser_name = "GeneratedParser";
    std::string namespace_name;
    bool verbose = false;
    bool debug_mode = false;
    bool show_help = false;
    bool show_version = false;
};

void printHelp(const char* program_name) {
    std::cout << "BNF Parser Generator - Generate standalone parsers from BNF/EBNF grammars\n";
    std::cout << "\nUsage: " << program_name << " [options]\n";
    std::cout << "\nOptions:\n";
    std::cout << "  -i, --input FILE       Input BNF/EBNF grammar file (required)\n";
    std::cout << "  -o, --output FILE      Output parser file (default: auto-generated)\n";
    std::cout << "  -l, --language LANG    Target language: cpp, dart, java, clojure (default: cpp)\n";
    std::cout << "  -n, --name NAME        Parser class name (default: GeneratedParser)\n";
    std::cout << "  --namespace NAME       Namespace/package name (optional)\n";
    std::cout << "  -v, --verbose          Verbose output\n";
    std::cout << "  -d, --debug            Generate debug code\n";
    std::cout << "  -h, --help             Show this help message\n";
    std::cout << "  --version              Show version information\n";
    std::cout << "\nExamples:\n";
    std::cout << "  " << program_name << " -i json.bnf -o json_parser.cpp\n";
    std::cout << "  " << program_name << " --input grammar.bnf --language cpp --name MyParser\n";
    std::cout << "  " << program_name << " -i calc.bnf -o calculator.cpp --namespace calc --verbose\n";
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
        } else if ((arg == "-l" || arg == "--language") && i + 1 < argc) {
            options.language = argv[++i];
        } else if ((arg == "-n" || arg == "--name") && i + 1 < argc) {
            options.parser_name = argv[++i];
        } else if (arg == "--namespace" && i + 1 < argc) {
            options.namespace_name = argv[++i];
        } else if (arg == "-v" || arg == "--verbose") {
            options.verbose = true;
        } else if (arg == "-d" || arg == "--debug") {
            options.debug_mode = true;
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
        gen_options.parser_name = options.parser_name;
        gen_options.namespace_name = options.namespace_name;
        gen_options.debug_mode = options.debug_mode;
        
        auto result = generator->generate(*grammar, gen_options);
        
        if (!result.success) {
            std::cerr << "Error: Code generation failed: " << result.error_message << "\n";
            return 1;
        }
        
        // Определение имени выходного файла
        std::string output_file = options.output_file;
        if (output_file.empty()) {
            output_file = result.parser_filename;
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
        
        if (!options.verbose) {
            std::cout << "Generated: " << output_file << "\n";
        } else {
            std::cout << "\n✅ Success!\n";
        }
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}

