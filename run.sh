#!/bin/bash
set -e

PROJECT_DIR="$(cd "$(dirname "$0")" && pwd)"

echo "==> Building..."
make -C "$PROJECT_DIR" -s

echo "==> Starting QEMU..."
qemu-system-x86_64 \
    -kernel "$PROJECT_DIR/build/kernel.elf" \
    -nographic
