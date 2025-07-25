FROM debian:bookworm-slim

# Install required packages
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    curl \
    wget \
    pkg-config \
    libssl-dev \
    libudev-dev \
    libclang-dev \
    libpq-dev \
    python3 \
    python3-pip \
    libasio-dev \
    clang \
    lldb \
    libc++-dev \
    libc++abi-dev \
    g++ \
    libstdc++-12-dev \
    && rm -rf /var/lib/apt/lists/*

# Set default compiler for ARM64 (M2 Mac)
ENV CC=clang
ENV CXX=clang++

# Install Rust and Cargo (needed for Solana CLI)
RUN curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y
ENV PATH="/root/.cargo/bin:${PATH}"

# Install Solana CLI tools (requires Rust/Cargo as Solana CLI is written in Rust)
RUN cargo install solana-cli --version 1.16.15

# Configure Solana to use testnet
RUN solana config set --url https://api.testnet.solana.com

# Configure git for non-interactive use
RUN git config --global url."https://".insteadOf git:// && \
    git config --global http.sslVerify false

# Clone and build Crow library
RUN git clone https://github.com/CrowCpp/Crow.git /tmp/crow && \
    cd /tmp/crow && \
    mkdir build && \
    cd build && \
    cmake .. -DCROW_BUILD_EXAMPLES=OFF -DCROW_BUILD_TESTS=OFF && \
    make install && \
    rm -rf /tmp/crow

# Set working directory
WORKDIR /app

# Copy source files
COPY . .

# Build the application using clang++
RUN make clean && make

# Expose port
EXPOSE 3000

# Run the application
CMD ["./main"]