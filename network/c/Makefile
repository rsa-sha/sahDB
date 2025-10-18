CC = gcc
CFLAGS = -Wall -Wextra -I../../include -g
LIB = ../../build/libkv.a
OBJS = network.o 
# protocol.o
TARGET = sahDB

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIB)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)