#!/usr/bin/env bash
# =============================================================================
# build.sh — build and run the compiler inside Docker
# =============================================================================
set -euo pipefail

IMAGE_NAME="compiler5"

usage() {
    cat <<EOF
Usage: $0 [command]

Commands:
  build              Build the Docker image (default)
  run <file> [N]     Compile <file> to a native executable (opt-level N, default 0)
  shell              Open an interactive shell inside the container
  test               Run unit tests inside the container
  clean              Remove the Docker image

Examples:
  $0 build
  $0 run example/example.txt
  $0 run example/example_1.txt 2        # compile with -O2
  $0 run example/example_oop.txt 3      # compile with -O3
  $0 shell
  $0 test
EOF
}

cmd_build() {
    echo "==> Building Docker image '${IMAGE_NAME}'..."
    docker build -t "${IMAGE_NAME}" .
    echo "==> Done."
}

cmd_run() {
    local src="${1:-example/example.txt}"
    local opt="${2:-0}"
    echo "==> Compiling '${src}' (opt-level ${opt}) inside Docker..."
    docker run --rm \
        -v "$(pwd):/compiler" \
        -w /compiler \
        "${IMAGE_NAME}" \
        /bin/bash -c "
            set -e
            ./build/compiler '${src}' --compile program --opt-level ${opt} --print-ir
            echo ''
            echo '==> Running program...'
            ./program
        "
}

cmd_shell() {
    echo "==> Opening shell in '${IMAGE_NAME}'..."
    docker run --rm -it \
        -v "$(pwd):/compiler" \
        -w /compiler \
        "${IMAGE_NAME}" \
        /bin/bash
}

cmd_test() {
    echo "==> Running tests inside Docker..."
    docker run --rm \
        -v "$(pwd):/compiler" \
        -w /compiler \
        "${IMAGE_NAME}" \
        /bin/bash -c "./build/compiler_tests"
}

cmd_clean() {
    echo "==> Removing Docker image '${IMAGE_NAME}'..."
    docker rmi -f "${IMAGE_NAME}" || true
}

# ---- dispatch ---------------------------------------------------------------
COMMAND="${1:-build}"
shift || true

case "${COMMAND}" in
    build)  cmd_build ;;
    run)    cmd_run "$@" ;;
    shell)  cmd_shell ;;
    test)   cmd_test ;;
    clean)  cmd_clean ;;
    help|-h|--help) usage ;;
    *)
        echo "Unknown command: ${COMMAND}"
        usage
        exit 1
        ;;
esac
