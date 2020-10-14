driver: main.o chip8.o
	gcc -o chip8 main.o chip8.o
main.o: main.c chip8.h
	gcc -c main.c
chip8.o: chip8.c chip8.h
	gcc -c chip8.c
clean:
	rm chip8 main.o chip8.0 *Zone.Identifier