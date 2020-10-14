#include "chip8.h"

#include <stdio.h>

int main(int argc, char* argv[]) {

	Chip8 c8;

	chip8_initialize(&c8);
	chip8_load_rom(&c8, "jumping.ch8");

	c8.logging = false;

	for (;;) {
		chip8_tick(&c8);

		if (c8.screen_update) {
			c8.screen_update = false;
			
			for (unsigned y = 0; y < 32; y++) {
				for (unsigned x = 0; x < 64; x++)
					printf("%s", c8.screen[x][y] == 1 ? "\u2593": "\u2591");
				printf("\n");
			}
		}
	}

	return 0;
}