#!/usr/bin/env python3
"""
Вспомогательный скрипт для запуска shell скриптов из GN build системы.
GN ожидает Python скрипты, поэтому этот скрипт служит оберткой для bash скриптов.
"""

import subprocess
import sys
import os

def main():
    if len(sys.argv) < 2:
        print("Usage: run_shell_script.py <shell_script> [args...]", file=sys.stderr)
        return 1
    
    shell_script = sys.argv[1]
    script_args = sys.argv[2:] if len(sys.argv) > 2 else []
    
    # Проверяем что скрипт существует
    if not os.path.exists(shell_script):
        print(f"Error: Shell script not found: {shell_script}", file=sys.stderr)
        return 1
    
    # Делаем скрипт исполняемым (если еще не)
    os.chmod(shell_script, 0o755)
    
    # Запускаем shell скрипт
    try:
        result = subprocess.run(
            ['/bin/bash', shell_script] + script_args,
            check=True,
            capture_output=True,
            text=True
        )
        
        # Выводим результат
        if result.stdout:
            print(result.stdout, end='')
        
        return 0
        
    except subprocess.CalledProcessError as e:
        print(f"Error running shell script: {shell_script}", file=sys.stderr)
        if e.stdout:
            print(e.stdout, file=sys.stderr)
        if e.stderr:
            print(e.stderr, file=sys.stderr)
        return e.returncode

if __name__ == '__main__':
    sys.exit(main())

