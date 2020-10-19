#ifndef DISPLAY_H
#define DISPLAY_H

#include "chip8.h"

#include <SDL.h>
#include <stdbool.h>

typedef struct display {

	unsigned int width;
	unsigned int height;

	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_Texture *texture;

	uint32_t *pixel_buffer;

} Display;

int display_init(Display *d, unsigned int width, unsigned int height);
void display_close(Display *d);

void display_draw(Display *d, Chip8* c8);

#endif