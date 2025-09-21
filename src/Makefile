# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -Wextra -g

# Output executable
TARGET = sahDB

# Object files
OBJS = db.o main.o common.o command.o

# Default target
all: $(TARGET)

# Rule to link object files into the final executable
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET)

# Rule to compile db.c into db.o
db.o: db.c db.h
	$(CC) $(CFLAGS) -c db.c

# Rule to compile common.h into common.o
common.o: common.c common.h
	$(CC) $(CFLAGS) -c common.c

# Rule to compile command.h into command.o
command.o: command.c command.h
	$(CC) $(CFLAGS) -c command.c

# Rule to compile main.c into main.o
main.o: main.c db.h
	$(CC) $(CFLAGS) -c main.c

# Clean up compiled files
clean:
	rm -f $(OBJS) $(TARGET)

# Phony targets (not actual files)
.PHONY: all clean
