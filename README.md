# BNF Parser

Universal parser for languages based on BNF grammars.

## Supported Languages

- **Prolog** - logic programming
- **Clojure (EDN)** - extensible data notation
- **JSON** - data interchange

## Usage

```cpp
#include "bnf_parser.hpp"

auto parser = bnf_parser::Parser::create();
auto grammar = bnf_parser::Grammar::fromFile("grammars/json.bnf");
auto tokenizer = parser->createTokenizer(*grammar);

auto tokens = tokenizer->tokenize(R"({"key": "value"})");
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
git submodule add https://github.com/your-org/bnf_parser.git deps/bnf_parser
```

### GN Build
```gn
# In your BUILD.gn
deps = [ "//deps/bnf_parser:bnf_parser" ]
```

## Features

- **Multi-language support** - Parse Prolog, Clojure, JSON and more
- **Flexible grammar system** - Easy to add new language grammars
- **High performance** - Optimized C++ implementation
- **Cross-platform** - Works on Linux, macOS, Windows
- **Modern C++** - C++17 standard with clean API
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
git clone https://github.com/your-org/bnf_parser.git
cd bnf_parser

# Build and test
./build_dev.sh test

# Run examples
./build_dev.sh examples
```

### Contributing
1. Follow conventional commit format
2. Ensure all tests pass
3. Update documentation if needed
4. Submit pull request

## License

This project is licensed under the MIT License - see the LICENSE file for details.
