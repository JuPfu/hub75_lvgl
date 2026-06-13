#!/bin/bash
set -e

BUILD_DIR="build"

echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "  hub75_lvgl build"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

if [ ! -f "$BUILD_DIR/build.ninja" ]; then
    echo "⚙️  Configuring (CMake)..."
    echo "   First run fetches LVGL from GitHub — please wait."
    cmake -B "$BUILD_DIR" -G Ninja -DCMAKE_BUILD_TYPE=Release
    echo "✅  Configuration done"
else
    echo "ℹ️  Build directory exists — skipping CMake configure"
fi

echo ""
echo "🔨 Building..."
echo "   Progress: [compiled/total elapsed]"
echo ""

NINJA_STATUS="[%f/%t %es] " cmake --build "$BUILD_DIR" --config Release

echo ""
echo "✅  Build complete → $BUILD_DIR/hub75_lvgl.uf2"
