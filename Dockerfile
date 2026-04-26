FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

# Install base tools and LLVM 17
RUN apt-get update && apt-get install -y \
    wget \
    gnupg \
    lsb-release \
    software-properties-common \
    ca-certificates \
    && wget -qO- https://apt.llvm.org/llvm-snapshot.gpg.key \
       | tee /etc/apt/trusted.gpg.d/apt.llvm.org.asc \
    && echo "deb http://apt.llvm.org/jammy/ llvm-toolchain-jammy-17 main" \
       > /etc/apt/sources.list.d/llvm.list \
    && apt-get update && apt-get install -y \
    llvm-17 \
    llvm-17-dev \
    clang-17 \
    lld-17 \
    cmake \
    ninja-build \
    build-essential \
    git \
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

WORKDIR /compiler

COPY . .

# Remove any stale build cache that may have been copied from the host
RUN rm -rf build

# Configure: use llvm-config-17 --cmakedir to get the correct path at build time
# On failure, print the full CMake error log before exiting
RUN LLVM_CMAKE=$(llvm-config-17 --cmakedir) && \
    echo "Using LLVM cmake dir: $LLVM_CMAKE" && \
    cmake -S . -B build \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_C_COMPILER=clang-17 \
        -DCMAKE_CXX_COMPILER=clang++-17 \
        -DLLVM_DIR="$LLVM_CMAKE" \
        -G Ninja \
    || (cat build/CMakeFiles/CMakeError.log 2>/dev/null; \
        cat build/CMakeFiles/CMakeOutput.log 2>/dev/null; \
        exit 1)

RUN cmake --build build

CMD ["/bin/bash"]
