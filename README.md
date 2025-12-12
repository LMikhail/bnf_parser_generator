# BNF Parser Generator

Generator of standalone parsers from BNF/EBNF grammars. Produces C++ code that can be used as library or executable.

## Status

Working implementation. Generator supports full EBNF syntax and produces functional parsers.

Currently implemented:
- C++ code generation (header + implementation)
- Full EBNF support (alternatives, sequences, repetitions, optionals, groups, character ranges)
- Standalone executable generation with CLI
- Library generation (static/shared)
- Recursive descent parsing with backtracking

## Usage

### Generate Parser

```bash
# Generate parser source code
bnf-parser-gen -i grammar.bnf -o MyParser.cpp

# Generate with standalone executable
bnf-parser-gen -i grammar.bnf -o MyParser.cpp --executable

# Generate with custom namespace
bnf-parser-gen -i grammar.bnf -o MyParser.cpp --namespace myparser
```

### Compile Generated Parser

```bash
# Compile as executable
g++ -std=c++17 -o my_parser MyParser_main.cpp MyParser.cpp

# Compile as shared library
g++ -std=c++17 -shared -fPIC -o libMyParser.so MyParser.cpp

# Compile as static library
g++ -std=c++17 -c MyParser.cpp -o MyParser.o
ar rcs libMyParser.a MyParser.o
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

The project includes reference grammars in `grammars/` directory:
- `json.bnf` - JSON data format
- `prolog.bnf` - Prolog logic programming language
- `clojure.bnf` - Clojure/EDN data notation

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
- Generated parsers have no runtime dependencies
- Recursive descent parsing with backtracking
- Multiple output formats: source code, static library, shared library, executable
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

This project is licensed under the MIT License - see the LICENSE file for details.
