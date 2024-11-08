
CC=gcc
LD=gcc

CFLAGS = -g -Wno-int-to-pointer-cast -Wno-pointer-to-int-cast -Werror -I$(INCDIR) -I../kernel/src

SRCDIR = src
INCDIR = ../kernel/include
BUILD_DIR = build

C_SOURCES = $(shell find "$(SRCDIR)" -name '*.c')
K_SOURCES = $(shell find "../kernel/src" -name '*.c')
HEADERS = $(shell find "$(INCDIR)" -name '*.h')

OBJ = ${C_SOURCES:%.c=$(BUILD_DIR)/%.o}

test: $(BUILD_DIR)/test
	$<

$(BUILD_DIR)/test: $(OBJ) $(HEADERS) $(K_SOURCES)
	$(LD) $(LDFLAGS) $(OBJ) -o $@

$(BUILD_DIR)/%.o: %.c $(HEADERS) Makefile
	@mkdir -p $(shell dirname $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	@rm -rf $(BUILD_DIR)

.PHONY: test clean
