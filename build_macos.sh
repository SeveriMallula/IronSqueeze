#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")"
cmake -S . -B build -G Xcode '-DCMAKE_OSX_ARCHITECTURES=arm64;x86_64'
cmake --build build --config Release --parallel 4
ctest --test-dir build -C Release --output-on-failure
bash packaging/macos/package.sh
