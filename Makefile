# Makefile
LDFLAGS=-lncurses -g

all: sssnake.c
	$(CC) -o sssnake sssnake.c $(LDFLAGS)

clean:
	$(RM) sssnake
