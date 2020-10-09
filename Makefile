driver: main.o
	gcc -g -Wall --std=c99 -o chip8 main.o
main.o: main.c
	gcc -g -Wall --std=c99 -c main.c -o main.o
clean:
	rm chip8 main.o *Zone.Identifier
