TARGET_SO = libdots.so
TARGET_AR = libdots.a
OBJS = \
	dots/control.o \
	dots/env.o \
	dots/msg.o
DEPS = $(OBJS:.o=.d)

CPPFLAGS = -MMD -Iinclude
CFLAGS = -std=c11 -pedantic -pedantic-errors -fPIC -O3 -Wall -Wextra -Werror
LDFLAGS = -shared
LDLIBS =

all: FORCE $(TARGET_SO) $(TARGET_AR) example

$(TARGET_SO): $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) $(LDLIBS) -o $(TARGET_SO)

$(TARGET_AR): $(OBJS)
	$(AR) rcs $(TARGET_AR) $(OBJS)

clean: FORCE
	rm -rf $(TARGET_SO) $(TARGET_AR) $(OBJS) $(DEPS)
	$(MAKE) -C example clean

FORCE:

example: FORCE
	$(MAKE) -C example

-include $(DEPS)
