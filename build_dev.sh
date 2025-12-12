#!/bin/bash

# Developer utilities for BNF Parser

set -e

# Colors
GREEN='\033[0;32m'
BLUE='\033[0;34m'
NC='\033[0m'

print_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

case "$1" in
    "test"|"t")
        print_info "Quick build and test (debug)..."
        "$PROJECT_ROOT/build.sh" -d tests
        ;;
    "release"|"r")
        print_info "Release build..."
        "$PROJECT_ROOT/build.sh" -r
        ;;
    "debug"|"d")
        print_info "Debug build..."
        "$PROJECT_ROOT/build.sh" -d
        ;;
    "clean"|"c")
        print_info "Clean..."
        "$PROJECT_ROOT/build.sh" clean
        ;;
    "examples"|"e")
        print_info "Build and run examples..."
        "$PROJECT_ROOT/build.sh" -d examples
        echo ""
        EXE="$PROJECT_ROOT/out/debug/shared/simple_demo"
        if [ -x "$EXE" ]; then
            print_info "Running simple_demo example:"
            "$EXE"
        else
            print_info "Example binary not found (skipping run): $EXE"
        fi
        ;;
    "help"|"h"|"")
        echo "BNF Parser developer tools:"
        echo ""
        echo "  ./build_dev.sh test      (t) - Quick build and test"
        echo "  ./build_dev.sh debug     (d) - Debug build"
        echo "  ./build_dev.sh release   (r) - Release build"
        echo "  ./build_dev.sh examples  (e) - Examples"
        echo "  ./build_dev.sh clean     (c) - Clean"
        echo "  ./build_dev.sh help      (h) - This help"
        ;;
    *)
        echo "Unknown command: $1"
        echo "Use './build_dev.sh help' for usage"
        exit 1
        ;;
esac
