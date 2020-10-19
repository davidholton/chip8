driver: main.o chip8.o display.o
	gcc -o chip8 main.o chip8.o display.o -lSDL2
main.o: main.c chip8.h display.h
	gcc -c main.c -I/usr/include/SDL2
chip8.o: chip8.c chip8.h
	gcc -c chip8.c
display.o: display.c display.h
	gcc -c display.c -I/usr/include/SDL2
clean:
	rm chip8 *.o *Zone.Identifier