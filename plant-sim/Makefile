CC = gcc
CFLAGS = -Wall -Wextra -g $(shell pkg-config --cflags libmodbus)
LIBS = $(shell pkg-config --cflags --libs libmodbus)

SRCS = main.c
OBJS = $(SRCS:.c=.o)
TARGET = main

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)