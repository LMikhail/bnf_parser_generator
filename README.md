# BNF Parser Generator

Generator of standalone parsers from BNF/EBNF grammars for various programming languages.

## Project Status

⚠️ **Architecture design phase** - The project is currently in architectural design phase. The implementation of the parser generator is planned.

## Target Languages

**Priority 1** (mandatory):
- **Clojure/EDN** - extensible data notation
- **Prolog** - logic programming

**Priority 2** (important):
- **JSON** - data interchange
- **SQL** - database queries

**Priority 3-6** (promising):
- Python, YAML, C++, Java, JavaScript, TypeScript, Rust, HTML, CSS, XML, UML

## Planned Usage

```bash
# Generate parser from BNF grammar
$ bnf-parser-gen --input json.bnf --output json_parser.cpp --language cpp
Generated: json_parser.cpp (789 lines)
```

```cpp
// User integrates generated parser
#include "json_parser.cpp"

int main() {
    std::string input = R"({"key": "value", "number": 42})";
    json::Parser parser(input);
    auto result = parser.parse();
    
    if (result.success) {
        result.tree->print();
    }
}
```

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

- **Parser Generator** - Generates standalone parsers from BNF/EBNF grammars
- **Multi-language output** - C++, Dart, Java, Clojure (planned)
- **Extended BNF** - Support for context-sensitive grammars via parameters
- **No runtime dependencies** - Generated parsers are standalone
- **High performance** - Native code with natural backtracking
- **Cross-platform** - Works on Linux, macOS, Windows
- **Modern C++17** - Clean architecture and API
- **Comprehensive testing** - Full test suite included
- **CI/CD ready** - GitHub Actions and GitLab CI configured
- **Automatic versioning** - Semantic versioning with conventional commits

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

📘 **[Russian Documentation](docs/ru/README.md)** - Полная документация на русском
- [Architecture](docs/ru/architecture.md) - Architectural design
- [Design Decisions](docs/ru/design_decisions.md) - History of architectural decisions
- [Parsing Algorithms Analysis](docs/ru/parsing_algorithms/parsing_algorithms_analysis.md) - 16 algorithms comparison
- [Grammar Classes](docs/ru/parsing_algorithms/grammar_classes.md) - Chomsky hierarchy

### Contributing
1. Follow conventional commit format
2. Ensure all tests pass
3. Update documentation if needed
4. Submit pull request

## License

This project is licensed under the MIT License - see the LICENSE file for details.
