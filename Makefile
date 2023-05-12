TARGET_SO = libdots.so
TARGET_AR = libdots.a
OBJS = \
	dots/env.o \
	dots/msg.o \
	dots/output.o \
	dots/request.o \
	dots/internal/control_msg.o
SRCS = $(OBJS:.o=.c)
DEPS = $(OBJS:.o=.d)

CPPFLAGS = -MMD -Iinclude
CFLAGS = -std=c11 -pedantic -pedantic-errors -fPIC -O3 -Wall -Wextra -Werror
LDFLAGS = -shared
LDLIBS = \
	-lpthread

RUST_DIR = rust

all: FORCE $(TARGET_SO) $(TARGET_AR) example

$(TARGET_SO): $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) $(LDLIBS) -o $(TARGET_SO)

$(TARGET_AR): $(OBJS)
	$(AR) rcs $(TARGET_AR) $(OBJS)

clean: FORCE
	rm -rf $(TARGET_SO) $(TARGET_AR) $(OBJS) $(DEPS)
	$(MAKE) -C example clean

FORCE:

example: $(TARGET_SO) FORCE
	$(MAKE) -C example

rust-publish: FORCE
	set -eux \
		&& tmpdir=$$(mktemp -d) \
		&& trap 'rm -rf "$$tmpdir"' EXIT \
		&& cp -R $(RUST_DIR)/. "$$tmpdir" \
		&& rm -rf "$$tmpdir/target" \
		&& ln -s "$$PWD/$(RUST_DIR)/target" "$$tmpdir/target" \
		&& mkdir -p $(RUST_DIR)/target \
		&& rm -rf "$$tmpdir/libdots" \
		&& mkdir "$$tmpdir/libdots" \
		&& cp -R .gitignore Makefile include "$$tmpdir/libdots" \
		&& ( cd "$$tmpdir/libdots" && mkdir -p $(shell dirname $(SRCS)) ) \
		&& for src in $(SRCS); do cp $$src "$$tmpdir/libdots/$$src"; done \
		&& cd "$$tmpdir" \
		&& cargo publish


-include $(DEPS)
