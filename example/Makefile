TARGET = example
DEP = $(TARGET:=.d)

CPPFLAGS = -MMD -I../include
CFLAGS = -std=c11 -pedantic -pedantic-errors -Wall -Wextra -Werror
LDFLAGS = -L..
LDLIBS = -ldots

all: FORCE $(TARGET)

$(TARGET): $(TARGET).c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) $(TARGET).c $(LDLIBS) -o $(TARGET)

clean: FORCE
	rm -rf $(TARGET) $(DEP)

FORCE:

-include $(DEP)
