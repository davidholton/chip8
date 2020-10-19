// #include "chip8.h"
#include "display.h"

#include <stdio.h>

const uint8_t KEYMAP[16] = {
	SDLK_x, // 0
	SDLK_1, // 1
	SDLK_2, // 2
	SDLK_3, // 3
	SDLK_q, // 4
	SDLK_w, // 5
	SDLK_e, // 6
	SDLK_a, // 7
	SDLK_s, // 8
	SDLK_d, // 9
	SDLK_z, // A
	SDLK_c, // B
	SDLK_4, // C
	SDLK_r, // D
	SDLK_f, // E
	SDLK_v  // F
};


int main(int argc, char* argv[]) {

	Chip8 c8;

	chip8_initialize(&c8);
	chip8_load_rom(&c8, "roms/BRIX.ch8");

	//c8.logging = true;

	// for (;;) {
	// 	chip8_tick(&c8);

	// 	if (c8.screen_update) {
	// 		c8.screen_update = false;
			
	// 		for (unsigned y = 0; y < 32; y++) {
	// 			for (unsigned x = 0; x < 64; x++)
	// 				printf("%s", c8.screen[x][y] == 1 ? "\u2593": "\u2591");
	// 			printf("\n");
	// 		}
	// 	}
	// }

	Display d;

	if (!display_init(&d, 64, 32)) {
		printf("Display could not initialize! SDL_Error: %s\n", SDL_GetError());
		exit(1);
	}

	int quit = 0;
	SDL_Event e;

	
	//uint32_t *pixel_buffer = malloc(SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(uint32_t));

	while (!quit) {
		while (SDL_PollEvent(&e) !=0 ) {
			if (e.type == SDL_QUIT) {
				quit = 1;
			}
			if (e.type == SDL_KEYDOWN) {
				for (int i = 0; i < 16; i++) {
					if (e.key.keysym.sym == KEYMAP[i]) {
						printf("KEYMAP[%d]\n", KEYMAP[i]);
						c8.keypad[i] = true;
					}
				}
			}
			if (e.type == SDL_KEYUP) {
				for (int i = 0; i < 16; i++) {
					if (e.key.keysym.sym == KEYMAP[i]) {
						c8.keypad[i] = false;
					}
				} 
			}
		}

		chip8_tick(&c8);

		display_draw(&d, &c8);

		SDL_Delay(3);
	}

	// SDL_Delay(2000);

	// stop SDL
	display_close(&d);

	return 0;
}