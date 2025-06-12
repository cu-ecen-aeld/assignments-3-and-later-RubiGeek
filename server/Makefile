CC = gcc
CFLAGS = -Wall -Wextra -Werror
TARGET = aesdsocket
SRC = aesdsocket.c
HEADER = aesdsocket.h
OBJ = $(SRC:.c=.o)

# Default rule (runs when you type 'make')
all: $(TARGET)

# Compile the program
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

# Generate object file
$(OBJ): $(SRC) $(HEADER)
	$(CC) $(CFLAGS) -c $<

# Clean build files
clean:
	rm -f $(OBJ) $(TARGET)

# Run the program
run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run