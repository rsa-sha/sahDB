# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -Wextra -g

# Output dir
BUILD_DIR = ../build

# Output executable
TARGET = $(BUILD_DIR)/sahDB


# Source file
SRCS = db.c main.c common.c command.c hash.c dbsave.c
# Object files
OBJS = $(addprefix $(BUILD_DIR)/, $(SRCS:.c=.o))

# Default target
all: $(TARGET)

# Rule to link object files into the final executable
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET)

# Compile each .c file to .o
$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up compiled files
clean:
	rm -f $(BUILD_DIR)/*.o $(TARGET)

# Phony targets (not actual files)
.PHONY: all clean
