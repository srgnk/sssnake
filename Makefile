CCFLAGS=-D _DEFAULT_SOURCE -std=c99 -g
LDFLAGS=-lncurses

all: sssnake.c
	$(CC) $(CCFLAGS) -o sssnake sssnake.c $(LDFLAGS)

clean:
	$(RM) sssnake
