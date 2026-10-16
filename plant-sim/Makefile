CC = gcc
CFLAGS = -Wall -Wextra -g
LIBS = $(shell pkg-config --cflags --libs libmidbus)

SRCS = main.c
OBJS = $(SRCS:.c=.o)
TARGET = plant-sim

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)