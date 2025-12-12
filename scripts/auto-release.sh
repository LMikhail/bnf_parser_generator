#!/bin/bash

# Automated release script for BNF Parser
# Based on conventional commits and automatic versioning

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
    print_info "Checking dependencies..."
    
    if ! command -v git &> /dev/null; then
        print_error "git not found"
        exit 1
    fi
    
    if ! command -v npm &> /dev/null; then
        print_warning "npm not found, will try standard-version via npx"
    fi
    
    print_success "Dependencies OK"
}

# Git status check
check_git_status() {
    print_info "Checking Git status..."
    
    if [ -n "$(git status --porcelain)" ]; then
        print_error "Working directory is not clean. Commit changes first."
        git status --short
        exit 1
    fi
    
    print_success "Git status is clean"
}

# Run tests
run_tests() {
    print_info "Running tests..."
    
    if [ -f "./build.sh" ]; then
        ./build.sh -d
        print_success "Build completed"
    else
        print_warning "build.sh not found, skipping build"
    fi
    
    if [ -f "./build_dev.sh" ]; then
        ./build_dev.sh test
        print_success "Tests passed"
    else
        print_warning "build_dev.sh not found, skipping tests"
    fi
}

# Automatic versioning
auto_version() {
    print_info "Automatic versioning..."
    
    # Check standard-version availability
    if command -v npx &> /dev/null; then
        print_info "Using standard-version for auto versioning..."
        npx standard-version
    else
        print_warning "npx unavailable, fallback to conventional-changelog..."
        
        # Install conventional-changelog if needed
        if ! command -v conventional-changelog &> /dev/null; then
            print_info "Installing conventional-changelog..."
            npm install -g conventional-changelog-cli
        fi
        
        # Generate CHANGELOG
        conventional-changelog -p angular -i CHANGELOG.md -s
        
        # Determine next version (simplified)
        LAST_TAG=$(git describe --tags --abbrev=0 2>/dev/null || echo "v0.0.0")
        print_info "Last tag: $LAST_TAG"
        
        # Analyze commits to choose version bump
        COMMITS_SINCE_TAG=$(git log --oneline $LAST_TAG..HEAD 2>/dev/null || git log --oneline)
        
        if echo "$COMMITS_SINCE_TAG" | grep -q "feat:"; then
            VERSION_TYPE="minor"
        elif echo "$COMMITS_SINCE_TAG" | grep -q "fix:"; then
            VERSION_TYPE="patch"
        elif echo "$COMMITS_SINCE_TAG" | grep -q "BREAKING CHANGE:"; then
            VERSION_TYPE="major"
        else
            print_warning "No version-impacting commits. Skipping release."
            exit 0
        fi
        
        print_info "Version bump: $VERSION_TYPE"
        
        # Commit CHANGELOG
        git add CHANGELOG.md
        git commit -s -m "chore: update CHANGELOG.md"
        
        # Create tag (simplified)
        NEW_VERSION=$(echo $LAST_TAG | sed 's/v//' | awk -F. -v type=$VERSION_TYPE '{
            if (type == "major") { $1++; $2=0; $3=0 }
            else if (type == "minor") { $2++; $3=0 }
            else if (type == "patch") { $3++ }
            print "v" $1 "." $2 "." $3
        }')
        
        git tag $NEW_VERSION
        print_success "Created tag: $NEW_VERSION"
    fi
}

# Push changes
push_changes() {
    print_info "Pushing changes..."
    
    git push origin main
    git push origin main --tags
    
    print_success "Changes pushed"
}

# Entry point
main() {
    print_info "Starting automated release for BNF Parser"
    
    check_dependencies
    check_git_status
    run_tests
    auto_version
    push_changes
    
    print_success "Release completed successfully"
}

# Run
main "$@"
