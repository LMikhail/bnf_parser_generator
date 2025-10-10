#!/bin/bash

# Автоматический release скрипт для BNF Parser
# Основан на conventional commits и автоматическом версионировании

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

# Проверка наличия необходимых инструментов
check_dependencies() {
    print_info "Проверка зависимостей..."
    
    if ! command -v git &> /dev/null; then
        print_error "git не найден"
        exit 1
    fi
    
    if ! command -v npm &> /dev/null; then
        print_warning "npm не найден, попытка установки standard-version через npx"
    fi
    
    print_success "Зависимости проверены"
}

# Проверка статуса git
check_git_status() {
    print_info "Проверка статуса Git..."
    
    if [ -n "$(git status --porcelain)" ]; then
        print_error "Рабочая директория не чистая. Сначала закоммитьте изменения."
        git status --short
        exit 1
    fi
    
    print_success "Git статус чистый"
}

# Запуск тестов
run_tests() {
    print_info "Запуск тестов..."
    
    if [ -f "./build.sh" ]; then
        ./build.sh -d
        print_success "Сборка завершена"
    else
        print_warning "build.sh не найден, пропускаем сборку"
    fi
    
    if [ -f "./build_dev.sh" ]; then
        ./build_dev.sh test
        print_success "Тесты пройдены"
    else
        print_warning "build_dev.sh не найден, пропускаем тесты"
    fi
}

# Автоматическое версионирование
auto_version() {
    print_info "Автоматическое версионирование..."
    
    # Проверяем наличие standard-version
    if command -v npx &> /dev/null; then
        print_info "Используем standard-version для автоматического версионирования..."
        npx standard-version
    else
        print_warning "npx недоступен, используем conventional-changelog..."
        
        # Установка conventional-changelog если нужно
        if ! command -v conventional-changelog &> /dev/null; then
            print_info "Установка conventional-changelog..."
            npm install -g conventional-changelog-cli
        fi
        
        # Генерация CHANGELOG
        conventional-changelog -p angular -i CHANGELOG.md -s
        
        # Определение новой версии (упрощенная логика)
        LAST_TAG=$(git describe --tags --abbrev=0 2>/dev/null || echo "v0.0.0")
        print_info "Последний тег: $LAST_TAG"
        
        # Анализ коммитов для определения типа версии
        COMMITS_SINCE_TAG=$(git log --oneline $LAST_TAG..HEAD 2>/dev/null || git log --oneline)
        
        if echo "$COMMITS_SINCE_TAG" | grep -q "feat:"; then
            VERSION_TYPE="minor"
        elif echo "$COMMITS_SINCE_TAG" | grep -q "fix:"; then
            VERSION_TYPE="patch"
        elif echo "$COMMITS_SINCE_TAG" | grep -q "BREAKING CHANGE:"; then
            VERSION_TYPE="major"
        else
            print_warning "Нет коммитов, влияющих на версию. Пропускаем release."
            exit 0
        fi
        
        print_info "Тип версии: $VERSION_TYPE"
        
        # Создание коммита с CHANGELOG
        git add CHANGELOG.md
        git commit -s -m "chore: update CHANGELOG.md"
        
        # Создание тега (упрощенная версия)
        NEW_VERSION=$(echo $LAST_TAG | sed 's/v//' | awk -F. -v type=$VERSION_TYPE '{
            if (type == "major") { $1++; $2=0; $3=0 }
            else if (type == "minor") { $2++; $3=0 }
            else if (type == "patch") { $3++ }
            print "v" $1 "." $2 "." $3
        }')
        
        git tag $NEW_VERSION
        print_success "Создан тег: $NEW_VERSION"
    fi
}

# Отправка изменений
push_changes() {
    print_info "Отправка изменений..."
    
    git push origin main
    git push origin main --tags
    
    print_success "Изменения отправлены"
}

# Основная функция
main() {
    print_info "Запуск автоматического release для BNF Parser"
    
    check_dependencies
    check_git_status
    run_tests
    auto_version
    push_changes
    
    print_success "Release завершен успешно"
}

# Запуск
main "$@"
