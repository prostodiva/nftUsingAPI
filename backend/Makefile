#version for lldb

CC = clang++
CFLAGS = -g -Wall -Wextra
CXXFLAGS = -g -Wall -Wextra -std=c++14 \
    -I/opt/homebrew/include \
    -I/opt/homebrew/include/crow \
    -I/opt/homebrew/opt/asio/include \
    -I/opt/homebrew/opt/boost/include \
    -I/usr/local/include\
    -pthread \
    -arch arm64 \
    -isysroot /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk
LDFLAGS = -pthread -L/opt/homebrew/lib -largon2

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

ggdb: CXXFLAGS += -g
ggdb: $(TARGET)


clean:
	rm -rf $(BUILDDIR) $(LIBDIR) $(TARGET)

.PHONY: all clean ggdb