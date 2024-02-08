SRC=main.c
OBJ=$(patsubst %.c,obj/%.o,$(SRC))
OUT=obj-convert

CC=gcc
CFLAGS=-std=c2x -pedantic-errors -Wall -Wextra -Wno-unused-parameter
LDFLAGS=-s
LIBS=

all: $(OUT)

.PHONY: all clean

$(OUT): $(OBJ)
	$(CC) $(LDFLAGS) $^ $(LIBS) -o $@

obj/%.o: src/%.c
	@mkdir -p `dirname $@`
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf obj $(OUT)
