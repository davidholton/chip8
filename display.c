#include "display.h"

int display_init(Display *d, unsigned int width, unsigned int height) {
	int success = 1;

	d->width = width;
	d->height = height;

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		success = 0;
	} else {
		d->window = SDL_CreateWindow("CHIP-8", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, d->width, d->height, SDL_WINDOW_SHOWN);
		if (d->window == NULL) {
			success = 0;
		} else {
			d->renderer = SDL_CreateRenderer(d->window, -1, SDL_RENDERER_ACCELERATED);
			if (d->renderer == NULL) {
				success = 0;
			} else {
				d->texture = SDL_CreateTexture(d->renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, d->width, d->height);
				if (d->texture == NULL) {
					success = 0;
				} else {
					SDL_SetRenderDrawColor(d->renderer, 0xFF, 0xFF, 0xFF, 0xFF);
					SDL_RenderClear(d->renderer);
					SDL_RenderPresent(d->renderer);

					d->pixel_buffer = malloc(width * height * sizeof(uint32_t));
					memset(d->pixel_buffer, 0, sizeof(d->pixel_buffer));
				}
			}
		}
	}

	return success;
}

void display_close(Display* d) {

	SDL_DestroyWindow(d->window);
	d->window = NULL;

	SDL_DestroyRenderer(d->renderer);
	d->renderer = NULL;

	SDL_DestroyTexture(d->texture);
	d->texture = NULL;

	free(d->pixel_buffer);

	SDL_Quit();
}

void display_draw(Display* d, Chip8* c8) {
	if (c8->screen_update) {
		c8->screen_update = false;
		
		for (int x = 0; x < 64; x++) {
			for (int y = 0; y < 32; y++) {
				d->pixel_buffer[y * 64 + x] = (0xFFFFFF00 * c8->screen[x][y]) | 0xFF;
			}
		}
	}
	
	SDL_UpdateTexture(d->texture, NULL, d->pixel_buffer, 64 * sizeof(uint32_t));

	SDL_RenderClear(d->renderer);
	SDL_RenderCopy(d->renderer, d->texture, NULL, NULL);
	SDL_RenderPresent(d->renderer);

	return;
}