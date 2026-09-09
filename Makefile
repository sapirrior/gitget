CXX ?= g++
CXXFLAGS ?= -std=c++20 -Wall -Wextra -Wpedantic -O2 -Iinclude
CURL_CFLAGS ?= $(shell pkg-config --cflags libcurl 2>/dev/null || curl-config --cflags)
CURL_LIBS ?= $(shell pkg-config --libs libcurl 2>/dev/null || curl-config --libs)

CXXFLAGS += $(CURL_CFLAGS)
LDFLAGS += $(CURL_LIBS)

TARGET = gitget
BUILD_DIR = build
SRCS = src/main.cpp \
       src/router.cpp \
       src/downloader.cpp \
       src/url_parser.cpp \
       src/providers/github.cpp \
       src/providers/gitlab.cpp \
       src/providers/bitbucket.cpp \
       src/providers/codeberg.cpp \
       src/providers/raw.cpp

OBJS = $(SRCS:src/%.cpp=$(BUILD_DIR)/%.o)

PREFIX ?= /usr/local
BINDIR ?= $(PREFIX)/bin

.PHONY: all clean install

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR) $(TARGET)

install: $(TARGET)
	install -d $(DESTDIR)$(BINDIR)
	install -m 755 $(TARGET) $(DESTDIR)$(BINDIR)
