#!/bin/bash

# BNF Parser Build Script
# Поддерживает различные варианты сборки

set -e  # Остановка при ошибке

# Цвета для вывода
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Функция для вывода с цветом
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

# Функция помощи
show_help() {
    echo "BNF Parser Build Script"
    echo ""
    echo "Использование: $0 [ОПЦИИ] [ЦЕЛЬ]"
    echo ""
    echo "ОПЦИИ:"
    echo "  -h, --help          Показать эту справку"
    echo "  -d, --debug         Debug сборка (по умолчанию)"
    echo "  -r, --release       Release сборка"
    echo "  -c, --clean         Очистить выходные директории"
    echo "  -v, --verbose       Подробный вывод"
    echo "  --no-tests          Не собирать тесты"
    echo "  --no-examples       Не собирать примеры"
    echo ""
    echo "ЦЕЛИ:"
    echo "  all                 Собрать всё (по умолчанию)"
    echo "  lib                 Только библиотека"
    echo "  tests               Только тесты"
    echo "  examples            Только примеры"
    echo "  clean               Очистить всё"
    echo ""
    echo "ПРИМЕРЫ:"
    echo "  $0                  # Debug сборка всего"
    echo "  $0 -r               # Release сборка всего"
    echo "  $0 -d lib           # Debug сборка только библиотеки"
    echo "  $0 -r --no-tests    # Release без тестов"
    echo "  $0 clean            # Очистка"
}

# Параметры по умолчанию
BUILD_TYPE="debug"
TARGET="all"
CLEAN=false
VERBOSE=false
BUILD_TESTS=true
BUILD_EXAMPLES=true
NINJA_ARGS=""

# Парсинг аргументов
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
        all|lib|tests|examples|clean)
            TARGET="$1"
            shift
            ;;
        *)
            print_error "Неизвестная опция: $1"
            echo "Используйте -h для справки"
            exit 1
            ;;
    esac
done

# Определяем директории
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
OUT_DIR="$PROJECT_ROOT/out/$BUILD_TYPE"

print_info "BNF Parser Build Script"
print_info "Проект: $PROJECT_ROOT"
print_info "Тип сборки: $BUILD_TYPE"
print_info "Цель: $TARGET"
print_info "Выходная директория: $OUT_DIR"

# Функция очистки
clean_build() {
    print_info "Очистка выходных директорий..."
    rm -rf "$PROJECT_ROOT/out"
    print_success "Очистка завершена"
}

# Функция генерации build файлов
generate_build() {
    print_info "Генерация build файлов для $BUILD_TYPE..."
    
    # Создаем args.gn файл
    mkdir -p "$OUT_DIR"
    cat > "$OUT_DIR/args.gn" << EOF
# Build arguments for $BUILD_TYPE build
is_debug = $([ "$BUILD_TYPE" = "debug" ] && echo "true" || echo "false")
bnf_parser_enable_unicode = true
bnf_parser_enable_tests = $BUILD_TESTS
bnf_parser_enable_examples = $BUILD_EXAMPLES
EOF

    # Генерируем build файлы
    cd "$PROJECT_ROOT"
    if ! gn gen "$OUT_DIR"; then
        print_error "Ошибка генерации build файлов"
        exit 1
    fi
    
    print_success "Build файлы сгенерированы"
}

# Функция сборки
build_target() {
    local build_target="$1"
    
    print_info "Сборка цели: $build_target"
    
    cd "$PROJECT_ROOT"
    
    # Определяем GN цели
    case $build_target in
        all)
            if [ "$BUILD_TESTS" = "true" ]; then
                GN_TARGET="all_with_tests"
            else
                GN_TARGET="all"
            fi
            ;;
        lib)
            GN_TARGET="bnf_parser"
            ;;
        tests)
            GN_TARGET="tests"
            ;;
        examples)
            GN_TARGET="examples"
            ;;
        *)
            print_error "Неизвестная цель: $build_target"
            exit 1
            ;;
    esac
    
    # Запускаем ninja
    if ! ninja -C "$OUT_DIR" $NINJA_ARGS "$GN_TARGET"; then
        print_error "Ошибка сборки"
        exit 1
    fi
    
    print_success "Сборка завершена успешно"
}

# Функция запуска тестов
run_tests() {
    if [ "$BUILD_TESTS" = "true" ] && [ -f "$OUT_DIR/basic_test" ]; then
        print_info "Запуск тестов..."
        if "$OUT_DIR/basic_test"; then
            print_success "Все тесты прошли"
        else
            print_error "Тесты провалились"
            exit 1
        fi
    fi
}

# Функция показа примеров
show_examples() {
    if [ "$BUILD_EXAMPLES" = "true" ]; then
        print_info "Доступные примеры:"
        for example in "$OUT_DIR"/*_example; do
            if [ -f "$example" ]; then
                echo "  $(basename "$example")"
            fi
        done
        print_info "Запустите пример: $OUT_DIR/prolog_example"
    fi
}

# Основная логика
main() {
    # Проверяем наличие необходимых инструментов
    if ! command -v gn &> /dev/null; then
        print_error "gn не найден. Установите GN build system."
        exit 1
    fi
    
    if ! command -v ninja &> /dev/null; then
        print_error "ninja не найден. Установите Ninja build system."
        exit 1
    fi
    
    # Выполняем действия
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
    
    print_success "Готово! 🎉"
}

# Запуск
main
