CC = gcc
CFLAGS = -Wall -Wextra -std=c11
TARGET = cinema

SRCS = main.c common.c film.c catalog.c user.c favorites.c ui.c menu.c
OBJS = $(SRCS:.c=.o)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET)
