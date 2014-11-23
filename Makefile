TARGET = irctl
PREFIX ?= /usr/local

BINDIR = $(DESTDIR)$(PREFIX)/bin

SRCS = $(wildcard src/*.c)
OBJS = $(SRCS:.c=.o)

#DEFINES = -DDEBUG
INCLUDES = -Isrc -Iext/irmp-2.6.7
CFLAGS = -Wall -Wextra -Werror -pedantic $(INCLUDES) $(DEFINES)

$(TARGET): $(OBJS)
	$(CC) -o $@ $^

.PHONY: install clean
install: $(TARGET)
	install -Dm755 $(TARGET) $(BINDIR)/$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
