#!/bin/bash

# BNF Parser Build Script
# Supports multiple build configurations

set -e  # Stop on error

# Output colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Colored output helpers
print_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Help message
show_help() {
    echo "BNF Parser Build Script"
    echo ""
    echo "Usage: $0 [OPTIONS] [TARGET]"
    echo ""
    echo "OPTIONS:"
    echo "  -h, --help          Show this help"
    echo "  -d, --debug         Debug build (default)"
    echo "  -r, --release       Release build"
    echo "  -c, --clean         Clean output directories"
    echo "  -v, --verbose       Verbose ninja output"
    echo "  --no-tests          Skip building tests"
    echo "  --no-examples       Skip building examples"
    echo "  --shared            Build shared library (default)"
    echo "  --static            Build static library"
    echo ""
    echo "TARGETS:"
    echo "  all                 Build everything (default)"
    echo "  lib                 Library only"
    echo "  tests               Tests only"
    echo "  examples            Examples only"
    echo "  clean               Clean everything"
    echo ""
    echo "EXAMPLES:"
    echo "  $0                  # Debug build of everything"
    echo "  $0 -r               # Release build of everything"
    echo "  $0 -d lib           # Debug build of library only"
    echo "  $0 -r --no-tests    # Release build without tests"
    echo "  $0 clean            # Clean"
}

# Default parameters
BUILD_TYPE="debug"
TARGET="all"
CLEAN=false
VERBOSE=false
BUILD_TESTS=true
BUILD_EXAMPLES=true
LIBRARY_TYPE="shared"
NINJA_ARGS=""

# Parse arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            show_help
            exit 0
            ;;
        -d|--debug)
            BUILD_TYPE="debug"
            shift
            ;;
        -r|--release)
            BUILD_TYPE="release"
            shift
            ;;
        -c|--clean)
            CLEAN=true
            shift
            ;;
        -v|--verbose)
            VERBOSE=true
            NINJA_ARGS="-v"
            shift
            ;;
        --no-tests)
            BUILD_TESTS=false
            shift
            ;;
        --no-examples)
            BUILD_EXAMPLES=false
            shift
            ;;
        --shared)
            LIBRARY_TYPE="shared"
            shift
            ;;
        --static)
            LIBRARY_TYPE="static"
            shift
            ;;
        all|lib|tests|examples|clean)
            TARGET="$1"
            shift
            ;;
        *)
            print_error "Unknown option: $1"
            echo "Use -h for help"
            exit 1
            ;;
    esac
done

# Directories
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
OUT_DIR="$PROJECT_ROOT/out/$BUILD_TYPE/$LIBRARY_TYPE"

print_info "BNF Parser Build Script"
print_info "Project: $PROJECT_ROOT"
print_info "Build type: $BUILD_TYPE"
print_info "Library type: $LIBRARY_TYPE"
print_info "Target: $TARGET"
print_info "Output dir: $OUT_DIR"

# Clean output
clean_build() {
    print_info "Cleaning output directories..."
    rm -rf "$PROJECT_ROOT/out"
    print_success "Clean completed"
}

# Generate build files
generate_build() {
    print_info "Generating build files for $BUILD_TYPE..."
    
    # Create args.gn
    mkdir -p "$OUT_DIR"
    cat > "$OUT_DIR/args.gn" << EOF
# Build arguments for $BUILD_TYPE build
is_debug = $([ "$BUILD_TYPE" = "debug" ] && echo "true" || echo "false")
bnf_parser_generator_enable_unicode = true
bnf_parser_generator_enable_tests = $BUILD_TESTS
bnf_parser_generator_enable_examples = $BUILD_EXAMPLES
bnf_parser_generator_library_type = "$LIBRARY_TYPE"
EOF

    # Generate build files
    cd "$PROJECT_ROOT"
    if ! gn gen "$OUT_DIR"; then
        print_error "Failed to generate build files"
        exit 1
    fi
    
    print_success "Build files generated"
}

# Build target
build_target() {
    local build_target="$1"
    
    print_info "Building target: $build_target"
    
    cd "$PROJECT_ROOT"
    
    # Map GN targets
    case $build_target in
        all)
            if [ "$BUILD_TESTS" = "true" ]; then
                GN_TARGET="all_with_tests"
            else
                GN_TARGET="all"
            fi
            ;;
        lib)
            GN_TARGET="bnf_parser_generator"
            ;;
        tests)
            GN_TARGET="tests"
            ;;
        examples)
            GN_TARGET="examples"
            ;;
        *)
            print_error "Unknown target: $build_target"
            exit 1
            ;;
    esac
    
    # Run ninja
    if ! ninja -C "$OUT_DIR" $NINJA_ARGS "$GN_TARGET"; then
        print_error "Build failed"
        exit 1
    fi
    
    print_success "Build completed successfully"
}

# Run tests
run_tests() {
    if [ "$BUILD_TESTS" = "true" ]; then
        print_info "Running tests..."
        
        # Add library path for shared builds
        export LD_LIBRARY_PATH="$OUT_DIR:$LD_LIBRARY_PATH"
        
        local all_passed=true
        
        # Run all tests
        for test_exe in "$OUT_DIR/basic_test" "$OUT_DIR/utf8_test" "$OUT_DIR/generator_test"; do
            if [ -f "$test_exe" ]; then
                local test_name=$(basename "$test_exe")
                print_info "Running $test_name..."
                if "$test_exe"; then
                    print_success "$test_name passed"
                else
                    print_error "$test_name failed"
                    all_passed=false
                fi
            fi
        done
        
        if [ "$all_passed" = "true" ]; then
            print_success "All tests passed"
        else
            print_error "Some tests failed"
            exit 1
        fi
    fi
}

# Show examples
show_examples() {
    if [ "$BUILD_EXAMPLES" = "true" ]; then
        print_info "Available examples:"
        if [ -f "$OUT_DIR/simple_demo" ]; then
            echo "  simple_demo"
            print_info "Run example: $OUT_DIR/simple_demo"
        fi
    fi
}

# Main logic
main() {
    # Check required tools
    if ! command -v gn &> /dev/null; then
        print_error "gn not found. Please install GN build system."
        exit 1
    fi
    
    if ! command -v ninja &> /dev/null; then
        print_error "ninja not found. Please install Ninja build system."
        exit 1
    fi
    
    # Execute actions
    case $TARGET in
        clean)
            clean_build
            exit 0
            ;;
        *)
            if [ "$CLEAN" = "true" ]; then
                clean_build
            fi
            
            generate_build
            build_target "$TARGET"
            
            if [ "$TARGET" = "all" ] || [ "$TARGET" = "tests" ]; then
                run_tests
            fi
            
            if [ "$TARGET" = "all" ] || [ "$TARGET" = "examples" ]; then
                show_examples
            fi
            ;;
    esac
    
    print_success "Done"
}

# Entry point
main
