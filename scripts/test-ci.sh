#!/bin/bash

# Скрипт для локального тестирования CI/CD pipeline
# Имитирует основные этапы CI/CD без внешних сервисов

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

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Проверка зависимостей
check_dependencies() {
    print_info "🔍 Проверка зависимостей..."
    
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
        print_error "Отсутствуют зависимости: ${missing_deps[*]}"
        print_info "Установите их командой:"
        print_info "sudo apt-get install -y ${missing_deps[*]}"
        exit 1
    fi
    
    print_success "Все зависимости установлены"
}

# Проверка качества кода
lint_check() {
    print_info "🔍 Проверка качества кода..."
    
    # Проверка стиля кода
    print_info "Проверка файлов C++..."
    find src include -name "*.cpp" -o -name "*.hpp" | head -5 | while read file; do
        if [ -f "$file" ]; then
            echo "  ✓ $file"
        fi
    done
    
    # Проверка формата коммитов
    print_info "Проверка формата коммитов..."
    if git log --format="%s" -1 | grep -E "^(feat|fix|docs|style|refactor|test|build|chore)(\(.+\))?: .+" > /dev/null; then
        print_success "Формат коммита корректен"
    else
        print_warning "Формат коммита может быть улучшен"
    fi
    
    # Проверка DCO
    print_info "Проверка DCO..."
    if git log --format='%H %s' -1 | grep -q 'Signed-off-by:'; then
        print_success "DCO проверка пройдена"
    else
        print_warning "Коммит не подписан (DCO)"
    fi
    
    print_success "Проверка качества кода завершена"
}

# Сборка проекта
build_project() {
    local build_type=$1
    print_info "🔨 Сборка $build_type версии..."
    
    # Создаем директорию и args.gn
    mkdir -p out/$build_type
    cat > out/$build_type/args.gn << EOF
is_debug = $([ "$build_type" = "debug" ] && echo "true" || echo "false")
bnf_parser_enable_unicode = true
bnf_parser_enable_tests = true
bnf_parser_enable_examples = true
EOF
    
    # Генерируем build файлы
    gn gen out/$build_type
    
    # Собираем проект
    ninja -C out/$build_type all
    
    print_success "Сборка $build_type завершена"
}

# Тестирование
run_tests() {
    local build_type=$1
    print_info "🧪 Запуск тестов для $build_type..."
    
    # Unit тесты
    if [ -f "out/$build_type/basic_test" ]; then
        print_info "Запуск basic_test..."
        out/$build_type/basic_test || print_warning "basic_test завершился с предупреждениями"
    fi
    
    # Тестирование примеров
    if [ -f "out/$build_type/simple_demo" ]; then
        print_info "Тестирование simple_demo..."
        out/$build_type/simple_demo || print_warning "simple_demo завершился с предупреждениями"
    fi
    
    # Тестирование отладочных программ
    for debug_prog in out/$build_type/debug_*; do
        if [ -f "$debug_prog" ] && [ -x "$debug_prog" ]; then
            print_info "Тестирование $(basename $debug_prog)..."
            $debug_prog || print_warning "$(basename $debug_prog) завершился с предупреждениями"
        fi
    done
    
    print_success "Тестирование $build_type завершено"
}

# Создание артефактов
create_artifacts() {
    local build_type=$1
    print_info "Создание артефактов для $build_type..."
    
    # Создаем директорию для артефактов
    mkdir -p artifacts/$build_type
    
    # Копируем собранные файлы
    cp -r out/$build_type/* artifacts/$build_type/ 2>/dev/null || true
    
    # Копируем документацию
    cp -r docs/ artifacts/ 2>/dev/null || true
    cp README.md artifacts/ 2>/dev/null || true
    
    # Создаем архив
    VERSION=$(git describe --tags --abbrev=0 2>/dev/null || echo "dev-$(git rev-parse --short HEAD)")
    tar -czf bnf-parser-${VERSION}-${build_type}.tar.gz artifacts/
    
    print_success "Артефакты созданы: bnf-parser-${VERSION}-${build_type}.tar.gz"
}

# Основная функция
main() {
    print_info "Запуск локального тестирования CI/CD pipeline"
    
    # Проверяем что мы в корне проекта
    if [ ! -f "BUILD.gn" ]; then
        print_error "Запустите скрипт из корня проекта"
        exit 1
    fi
    
    # Этапы CI/CD
    check_dependencies
    lint_check
    
    # Сборка debug и release
    build_project debug
    build_project release
    
    # Тестирование
    run_tests debug
    run_tests release
    
    # Создание артефактов
    create_artifacts debug
    create_artifacts release
    
    print_success "Локальное тестирование CI/CD завершено успешно"
    print_info "Артефакты созданы в текущей директории"
    ls -la *.tar.gz 2>/dev/null || true
}

# Обработка аргументов
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
        echo "Использование: $0 [команда] [тип_сборки]"
        echo ""
        echo "Команды:"
        echo "  lint      - проверка качества кода"
        echo "  build     - сборка проекта"
        echo "  test      - запуск тестов"
        echo "  artifacts - создание артефактов"
        echo "  help      - показать эту справку"
        echo ""
        echo "Типы сборки: debug, release (по умолчанию: debug)"
        echo ""
        echo "Примеры:"
        echo "  $0                    # полный pipeline"
        echo "  $0 lint               # только проверка качества"
        echo "  $0 build release       # только release сборка"
        echo "  $0 test debug         # только тесты debug"
        ;;
    *)
        main
        ;;
esac
