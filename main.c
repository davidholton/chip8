// #include "chip8.h"
#include "display.h"

#include <stdio.h>

int main(int argc, char* argv[]) {

	Chip8 c8;

	chip8_initialize(&c8);
	chip8_load_rom(&c8, "roms/BRIX.ch8");

	//c8.logging = true;

	Display d;

	if (!display_init(&d, 64, 32)) {
		printf("Display could not initialize! SDL_Error: %s\n", SDL_GetError());
		exit(1);
	}

	int quit = 0;
	SDL_Event e;

	while (!quit) {
		while (SDL_PollEvent(&e) != 0) {
			if (e.type == SDL_QUIT) {
				quit = 1;
			}
			if (e.type == SDL_KEYDOWN) {
				for (int i = 0; i < 16; i++) {
					if (e.key.keysym.sym == KEYMAP[i])
						c8.keypad[i] = true;
				}
			}
			if (e.type == SDL_KEYUP) {
				for (int i = 0; i < 16; i++) {
					if (e.key.keysym.sym == KEYMAP[i])
						c8.keypad[i] = false;
				} 
			}
		}

		chip8_tick(&c8);

		display_draw(&d, &c8);

		SDL_Delay(2);
	}

	// stop SDL
	display_close(&d);

	return 0;
}