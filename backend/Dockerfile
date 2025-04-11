FROM debian:bookworm-slim

# Install essential dependencies
RUN apt-get update && apt-get install -y \
    clang \
    make \
    cmake \
    git \
    libargon2-dev \
    libasio-dev \
    libssl-dev \
    zlib1g-dev \
    pkg-config \
    libc6-dev \
    linux-libc-dev \
    libboost-system-dev \
    libboost-filesystem-dev \
    libboost-thread-dev \
    libboost-program-options-dev \
    gcc-12 \
    g++-12 \
    libc++-dev \
    libc++abi-dev \
    libstdc++-12-dev \
    && rm -rf /var/lib/apt/lists/*

# Set gcc-12 as the default compiler
RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-12 100 \
    && update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-12 100

# Install Crow with explicit ASIO include path
RUN git clone https://github.com/CrowCpp/Crow.git \
    && cd Crow \
    && mkdir build \
    && cd build \
    && cmake .. \
        -DCROW_BUILD_EXAMPLES=OFF \
        -DCROW_BUILD_TESTS=OFF \
        -DASIO_INCLUDE_DIR=/usr/include \
    && make install \
    && cd / \
    && rm -rf Crow

WORKDIR /app

# Copy backend files
COPY . .

# Create build directory
RUN mkdir -p build

# Set environment variables for the build
ENV CPLUS_INCLUDE_PATH=/usr/include/c++/12:/usr/include/aarch64-linux-gnu/c++/12:/usr/include/c++/12/aarch64-linux-gnu:/usr/local/include:/usr/include/aarch64-linux-gnu:/usr/include:/app/include:${CPLUS_INCLUDE_PATH}
ENV C_INCLUDE_PATH=/usr/lib/gcc/aarch64-linux-gnu/12/include:/usr/local/include:/usr/include/aarch64-linux-gnu:/usr/include:${C_INCLUDE_PATH}
ENV PKG_CONFIG_PATH=/usr/lib/pkgconfig:${PKG_CONFIG_PATH}
ENV LD_LIBRARY_PATH=/usr/lib/gcc/aarch64-linux-gnu/12:/usr/lib/aarch64-linux-gnu:${LD_LIBRARY_PATH}

# Build with verbose output and C++17
RUN make CXXFLAGS="-g -Wall -Wextra -std=c++17 -I/usr/include -I/usr/local/include -I/app/include -I/usr/include/argon2 -I/usr/local/include/crow -pthread" LDFLAGS="-largon2" VERBOSE=1

EXPOSE 3000

CMD ["./main"]