
CC = gcc
CFLAGS = -c -std=c99 -Wall -Wextra
EXE = chip8

HEADERS = chip8.h display.h
SOURCES = chip8.c display.c main.c
OBJS = $(SOURCES: .c = .o)

all: $(EXE)

$(EXE): $(OBJS)
	$(CC) $(OBJS) -lSDL2 -I/usr/include/SDL2 -o $(EXE)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -rf *.o *Zone.Identifier $(EXE)