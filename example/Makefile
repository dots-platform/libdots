TARGET = example
DEP = $(TARGET:=.d)

CPPFLAGS = -MMD -I../include
CFLAGS = -std=c11 -pedantic -pedantic-errors -Wall -Wextra -Werror
LDFLAGS = -L..
LDLIBS = -ldots

all: FORCE $(TARGET)

$(TARGET): $(TARGET).c

FORCE:

-include $(DEP)
