TARGET = irctl

SRC = $(wildcard src/*.c)
OBJ = $(SRC:.c=.o)

#DEFINES = -DDEBUG=1
INCLUDES = -Isrc -Iext/irmp-2.6.7
CFLAGS = -Wall -Wextra -Werror -pedantic $(INCLUDES) $(DEFINES)

$(TARGET): $(OBJ)
	$(CC) -o $@ $+

.PHONY: clean
clean:
	rm -f $(OBJ) $(TARGET)
