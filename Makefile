# --- Build Configuration ---

# Compiler to use
CC = gcc

# Strict Compilation Flags (The core requirement)
# -Wall: Enable all standard warnings
# -Wextra: Enable extra warnings
# -Werror: Treat warnings as errors (Non-negotiable for robust code)
# -pedantic: Enforce strict adherence to the C standard
# -g: Include debugging information (for GDB)
# -std=c11: Specify the C standard
CFLAGS = -Wall -Wextra -Werror -pedantic -g 

# Linker Flags (e.g., add -lm for math library)
LDFLAGS =

# Name of the final executable
TARGET = program

# --- File Discovery ---

# Find all .c files in the current directory
SOURCES = $(wildcard *.c)

# Generate object file names from source files (e.g., main.c -> main.o)
OBJECTS = $(SOURCES:.c=.o)

# --- Main Targets ---

# Default target: builds the program
all: $(TARGET)

# Link the final executable: $(TARGET)
$(TARGET): $(OBJECTS)
	@echo "--- Linking executable: $(TARGET) ---"
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

# Compile C source files into object files: %.o
# This is a pattern rule that applies to all files ending in .o
%.o: %.c
	@echo "--- Compiling $< ---"
	$(CC) $(CFLAGS) $< -o $@

# --- Utility Targets ---

# Clean target: remove all generated files
clean:
	@echo "--- Cleaning up build artifacts ---"
	rm -f $(TARGET) $(OBJECTS)

.PHONY: all clean
