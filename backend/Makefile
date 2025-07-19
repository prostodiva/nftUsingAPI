# Makefile for C++ NFT Marketplace Backend
# Compatible with both macOS and Docker environments

CC = clang++
CFLAGS = -g -Wall -Wextra
CXXFLAGS = -g -Wall -Wextra -std=c++17 \
    -I/usr/local/include \
    -I/usr/include \
    -I/usr/include/c++/v1 \
    -pthread

# macOS-specific flags (only used on macOS)
ifeq ($(shell uname),Darwin)
    CXXFLAGS += -I/opt/homebrew/include \
                -I/opt/homebrew/include/crow \
                -I/opt/homebrew/opt/asio/include \
                -I/opt/homebrew/opt/boost/include \
                -arch arm64 \
                -isysroot $(shell xcrun --show-sdk-path) \
                -I$(shell xcrun --show-sdk-path)/usr/include/c++/v1 \
                -I$(shell xcrun --show-sdk-path)/usr/include \
                -I$(shell xcrun --show-sdk-path)/System/Library/Frameworks
    LDFLAGS = -pthread -L/opt/homebrew/lib -largon2
else
    # Linux/Docker flags
    CXXFLAGS += -I/usr/local/include/crow \
                -I/usr/include/c++/$(shell $(CC) -dumpversion) \
                -I/usr/include/aarch64-linux-gnu/c++/$(shell $(CC) -dumpversion) \
                -I/usr/include/aarch64-linux-gnu \
                -I/usr/include/c++/$(shell $(CC) -dumpversion)/aarch64-linux-gnu
    LDFLAGS = -pthread -largon2
endif

SRCDIR = src
INCDIR = include
BUILDDIR = build

SRCS = $(wildcard $(SRCDIR)/*.cpp)
OBJS = $(SRCS:$(SRCDIR)/%.cpp=$(BUILDDIR)/%.o)
DEPS = $(OBJS:.o=.d)

TARGET = main

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(BUILDDIR)
	$(CC) $(CXXFLAGS) -MMD -MP -c $< -o $@

-include $(DEPS)

# Debug build with extra debugging info
debug: CXXFLAGS += -g -O0 -DDEBUG
debug: $(TARGET)

# Debug build with ASAN and UBSAN
debug-asan: CXXFLAGS += -g -O0 -DDEBUG -fsanitize=address -fsanitize=undefined -fno-omit-frame-pointer
debug-asan: LDFLAGS += -fsanitize=address -fsanitize=undefined
debug-asan: $(TARGET)

# Release build
release: CXXFLAGS += -O2 -DNDEBUG
release: $(TARGET)

clean:
	rm -rf $(BUILDDIR) $(TARGET)

.PHONY: all clean debug release