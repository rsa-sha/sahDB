CC = gcc
CFLAGS = -Wall -Wextra -I../include -g
OBJS = db.o hash.o command.o common.o config.o dbsave.o ttl.o
TARGET = libkv.a

all: $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJS)
	ar rcs $@ $^

clean:
	rm -f $(OBJS) $(TARGET)