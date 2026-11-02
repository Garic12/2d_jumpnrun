#!/usr/bin/env bash
# Cross-compiles Sidescroller.exe for Windows from Linux using MinGW-w64.
# Requires: mingw-w64-gcc (Arch: sudo pacman -S mingw-w64-gcc), cmake, git.
# Output: dist/Sidescroller/ and Sidescroller-windows.zip (if zip is installed).
set -euo pipefail

cd "$(dirname "$0")"

TRIPLE=x86_64-w64-mingw32
BUILD_DIR=build-windows
DIST_DIR=dist/Sidescroller

if ! command -v "$TRIPLE-g++" >/dev/null; then
    echo "error: $TRIPLE-g++ not found. Install MinGW-w64 (Arch: sudo pacman -S mingw-w64-gcc)." >&2
    exit 1
fi

# CMake toolchain file telling it to target Windows with the MinGW compilers
TOOLCHAIN="$BUILD_DIR/mingw-toolchain.cmake"
mkdir -p "$BUILD_DIR"
cat > "$TOOLCHAIN" <<EOF
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)
set(CMAKE_C_COMPILER   $TRIPLE-gcc)
set(CMAKE_CXX_COMPILER $TRIPLE-g++)
set(CMAKE_RC_COMPILER  $TRIPLE-windres)
set(CMAKE_FIND_ROOT_PATH /usr/$TRIPLE)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
EOF

# Statically link libgcc/libstdc++/winpthread so the exe runs without extra DLLs.
# -mwindows hides the console window; remove it if you want console output.
cmake -B "$BUILD_DIR" \
    -DCMAKE_TOOLCHAIN_FILE="$PWD/$TOOLCHAIN" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_EXE_LINKER_FLAGS="-static -static-libgcc -static-libstdc++ -mwindows"

cmake --build "$BUILD_DIR" --parallel "$(nproc)"

# Package the exe together with the assets it loads at runtime
rm -rf "$DIST_DIR"
mkdir -p "$DIST_DIR"
cp "$BUILD_DIR/Sidescroller.exe" "$DIST_DIR/"
cp -r ressources "$DIST_DIR/"

if command -v zip >/dev/null; then
    rm -f Sidescroller-windows.zip
    (cd dist && zip -qr ../Sidescroller-windows.zip Sidescroller)
    echo "Built: $DIST_DIR/Sidescroller.exe and Sidescroller-windows.zip"
else
    echo "Built: $DIST_DIR/Sidescroller.exe"
fi
