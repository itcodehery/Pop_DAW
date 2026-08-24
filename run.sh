#!/bin/bash

# Exit immediately if a command exits with a non-zero status
set -e

if [ ! -d "build/debug" ]; then
    echo "Build directory not found. Configuring with CMake..."
    cmake --preset debug
fi

echo "Building Pop DAW..."
cmake --build build/debug -j$(nproc)

echo "Build successful! Launching Pop DAW..."
"./build/debug/PopDAW_artefacts/Debug/Pop DAW"
