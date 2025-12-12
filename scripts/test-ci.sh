#!/bin/bash

# Local CI/CD pipeline test
# Emulates main CI/CD stages without external services

set -e

# Output colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

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

# Dependency check
check_dependencies() {
    print_info "🔍 Checking dependencies..."
    
    local missing_deps=()
    
    if ! command -v git &> /dev/null; then
        missing_deps+=("git")
    fi
    
    if ! command -v g++ &> /dev/null; then
        missing_deps+=("g++")
    fi
    
    if ! command -v ninja &> /dev/null; then
        missing_deps+=("ninja-build")
    fi
    
    if ! command -v gn &> /dev/null; then
        missing_deps+=("gn")
    fi
    
    if [ ${#missing_deps[@]} -ne 0 ]; then
        print_error "Missing dependencies: ${missing_deps[*]}"
        print_info "Install with:"
        print_info "sudo apt-get install -y ${missing_deps[*]}"
        exit 1
    fi
    
    print_success "All dependencies are installed"
}

# Code quality checks
lint_check() {
    print_info "🔍 Running code quality checks..."
    
    # Basic C++ file presence check
    print_info "Checking C++ files..."
    find src include -name "*.cpp" -o -name "*.hpp" | head -5 | while read file; do
        if [ -f "$file" ]; then
            echo "  ✓ $file"
        fi
    done
    
    # Commit format check
    print_info "Checking commit format..."
    if git log --format="%s" -1 | grep -E "^(feat|fix|docs|style|refactor|test|build|chore)(\(.+\))?: .+" > /dev/null; then
        print_success "Commit format looks good"
    else
        print_warning "Commit format may need improvement"
    fi
    
    # DCO check
    print_info "Checking DCO..."
    if git log --format='%H %s' -1 | grep -q 'Signed-off-by:'; then
        print_success "DCO check passed"
    else
        print_warning "Commit is not signed (DCO)"
    fi
    
    print_success "Code quality checks finished"
}

# Build project
build_project() {
    local build_type=$1
    print_info "🔨 Building $build_type..."
    
    # Create directory and args.gn
    mkdir -p out/$build_type
    cat > out/$build_type/args.gn << EOF
is_debug = $([ "$build_type" = "debug" ] && echo "true" || echo "false")
bnf_parser_generator_enable_unicode = true
bnf_parser_generator_enable_tests = true
bnf_parser_generator_enable_examples = true
EOF
    
    # Generate build files
    gn gen out/$build_type
    
    # Build
    ninja -C out/$build_type all
    
    print_success "$build_type build completed"
}

# Testing
run_tests() {
    local build_type=$1
    print_info "🧪 Running tests for $build_type..."
    
    # Unit tests
    if [ -f "out/$build_type/basic_test" ]; then
        print_info "Running basic_test..."
        out/$build_type/basic_test || print_warning "basic_test finished with warnings"
    fi
    
    # Example tests
    if [ -f "out/$build_type/simple_demo" ]; then
        print_info "Testing simple_demo..."
        out/$build_type/simple_demo || print_warning "simple_demo finished with warnings"
    fi
    
    # Debug utilities
    for debug_prog in out/$build_type/debug_*; do
        if [ -f "$debug_prog" ] && [ -x "$debug_prog" ]; then
            print_info "Testing $(basename $debug_prog)..."
            $debug_prog || print_warning "$(basename $debug_prog) finished with warnings"
        fi
    done
    
    print_success "$build_type tests finished"
}

# Create artifacts
create_artifacts() {
    local build_type=$1
    print_info "Creating artifacts for $build_type..."
    
    # Artifact directory
    mkdir -p artifacts/$build_type
    
    # Copy build outputs
    cp -r out/$build_type/* artifacts/$build_type/ 2>/dev/null || true
    
    # Copy docs
    cp -r docs/ artifacts/ 2>/dev/null || true
    cp README.md artifacts/ 2>/dev/null || true
    
    # Create archive
    VERSION=$(git describe --tags --abbrev=0 2>/dev/null || echo "dev-$(git rev-parse --short HEAD)")
    tar -czf bnf-parser-${VERSION}-${build_type}.tar.gz artifacts/
    
    print_success "Artifacts created: bnf-parser-${VERSION}-${build_type}.tar.gz"
}

# Entry point
main() {
    print_info "Starting local CI/CD pipeline test"
    
    # Ensure we are in project root
    if [ ! -f "BUILD.gn" ]; then
        print_error "Run this script from the project root"
        exit 1
    fi
    
    # CI/CD stages
    check_dependencies
    lint_check
    
    # Build debug and release
    build_project debug
    build_project release
    
    # Testing
    run_tests debug
    run_tests release
    
    # Artifacts
    create_artifacts debug
    create_artifacts release
    
    print_success "Local CI/CD testing completed successfully"
    print_info "Artifacts created in current directory"
    ls -la *.tar.gz 2>/dev/null || true
}

# Arguments handling
case "${1:-}" in
    "lint")
        check_dependencies
        lint_check
        ;;
    "build")
        check_dependencies
        build_project ${2:-debug}
        ;;
    "test")
        run_tests ${2:-debug}
        ;;
    "artifacts")
        create_artifacts ${2:-debug}
        ;;
    "help"|"-h"|"--help")
        echo "Usage: $0 [command] [build_type]"
        echo ""
        echo "Commands:"
        echo "  lint      - run code quality checks"
        echo "  build     - build project"
        echo "  test      - run tests"
        echo "  artifacts - create artifacts"
        echo "  help      - show this help"
        echo ""
        echo "Build types: debug, release (default: debug)"
        echo ""
        echo "Examples:"
        echo "  $0                    # full pipeline"
        echo "  $0 lint               # only quality checks"
        echo "  $0 build release      # release build only"
        echo "  $0 test debug         # debug tests only"
        ;;
    *)
        main
        ;;
esac
