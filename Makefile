CC ?= cc
CFLAGS += -Wall -Wextra -Wshadow -ggdb -Og

SRC_FILES = test.c
OBJ_FILES = $(SRC_FILES:.c=.o)
BIN = a.out

$(BIN): $(OBJ_FILES)
	$(CC) -o $(BIN) $(OBJ_FILES) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -rf *.o $(BIN)
