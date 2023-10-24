$(shell mkdir -p build)

CC               ?= clang
CFLAGS            = -std=c99 -O2 -Wall -Wextra -Wpedantic -Werror-implicit-function-declaration -Wno-unused-parameter -Wno-missing-field-initializers
LDFLAGS 	      = -lsystemd

UBJSON_SRCFILES  := $(shell find ubjson/ -name '*.c')
UBJSON_OBJFILES  := $(patsubst ubjson/%.c,build/%.o,$(UBJSON_SRCFILES))

all: ubjson
	$(CC) foobard.c $(CFLAGS) $(LDFLAGS) -Lbuild/ -lubjson -o build/foobard

build/%.o: ubjson/%.c
	$(CC) -c -o $@ $< $(CFLAGS)

ubjson: $(UBJSON_OBJFILES)
	ar rcs build/libubjson.a $^

.PHONY: clean
clean:
	rm -rf build/
