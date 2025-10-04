#!/bin/bash

# Быстрые команды для BNF Parser

set -e

# Цвета
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
        print_info "Быстрая сборка и тест (debug)..."
        "$PROJECT_ROOT/build.sh" -d tests
        ;;
    "release"|"r")
        print_info "Release сборка..."
        "$PROJECT_ROOT/build.sh" -r
        ;;
    "debug"|"d")
        print_info "Debug сборка..."
        "$PROJECT_ROOT/build.sh" -d
        ;;
    "clean"|"c")
        print_info "Очистка..."
        "$PROJECT_ROOT/build.sh" clean
        ;;
    "examples"|"e")
        print_info "Сборка и запуск примеров..."
        "$PROJECT_ROOT/build.sh" -d examples
        echo ""
        print_info "Запуск Prolog примера:"
        "$PROJECT_ROOT/out/debug/prolog_example"
        ;;
    "help"|"h"|"")
        echo "Быстрые команды BNF Parser:"
        echo ""
        echo "  ./quick.sh test      (t) - Быстрая сборка и тест"
        echo "  ./quick.sh debug     (d) - Debug сборка"
        echo "  ./quick.sh release   (r) - Release сборка"
        echo "  ./quick.sh examples  (e) - Примеры"
        echo "  ./quick.sh clean     (c) - Очистка"
        echo "  ./quick.sh help      (h) - Эта справка"
        ;;
    *)
        echo "Неизвестная команда: $1"
        echo "Используйте './quick.sh help' для справки"
        exit 1
        ;;
esac
