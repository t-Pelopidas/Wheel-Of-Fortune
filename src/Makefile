CC = gcc

CFLAGS = -Wall -Wextra -Werror -pedantic -g -D_DEFAULT_SOURCE

all: server client

server: server.o
	@echo "--- Linking server ---"
	$(CC) server.o -o server
	rm server.o

client: client.o
	@echo "--- Linking client ---"
	$(CC) client.o -o client
	rm client.o

%.o: %.c
	@echo "--- Compiling $< ---"
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	@echo "--- Cleaning up ---"
	rm -f server client *.o
