#!/usr/bin/env bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
EMSDK_DIR="$SCRIPT_DIR/emsdk"

# Install emsdk if not already present
if [ ! -d "$EMSDK_DIR" ]; then
    echo "==> Cloning emsdk..."
    git clone https://github.com/emscripten-core/emsdk.git "$EMSDK_DIR"
fi

run_test() {
    local version="$1"
    echo ""
    echo "=============================="
    echo " Testing with Emscripten $version"
    echo "=============================="

    cd "$EMSDK_DIR"
    ./emsdk install "$version"
    ./emsdk activate "$version"
    source "$EMSDK_DIR/emsdk_env.sh"

    echo "emcc version: $(emcc --version | head -1)"
    echo "node version: $(node --version)"

    cd "$SCRIPT_DIR"
    em++ -pthread -o "test_sleep_${version}.js" test_sleep.cpp

    echo -n "Result: "
    node "test_sleep_${version}.js" && echo "" || echo ""
}

run_test "5.0.5"
run_test "5.0.6"

echo ""
echo "=============================="
echo " Done"
echo "=============================="
