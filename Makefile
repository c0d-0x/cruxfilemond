CC             := gcc

CFLAGS         := -Wall -Wextra -Wformat-security -Wformat-overflow=2 #-ggdb
CFLAGS += -O3 -s # strip bin
# Debugging params 
CFLAGS += -fsanitize=address
CFLAGS += -fno-omit-frame-pointer
CFLAGS += -fsanitize-recover=address

INCLUDE        := -Iinclude -Ilib

LDLIBS         := -lm -ldl -lpthread

SRC            := $(wildcard src/core/*.c)
CONF           := $(wildcard src/conf/*.c)
EVENT		   := $(wildcard src/event/*.c)
ALL_SRC        := $(SRC) $(CONF) $(EVENT)

OBJ            := $(ALL_SRC:src/%.c=build/%.o)

BIN            := bin/cruxfilemond
BIN_NAME	   := cruxfilemond
CRX_MSG        := cruxfilemond_ipc

PREFIX         := /usr/
all: $(BIN)
	@echo '[+] Build complete (dev).'

$(BIN): $(OBJ)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS)  $(OBJ) -o $@ $(LDLIBS)

build/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(INCLUDE) $(CFLAGS) -c $< -o $@


.PHONY: all clean run 

clean:
	@rm -rf build $(BIN)
	@echo "[+] Clean up complete."

run:
	./$(BIN) -d
