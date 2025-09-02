CC = gcc

CFLAGS = -Wall -Werror -std=c11 -march=native -O0 -g

BIN = hashmap

SRC = src/main.c lib/hashmap.c

OBJS = src/main.o lib/hashmap.o


.SUFFIXES: .o .c


$(BIN): $(OBJS)
	$(CC) $(OBJS) $(CFLAGS) -o $(BIN)


.c.o:
	$(CC) -Iinclude -c $< -o $@ $(CFLAGS)


run: $(BIN)
	@$(MAKE) && ./$(BIN)


tests: test_main.c
	$(CC) -Iinclude $< -o $@ $(CFLAGS)


clean:
	rm -f $(OBJS) $(BIN)
