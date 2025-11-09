#!/bin/bash
set -e
if [ $# -lt 2 ]; then
    echo "Использование: $0 <количество_игроков> <юзермод(y/n)>"
    echo "Пример: $0 10 y"
    exit 1
fi

BUILD_DIR="$(pwd)/build"
mkdir -p "$BUILD_DIR"
cmake -S "$(pwd)" -B "$BUILD_DIR"
cmake --build "$BUILD_DIR" -j
printf "%s\n%s\n" "$1" "$2" | "$BUILD_DIR/mafia"