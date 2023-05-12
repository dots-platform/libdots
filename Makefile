TARGET_SO = libdots.so
TARGET_AR = libdots.a
OBJS = \
	dots/env.o \
	dots/msg.o \
	dots/output.o \
	dots/request.o \
	dots/internal/control_msg.o
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

.PHONY: rust
rust: FORCE
	rm -rf $(RUST_DIR)/libdots
	mkdir $(RUST_DIR)/libdots
	set -x \
		&& origpwd=$$PWD \
		&& tmpdir=$$(mktemp -d) \
		&& trap 'rm -rf "$$tmpdir"' EXIT \
		&& cp -R .git .gitignore Makefile dots include "$$tmpdir" \
		&& cd "$$tmpdir" \
		&& git clean -dfx \
		&& cp -R .gitignore Makefile dots include "$$origpwd/$(RUST_DIR)/libdots"
	cd $(RUST_DIR) && cargo build

rust-publish: rust FORCE
	set -x \
		&& tmpdir=$$(mktemp -d) \
		&& trap 'rm -rf "$$tmpdir"' EXIT \
		&& cp -R $(RUST_DIR)/. "$$tmpdir" \
		&& rm -rf "$$tmpdir/target" \
		&& ln -s "$$PWD/$(RUST_DIR)/target" "$$tmpdir/target" \
		&& cd "$$tmpdir" \
		&& cargo publish


-include $(DEPS)
