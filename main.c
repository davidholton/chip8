#include "chip8.h"

#include <SDL.h>
#include <stdio.h>

const int SCREEN_WIDTH = 64;
const int SCREEN_HEIGHT = 32;

int display_init(void);
void display_close(void);

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
SDL_Texture *texture = NULL;

int main(int argc, char* argv[]) {

	Chip8 c8;

	chip8_initialize(&c8);
	chip8_load_rom(&c8, "roms/test_opcode.ch8");

	c8.logging = true;

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

	if (!display_init()) {
		printf("Display could not initialize! SDL_Error: %s\n", SDL_GetError());
		exit(1);
	}

	int quit = 0;
	SDL_Event e;

	uint32_t *pixels = malloc(SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(uint32_t));

	while (!quit) {
		while (SDL_PollEvent(&e) !=0 ) {
			if (e.type == SDL_QUIT) {
				quit = 1;
			}
		}

		chip8_tick(&c8);

		if (c8.screen_update) {
			c8.screen_update = false;
			
			for (int x = 0; x < SCREEN_WIDTH; x++) {
				for (int y = 0; y < SCREEN_HEIGHT; y++) {
					pixels[y * SCREEN_WIDTH + x] = (0xFFFFFF00 * c8.screen[x][y]) | 0xFF;
				}
			}
		}

		SDL_UpdateTexture(texture, NULL, pixels, SCREEN_WIDTH * sizeof(uint32_t));

		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, texture, NULL, NULL);
		SDL_RenderPresent(renderer);
	}

	
	

	// SDL_Delay(2000);

	// stop SDL
	display_close();

	return 0;
}

int display_init(void) {
	int success = 1;

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		success = 0;
	} else {
		window = SDL_CreateWindow("CHIP-8", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
		if (window == NULL) {
			success = 0;
		} else {
			renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
			if (renderer == NULL) {
				success = 0;
			} else {
				texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, SCREEN_WIDTH, SCREEN_HEIGHT);
				if (texture == NULL) {
					success = 0;
				} else {
					SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
					SDL_RenderClear(renderer);
					SDL_RenderPresent(renderer);
				}
			}
		}
	}

	return success;
}

void display_close(void) {

	SDL_DestroyWindow(window);
	window = NULL;

	SDL_DestroyRenderer(renderer);
	renderer = NULL;

	SDL_DestroyTexture(texture);
	texture = NULL;

	SDL_Quit();
}