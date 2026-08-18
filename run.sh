#!/bin/bash

# Exit immediately if a command exits with a non-zero status
set -e

echo "Building Pop DAW..."
cmake --build build/debug -j$(nproc)

echo "Build successful! Launching Pop DAW..."
./build/debug/PopDAW_artefacts/Debug/Pop\ DAW
