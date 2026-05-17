FROM --platform=linux/amd64 ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

# Ubuntu 24.04 (noble) ships LLVM 17 in its default repos.
# No external apt.llvm.org repository needed.
RUN apt-get update && apt-get install -y --no-install-recommends \
    llvm-17 \
    llvm-17-dev \
    clang-17 \
    lld-17 \
    cmake \
    ninja-build \
    build-essential \
    git \
    ca-certificates \
    libffi-dev \
    libncurses-dev \
    zlib1g-dev \
    libzstd-dev \
    && apt-get clean && rm -rf /var/lib/apt/lists/*

# Make versioned tools available under their canonical names
RUN update-alternatives --install /usr/bin/llvm-config llvm-config /usr/bin/llvm-config-17 100 \
    && update-alternatives --install /usr/bin/clang   clang   /usr/bin/clang-17   100 \
    && update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-17 100

# Verify LLVM cmake dir exists and print it for debugging
RUN echo "LLVM cmake dir: $(llvm-config-17 --cmakedir)" \
    && ls "$(llvm-config-17 --cmakedir)"

WORKDIR /src

COPY . .

# Remove any stale build cache that may have been copied from the host
RUN rm -rf build

# Build into /opt/build so it survives volume mounts over /compiler at runtime
RUN LLVM_CMAKE=$(llvm-config-17 --cmakedir) && \
    echo "Using LLVM cmake dir: $LLVM_CMAKE" && \
    cmake -S . -B /opt/build \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_C_COMPILER=clang-17 \
        -DCMAKE_CXX_COMPILER=clang++-17 \
        -DLLVM_DIR="$LLVM_CMAKE" \
        -G Ninja \
    || (cat /opt/build/CMakeFiles/CMakeError.log 2>/dev/null; \
        cat /opt/build/CMakeFiles/CMakeOutput.log 2>/dev/null; \
        exit 1)

RUN cmake --build /opt/build

WORKDIR /compiler

CMD ["/bin/bash"]
