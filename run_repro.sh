#!/usr/bin/env bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
EMSDK_DIR="$SCRIPT_DIR/emsdk"
EMSCRIPTEN_GIT_DIR="$SCRIPT_DIR/emscripten-src"
EMCC="$EMSDK_DIR/upstream/emscripten/emcc.py"
EMPP="$EMSDK_DIR/upstream/emscripten/em++.py"
NODE="$EMSDK_DIR/node/22.16.0_64bit/bin/node"
PYTHON="$EMSDK_DIR/python/3.13.3_64bit/bin/python3"

# Install emsdk 5.0.6 toolchain if not already present
if [ ! -d "$EMSDK_DIR" ]; then
    echo "==> Cloning emsdk..."
    git clone https://github.com/emscripten-core/emsdk.git "$EMSDK_DIR"
    cd "$EMSDK_DIR"
    ./emsdk install 5.0.6
    ./emsdk activate 5.0.6
fi
source "$EMSDK_DIR/emsdk_env.sh"

# Clone emscripten source separately for git checkout
if [ ! -d "$EMSCRIPTEN_GIT_DIR/.git" ]; then
    echo "==> Cloning emscripten source repo..."
    git clone https://github.com/emscripten-core/emscripten.git "$EMSCRIPTEN_GIT_DIR"
fi

# The 3 files that changed in commit 01b61e46d8
CHANGED_FILES=(
    "system/lib/pthread/library_pthread.c"
    "system/lib/pthread/library_pthread_stub.c"
    "system/lib/pthread/emscripten_futex_wait.c"
)

run_test() {
    local sha="$1"
    local label="$2"

    echo ""
    echo "=============================="
    echo " Testing emscripten @ $label ($sha)"
    echo "=============================="

    # Checkout the desired commit
    cd "$EMSCRIPTEN_GIT_DIR"
    git checkout "$sha"
    echo "emscripten source commit: $(git rev-parse --short HEAD)"

    # Copy only the changed files into emsdk's emscripten installation
    for f in "${CHANGED_FILES[@]}"; do
        cp "$EMSCRIPTEN_GIT_DIR/$f" "$EMSDK_DIR/upstream/emscripten/$f"
    done

    # Clear cache so libraries are rebuilt from the updated source
    "$PYTHON" "$EMCC" --clear-cache

    cd "$SCRIPT_DIR"
    echo "emcc: $("$PYTHON" "$EMCC" --version | head -1)"
    echo "node: $("$NODE" --version)"

    "$PYTHON" "$EMPP" -pthread -o "test_sleep_${label}.js" test_sleep.cpp

    echo -n "Result: "
    "$NODE" "test_sleep_${label}.js" || true
}

# Parent of 01b61e46d8 - before "Remove loop from emscripten_thread_sleep"
run_test "82e188595e" "before-01b61e46d8"

# The commit itself - after "Remove loop from emscripten_thread_sleep"
run_test "01b61e46d8" "after-01b61e46d8"

echo ""
echo "=============================="
echo " Done"
echo "=============================="
