#!/usr/bin/env bash
# =============================================================================
# build.sh — build and run the compiler inside Docker
# =============================================================================
set -euo pipefail

IMAGE_NAME="compiler2"

usage() {
    cat <<EOF
Usage: $0 [command]

Commands:
  build              Build the Docker image (default)
  run <file> [opts]  Compile <file>, emit IR, link with clang, and run
  shell              Open an interactive shell inside the container
  test               Run unit tests inside the container
  clean              Remove the Docker image

Examples:
  $0 build
  $0 run example/example.txt
  $0 run example/example_shadowing.txt
  $0 run example/example_oop.txt
  $0 shell
  $0 test
EOF
}

cmd_build() {
    echo "==> Building Docker image '${IMAGE_NAME}'..."
    docker build --platform linux/amd64 --pull -t "${IMAGE_NAME}" .
    echo "==> Done."
}

cmd_run() {
    local src="${1:-example/example.txt}"
    shift || true
    echo "==> Compiling '${src}' inside Docker..."
    docker run --rm --platform linux/amd64 \
        -v "$(pwd):/compiler" \
        -w /compiler \
        "${IMAGE_NAME}" \
        /bin/bash -c "
            set -e
            /opt/build/compiler '${src}' -o output.ll --print-ir $*
            echo ''
            echo '==> Compiling IR with clang...'
            clang output.ll -o program
            echo '==> Running program...'
            ./program
        "
}

cmd_shell() {
    echo "==> Opening shell in '${IMAGE_NAME}'..."
    docker run --rm -it --platform linux/amd64 \
        -v "$(pwd):/compiler" \
        -w /compiler \
        "${IMAGE_NAME}" \
        /bin/bash
}

cmd_test() {
    echo "==> Running tests inside Docker..."
    docker run --rm --platform linux/amd64 \
        -v "$(pwd):/compiler" \
        -w /compiler \
        "${IMAGE_NAME}" \
        /bin/bash -c "/opt/build/compiler_tests"
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
