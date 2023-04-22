CC = gcc
CFLAGS = -Wall -pthread
INCLUDE_DIR = include
SRC_DIR = src

ifdef DEBUG
CFLAGS += -g -DDEBUG
endif

# Add all C source files in the src directory to the SOURCES variable
SOURCES := $(wildcard $(SRC_DIR)/*.c)

# Replace .c extensions with .o extensions for each source file
OBJECTS := $(SOURCES:$(SRC_DIR)/%.c=%.o)

# Add include directory to the compiler's include path
CFLAGS += -I$(INCLUDE_DIR)

# Default target: build the executable
webserver: $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $(OBJECTS)

# Build object files from C source files
%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up object files and the executable
clean:
	rm -f webserver $(OBJECTS)
