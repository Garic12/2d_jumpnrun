#!/usr/bin/env bash
# Builds release executables for Linux and Windows (cross-compiled with MinGW-w64).
# Resources are embedded into the binaries, so each build is a single file.
# Requires: cmake, git, gcc, mingw-w64-gcc (Arch: sudo pacman -S mingw-w64-gcc).
# Output: dist/Sidescroller-linux and dist/Sidescroller-windows.exe
set -euo pipefail

cd "$(dirname "$0")"

DIST_DIR=dist
JOBS="$(nproc)"

# ---------- Linux ----------
build_linux() {
    local build_dir=build-linux
    echo "==> Building Linux"
    # Statically link libgcc/libstdc++ so it runs on distros with older toolchains.
    # (glibc, OpenGL and X11/Wayland stay dynamic - those must come from the system.)
    cmake -B "$build_dir" \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_EXE_LINKER_FLAGS="-static-libgcc -static-libstdc++"
    cmake --build "$build_dir" --parallel "$JOBS"
    install -m 755 "$build_dir/Sidescroller" "$DIST_DIR/Sidescroller-linux"
    strip "$DIST_DIR/Sidescroller-linux"
}

# ---------- Windows ----------
build_windows() {
    local triple=x86_64-w64-mingw32
    local build_dir=build-windows
    echo "==> Building Windows"

    if ! command -v "$triple-g++" >/dev/null; then
        echo "error: $triple-g++ not found. Install MinGW-w64 (Arch: sudo pacman -S mingw-w64-gcc)." >&2
        exit 1
    fi

    # CMake toolchain file telling it to target Windows with the MinGW compilers
    local toolchain="$build_dir/mingw-toolchain.cmake"
    mkdir -p "$build_dir"
    cat > "$toolchain" <<TOOLCHAIN
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)
set(CMAKE_C_COMPILER   $triple-gcc)
set(CMAKE_CXX_COMPILER $triple-g++)
set(CMAKE_RC_COMPILER  $triple-windres)
set(CMAKE_FIND_ROOT_PATH /usr/$triple)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
TOOLCHAIN

    # Fully static so the exe runs without extra DLLs.
    # -mwindows hides the console window; remove it if you want console output.
    cmake -B "$build_dir" \
        -DCMAKE_TOOLCHAIN_FILE="$PWD/$toolchain" \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_EXE_LINKER_FLAGS="-static -static-libgcc -static-libstdc++ -mwindows"
    cmake --build "$build_dir" --parallel "$JOBS"
    install -m 755 "$build_dir/Sidescroller.exe" "$DIST_DIR/Sidescroller-windows.exe"
    "$triple-strip" "$DIST_DIR/Sidescroller-windows.exe"
}

rm -rf "$DIST_DIR"
mkdir -p "$DIST_DIR"

build_linux
build_windows

echo
echo "Built:"
ls -lh "$DIST_DIR"
