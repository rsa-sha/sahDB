CC = gcc
CFLAGS = -Wall -Wextra -I../../include -g
# for commands.yaml
LDFLAGS = -L/usr/lib64 -lyaml
LIB = ../../build/libkv.a
OBJS = network.o 
# protocol.o
TARGET = sahDB

all: $(TARGET)

$(TARGET): $(OBJS) $(LIB)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)