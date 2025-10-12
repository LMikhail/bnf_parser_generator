# BNF Parser Generator

Generator of standalone parsers from BNF/EBNF grammars. Produces C++ code that can be used as library or executable.

## Status

Working implementation. Generator supports full EBNF syntax and produces functional parsers.

Currently implemented:
- C++ code generation (header + implementation)
- Full EBNF support (alternatives, sequences, repetitions, optionals, groups, character ranges)
- Full Unicode support (UTF-8, codepoint ranges U+0000 to U+10FFFF)
- Standalone executable generation with CLI
- Library generation (static/shared)
- Recursive descent parsing with backtracking
- Automatic parser naming from grammar files

## Usage

### Generate Parser

```bash
# Generate parser source code (auto-names from grammar file)
./out/debug/shared/bnf-parser-gen -i grammars/json.bnf

# Generate with standalone executable
./out/debug/shared/bnf-parser-gen -i grammars/json.bnf --executable

# Specify custom output directory
./out/debug/shared/bnf-parser-gen -i grammars/json.bnf --output-dir my_parsers/json

# Use custom parser name
./out/debug/shared/bnf-parser-gen -i grammars/json.bnf --name MyJsonParser

# Generate with verbose output
./out/debug/shared/bnf-parser-gen -i grammars/json.bnf --executable --verbose
```

Output structure:
- Source only: `generated/<grammar_name>/exec/release/<parser_name>.cpp`
- With executable: `generated/<grammar_name>/exec/release/<parser_name>.cpp` and `<parser_name>_main.cpp`

### Compile Generated Parser

```bash
# Example: compile JSON parser as executable
cd generated/json/exec/release
g++ -std=c++17 -o json_parser json_parser_main.cpp json_parser.cpp

# Compile as shared library
g++ -std=c++17 -shared -fPIC -o libjson_parser.so json_parser.cpp

# Compile as static library
g++ -std=c++17 -c json_parser.cpp -o json_parser.o
ar rcs libjson_parser.a json_parser.o
```

### Run Generated Parser

```bash
# Parse a file with generated parser
./json_parser examples/test.json

# Show AST structure
./json_parser examples/test.json --ast

# Verbose parsing output
./json_parser examples/test.json --verbose

# Show help
./json_parser --help
```

### Use Generated Parser

```cpp
#include "MyParser.cpp"

int main() {
    std::string input = "...";
    MyParser parser(input);
    auto ast = parser.parse();
    
    if (ast) {
        // Process AST
    } else {
        std::cerr << parser.getError() << std::endl;
    }
}
```

## Example Grammars

The project includes complete reference grammars in `grammars/` directory:
- `json.bnf` - Full JSON specification with Unicode support
- `prolog.bnf` - Prolog logic programming language (facts, rules, queries)
- `clojure.bnf` - Clojure/EDN data notation with all data structures

Test files with comprehensive coverage are provided in `examples/`:
- `examples/test.json` - JSON with Unicode, escapes, nested structures, all number formats
- `examples/test.pl` - Prolog with Unicode comments, facts, rules, lists, terms
- `examples/test.clj` - Clojure with all data types, functions, macros, reader literals

All grammars use pure EBNF with full Unicode support (U+0000 to U+10FFFF) and are ready for parsing real-world code.

## Building

### Quick Build (Recommended)
```bash
# Quick commands
./build_dev.sh test      # Build and tests
./build_dev.sh debug     # Debug build
./build_dev.sh release   # Release build
./build_dev.sh examples  # Examples
./build_dev.sh clean     # Clean

# Full build script
./build.sh           # Debug build everything
./build.sh -r        # Release build
./build.sh -d lib    # Library only
./build.sh --help    # Help
```

### Manual Build (GN)
```bash
gn gen out/release --args="is_debug=false"
ninja -C out/release
```

## Integration

### Git Submodule
```bash
git submodule add https://github.com/your-org/bnf_parser_generator.git deps/bnf_parser_generator
```

### GN Build
```gn
# In your BUILD.gn
deps = [ "//deps/bnf_parser_generator:bnf_parser_generator" ]
```

## Features

- Generates standalone parsers from BNF/EBNF grammars
- C++ code generation (C++17 standard)
- Full EBNF syntax support (alternatives, sequences, repetitions, optionals, groups, character ranges)
- Complete Unicode support (UTF-8, full range U+0000 to U+10FFFF)
- Generated parsers have no runtime dependencies
- Recursive descent parsing with backtracking
- Multiple output formats: source code, static library, shared library, executable
- Automatic parser naming from grammar files (e.g., `json.bnf` → `json_parser.cpp`)
- CLI tool with command-line argument parsing (--ast, --verbose, --help)
- Cross-platform: Linux, macOS, Windows
- Full test suite included
- CI/CD configured (GitHub Actions, GitLab CI)
- Semantic versioning with conventional commits

## Development

### Prerequisites
- C++17 compatible compiler (GCC, Clang, MSVC)
- GN build system
- Ninja build tool

### Quick Start
```bash
# Clone repository
git clone https://github.com/your-org/bnf_parser_generator.git
cd bnf_parser_generator

# Build and test
./build_dev.sh test

# Run examples
./build_dev.sh examples
```

### Documentation

[Russian Documentation](docs/ru/README.md) - Full documentation in Russian
- [Architecture](docs/ru/architecture.md) - System architecture
- [Design Decisions](docs/ru/design_decisions.md) - Architectural decisions history
- [Parsing Algorithms Analysis](docs/ru/parsing_algorithms/parsing_algorithms_analysis.md) - Comparison of 16 parsing algorithms
- [Grammar Classes](docs/ru/parsing_algorithms/grammar_classes.md) - Chomsky hierarchy reference

### Contributing
1. Follow conventional commit format
2. Ensure all tests pass
3. Update documentation if needed
4. Submit pull request

## License

This project is licensed under the Apache 2.0 License - see the LICENSE file for details.
