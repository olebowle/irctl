TARGET = irctl
PREFIX ?= /usr/local

BINDIR = $(DESTDIR)$(PREFIX)/bin

SRC = $(wildcard src/*.c)
OBJ = $(SRC:.c=.o)

#DEFINES = -DDEBUG=1
INCLUDES = -Isrc -Iext/irmp-2.6.7
CFLAGS = -Wall -Wextra -Werror -pedantic $(INCLUDES) $(DEFINES)

$(TARGET): $(OBJ)
	$(CC) -o $@ $+

install: $(TARGET)
	install -Dm644 $(TARGET) $(BINDIR)/$(TARGET)

.PHONY: clean
clean:
	rm -f $(OBJ) $(TARGET)
