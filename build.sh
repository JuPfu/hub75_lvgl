#!/usr/bin/env bash
# build.sh — configure & build hub75_lvgl, with an optional clean start.
#
# Usage:
#   ./build.sh              Reuse existing build/ if present (or create it)
#   ./build.sh -f|--fresh   Wipe build/ (and optionally .deps/) before building
#   ./build.sh -h|--help    Show this help
set -euo pipefail

BUILD_DIR="build"
DEPS_DIR=".deps"
FRESH=0

usage() {
    cat <<EOF
Usage: $0 [-f|--fresh] [-h|--help]
  -f, --fresh   Remove ${BUILD_DIR}/ before configuring (prompts about ${DEPS_DIR}/ too)
  -h, --help    Show this help
EOF
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        -f|--fresh) FRESH=1; shift ;;
        -h|--help)  usage; exit 0 ;;
        *) echo "Unknown option: $1"; usage; exit 1 ;;
    esac
done

# --- Decide whether to start from scratch -----------------------------------
if [[ -d "$BUILD_DIR" && $FRESH -eq 0 ]]; then
    read -rp "Build directory '${BUILD_DIR}' already exists. Reuse it? [Y/n] " reply
    reply=${reply:-Y}
    [[ "$reply" =~ ^[Nn] ]] && FRESH=1
fi

if [[ $FRESH -eq 1 && -d "$BUILD_DIR" ]]; then
    echo "==> Removing ${BUILD_DIR} ..."
    rm -rf "$BUILD_DIR"
fi

if [[ -d "$DEPS_DIR" ]]; then
    read -rp "Also clear cached LVGL/hub75 sources in '${DEPS_DIR}' (forces a full re-clone)? [y/N] " reply2
    if [[ "${reply2:-N}" =~ ^[Yy] ]]; then
        echo "==> Removing ${DEPS_DIR} ..."
        rm -rf "$DEPS_DIR"
    fi
fi

# --- Configure & build --------------------------------------------------------
# -B creates the build directory itself if it doesn't exist yet — no manual
# mkdir/cd dance needed, and no "directory does not exist" error is possible.
echo "==> Configuring ..."
cmake -S . -B "$BUILD_DIR" -G Ninja

echo "==> Building ..."
cmake --build "$BUILD_DIR" -j"$(nproc)"

echo "==> Done. Binaries are in ${BUILD_DIR}/"
