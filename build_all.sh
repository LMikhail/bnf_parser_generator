#!/bin/bash

# Builds all four library variants:
# out/debug/shared, out/debug/static, out/release/shared, out/release/static

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

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_info "Building all BNF Parser library variants..."

# Variants list, format: "build_type/library_type"
variants=(
    "debug/shared"
    "debug/static" 
    "release/shared"
    "release/static"
)

# Build a single variant
build_variant() {
    local variant=$1
    local build_type=${variant%/*}
    local library_type=${variant#*/}
    
    print_info "Building variant: $variant (build: $build_type, library: $library_type)"
    
    if [ "$build_type" = "debug" ]; then
        BUILD_CMD="./build.sh -d"
    else
        BUILD_CMD="./build.sh -r"
    fi
    
    if [ "$library_type" = "static" ]; then
        BUILD_CMD="$BUILD_CMD --static"
    else
        BUILD_CMD="$BUILD_CMD --shared"
    fi
    
    if $BUILD_CMD; then
        print_success "✅ $variant built successfully"
    else
        print_error "Build failed for $variant"
        return 1
    fi
}

# Build all variants
failed_variants=()
for variant in "${variants[@]}"; do
    if ! build_variant "$variant"; then
        failed_variants+=("$variant")
    fi
done

# Final report
echo ""
print_info "=== SUMMARY REPORT ==="

if [ ${#failed_variants[@]} -eq 0 ]; then
    print_success "All variants built successfully"
    echo ""
    print_info "Produced libraries:"
    find out/ -name "*.so" -o -name "*.a" | sort | while read lib; do
        echo "  $lib"
    done
else
    print_error "Build errors for the following variants:"
    for variant in "${failed_variants[@]}"; do
        echo "  - $variant"
    done
    exit 1
fi

echo ""
print_info "Directory structure:"
ls -la out/ | grep "^d" | awk '{print "  📁 " $9}' | grep -v "^📁 \.$" | grep -v "^📁 \.\.$"
