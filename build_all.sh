#!/bin/bash

# Скрипт для сборки всех четырех вариантов библиотек
# out/debug/shared, out/debug/static, out/release/shared, out/release/static

set -e

# Цвета для вывода
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

print_info "Сборка всех вариантов BNF Parser библиотек..."

# Массив вариантов для сборки
# Формат: "build_type/library_type"
variants=(
    "debug/shared"
    "debug/static" 
    "release/shared"
    "release/static"
)

# Функция для сборки одного варианта
build_variant() {
    local variant=$1
    local build_type=${variant%/*}
    local library_type=${variant#*/}
    
    print_info "Сборка варианта: $variant (build: $build_type, library: $library_type)"
    
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
        print_success "✅ $variant собран успешно"
    else
        print_error "Ошибка сборки $variant"
        return 1
    fi
}

# Собираем все варианты
failed_variants=()
for variant in "${variants[@]}"; do
    if ! build_variant "$variant"; then
        failed_variants+=("$variant")
    fi
done

# Итоговый отчет
echo ""
print_info "=== ИТОГОВЫЙ ОТЧЕТ ==="

if [ ${#failed_variants[@]} -eq 0 ]; then
    print_success "Все варианты собраны успешно"
    echo ""
    print_info "Созданные библиотеки:"
    find out/ -name "*.so" -o -name "*.a" | sort | while read lib; do
        echo "  $lib"
    done
else
    print_error "Ошибки при сборке следующих вариантов:"
    for variant in "${failed_variants[@]}"; do
        echo "  - $variant"
    done
    exit 1
fi

echo ""
print_info "Структура директорий:"
ls -la out/ | grep "^d" | awk '{print "  📁 " $9}' | grep -v "^📁 \.$" | grep -v "^📁 \.\.$"
