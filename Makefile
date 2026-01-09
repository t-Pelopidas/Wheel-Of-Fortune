# --- Build Configuration ---
CC = gcc

# Strict Compilation Flags
# Added -D_DEFAULT_SOURCE to suppress common warnings with standard headers
CFLAGS = -Wall -Wextra -Werror -pedantic -g -D_DEFAULT_SOURCE

# --- Main Targets ---

all: server client

# Build the Server Executable
server: server.o
	@echo "--- Linking server ---"
	$(CC) server.o -o server

# Build the Client Executable
client: client.o
	@echo "--- Linking client ---"
	$(CC) client.o -o client

# --- Compilation Rule ---

# The -c flag is CRITICAL. It compiles source to object code without linking.
%.o: %.c
	@echo "--- Compiling $< ---"
	$(CC) -c $(CFLAGS) $< -o $@
	rm *.o


# --- Utility ---

clean:
	@echo "--- Cleaning up ---"
	rm -f server client *.o
