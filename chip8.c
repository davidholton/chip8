#include "chip8.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void chip8_initialize(Chip8* c8) {

	c8->sp = 0;
	c8->pc = 0x200;
	c8->I  = 0;

	c8->screen_update = false;

	c8->dt     = 0;
	c8->st     = 0;
	c8->cycles = 0;

	c8->opcode  = 0x0000;
	c8->logging = false;

	memset(c8->mem,    0, sizeof(c8->mem));
	memset(c8->v,      0, sizeof(c8->v));
	memset(c8->stack,  0, sizeof(c8->stack));
	memset(c8->screen, 0, sizeof(c8->screen));
	for (int i = 0; i < 16; i++)
		c8->keypad[i] = false;

	// Fonts are 4-bits x 5-bits
	unsigned char font_data[80] = {
		0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
		0x20, 0x60, 0x20, 0x20, 0x70, // 1
		0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
		0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
		0x90, 0x90, 0xF0, 0x10, 0x10, // 4
		0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
		0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
		0xF0, 0x10, 0x20, 0x40, 0x40, // 7
		0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
		0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
		0xF0, 0x90, 0xF0, 0x90, 0x90, // A
		0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
		0xF0, 0x80, 0x80, 0x80, 0xF0, // C
		0xE0, 0x90, 0x90, 0x90, 0xE0, // D
		0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
		0xF0, 0x80, 0xF0, 0x80, 0x80  // F 
	};

	for (int i = 0; i < 80; i++)
		c8->mem[i] = font_data[i];

	return;
}

void chip8_load_rom(Chip8* c8, const char* filename) {
	FILE* fp = fopen(filename, "r");
	if (fp == NULL) {
		printf("ERROR: could not open rom file \"%s\"\n", filename);
		exit(1);
	}

	fseek(fp, 0L, SEEK_END);
	long int rom_size = ftell(fp);
	if (rom_size > 0x1000) {
		printf("ERROR: rom size is too large to load\n");
		exit(1);
	}

	fseek(fp, 0L, SEEK_SET);

	uint8_t* buffer = (uint8_t*)malloc(rom_size * sizeof(uint8_t));
	if (buffer == NULL) {
		printf("ERROR: could not create rom buffer\n");
		fclose(fp);
		exit(1);
	}

	fread(buffer, sizeof(uint8_t), rom_size, fp);

	for (long int i = 0; i < rom_size; i++)
		c8->mem[0x200 + i] = buffer[i];

	fclose(fp);
	free(buffer);

	return;
}

void chip8_tick(Chip8* c8) {
	c8->cycles += 1;

	// COSMAC VIP systems ran at 540hz and the timers ran at 60hz
	// So very roughly every 9th execution cycle we should expect
	// the timers to decrement
	if (c8->cycles % 9 == 0)
		chip8_decrement_timers(c8);

	c8->opcode = fetch_opcode(c8);;

	chip8_execute(c8);

	// Increment program counter by two bytes if its not:
	// 1nnn, 2nnn, Bnnn because they change the program counter
	uint8_t a = (c8->opcode & 0xF000) >> 12;
	if (a != 0x1 && a != 0x2 && a != 0xB)
		c8->pc += 2;

	return;
}

void chip8_execute(Chip8* c8) {

	// Variables used by opcodes
	uint8_t  a   = (c8->opcode & 0xF000) >> 12;
	uint8_t  x   = (c8->opcode & 0x0F00) >> 8;
	uint8_t  y   = (c8->opcode & 0x00F0) >> 4;
	uint8_t  n   =  c8->opcode & 0x000F;
	uint8_t  kk  =  c8->opcode & 0x00FF;
	uint16_t nnn =  c8->opcode & 0x0FFF;

	switch (a) {
		case 0x0: {
			switch (kk) {
				case 0xE0: {
					if (c8->logging) printf("00E0 - CLS\n");
					memset(c8->screen, 0, sizeof(c8->screen));

					break;
				}
				case 0xEE: {
					if (c8->logging) printf("00EE - RET\n");
					c8->pc = c8->stack[c8->sp];
					c8->sp--;

					break;
				}
				default: {
					printf("1ERROR: Could not execute opcode: %04x\n", c8->opcode);
					exit(1);
				}
			}
			break;
		}
		case 0x1: {
			if (c8->logging) printf("1nnn - JP addr\n");
			c8->pc = nnn;

			break;
		}
		case 0x2: {
			if (c8->logging) printf("2nnn - CALL addr\n");
			c8->stack[++c8->sp] = c8->pc;
			c8->pc = nnn;

			break;
		}
		case 0x3: {
			if (c8->logging) printf("3xkk - SE Vx, byte\n");
			if (c8->v[x] == kk)
				c8->pc += 2;

			break;
		}
		case 0x4: {
			if (c8->logging) printf("4xkk - SNE Vx, byte\n");
			if (c8->v[x] != kk)
				c8->pc += 2;

			break;
		}
		case 0x5: {
			if (c8->logging) printf("5xy0 - SE Vx, Vy\n");
			if (c8->v[x] == c8->v[y])
				c8->pc += 2;

			break;
		}
		case 0x6: {
			if (c8->logging) printf("6xkk - LD Vx, byte\n");
			c8->v[x] = kk;

			break;
		}
		case 0x7: {
			if (c8->logging) printf("7xkk - ADD Vx, byte\n");
			c8->v[x] += kk;

			break;
		}
		case 0x8: {
			switch (n) {
				case 0x0: {
					if (c8->logging) printf("8xy0 - LD Vx, Vy\n");
					c8->v[x] = c8->v[y];

					break;
				}
				case 0x1: {
					if (c8->logging) printf("8xy1 - OR Vx, Vy\n");
					c8->v[x] |= c8->v[y];

					break;
				}
				case 0x2: {
					if (c8->logging) printf("8xy2 - AND Vx, Vy\n");
					c8->v[x] &= c8->v[y];

					break;
				}
				case 0x3: {
					if (c8->logging) printf("8xy3 - XOR Vx, Vy\n");
					c8->v[x] ^= c8->v[y];

					break;
				}
				case 0x4: {
					if (c8->logging) printf("8xy4 - ADD Vx, Vy\n");
					uint16_t result = c8->v[x] + c8->v[y];
					c8->v[0xF] = (result > 0xFF) ? 1 : 0;
					c8->v[x] = result & 0xFF; // Only store low byte

					break;
				}
				case 0x5: {
					if (c8->logging) printf("8xy5 - SUB Vx, Vy\n");
					c8->v[0xF] = c8->v[x] > c8->v[y] ? 1 : 0;
					c8->v[x] -= c8->v[y];

					break;
				}
				case 0x6: {
					if (c8->logging) printf("8xy6 - SHR Vx {, Vy}\n");
					c8->v[0xF] = c8->v[x] & 0x1;
					c8->v[x] >>= 1; if (c8->logging) printf("divide by two\n");

					break;
				}
				case 0x7: {
					if (c8->logging) printf("8xy7 - SUBN Vx, Vy\n");
					c8->v[0xF] = c8->v[y] > c8->v[x] ? 1 : 0;
					c8->v[x] = c8->v[y] - c8->v[x];

					break;
				}
				case 0xE: {
					if (c8->logging) printf("8xyE - SHL Vx, {, Vy}\n");
					c8->v[0xF] = c8->v[x] >> 7;
					c8->v[x] <<= 1; // multiple by two

					break;
				}
				default: {
					printf("2ERROR: Could not execute opcode: %04x\n", c8->opcode);
					exit(1);
				}
			}
			break;
		}
		case 0x9: {
			if (c8->logging) printf("9xy0 - SNE vx, Vy\n");
			if (c8->v[x] != c8->v[y])
				c8->pc += 2;

			break;
		}
		case 0xA: {
			if (c8->logging) printf("Annn - LD I, addr\n");
			c8->I = nnn;

			break;
		}
		case 0xB: {
			if (c8->logging) printf("Bnnn - JP V0, addr\n");
			c8->pc = nnn + c8->v[0x0];

			break;
		}
		case 0xC: {
			if (c8->logging) printf("Cxkk - RND Vc, byte\n");
			c8->v[x] = (rand() % 256) & kk; // todo: dont use rand or at least set a seed

			break;
		}
		case 0xD: {
			if (c8->logging) printf("Dxyn - DRW Vx, Vy, nibble\n");
			uint8_t x0 = c8->v[x] % 0x40;
			uint8_t y0 = c8->v[y] % 0x20;

			c8->v[0xF] = 0;
			c8->screen_update = true;

			for (unsigned i = 0; i < n; i++) {
				uint8_t line_byte = c8->mem[c8->I + i];

				for (int k = 0; k < 8; k++) {
					uint8_t bit = (line_byte >> (7 - k)) & 1;

					if (((line_byte >> (7 - k)) & 1) == 1) {
						if (c8->screen[x0 + k][y0 + i] == 1)
							c8->v[0xF] = 1;

						c8->screen[x0 + k][y0 + i] ^= bit;
					}
				}
			}
			
			// for (int line = 0; y < n; y++) {
			// 	uint8_t pixel = c8->v[c8->I + line];
			// 	for (uint8_t xline = 0; xline < 8; xline++) {
			// 		if ((pixel & (0x80>>xline)) != 0) {
			// 			if (c8->screen[x0 + xline][y0+line] == 1)
			// 				c8->v[0xF] = 1;
			// 			 c8->screen[x0 + xline][y0+line]  ^= 1;
			// 		}
			// 	}
			// }

			break;
		}
		case 0xE: {
			switch (kk) {
				case 0x9E: {
					if (c8->logging) printf("Ex9E - SKP Vx\n");
					if (c8->keypad[x] == true)
						c8->pc += 2;

					break;
				}
				case 0xA1: {
					if (c8->logging) printf("ExA1 - SKNP Vx\n");
					if (c8->keypad[x] == false)
						c8->pc += 2;

					break;
				}
				default: {
					printf("3ERROR: Could not execute opcode: %04x\n", c8->opcode);
					exit(1);
				}
			}
			break;
		}
		case 0xF: {
			switch (kk) {
				case 0x07: {
					if (c8->logging) printf("Fx07 - LD Vx, DT\n");
					c8->v[x] = c8->dt;

					break;
				}
				case 0x0A: {
					if (c8->logging) printf("Fx0A - LD Vx, K\n");
					for (unsigned k = 0; k < 16; k++) {
						if (c8->keypad[k] == true) {
							c8->v[x] = k;
							if (c8->logging) printf("break; todo\n");
						}
					}

					if (c8->logging) printf("Decrement PC by two bytes to create a spin lock\n");
					c8->pc -= 2;

					break;
				}
				case 0x15: {
					if (c8->logging) printf("Fx15 - LD DT, Vx\n");
					c8->dt = c8->v[x];
					
					break;
				}
				case 0x18: {
					if (c8->logging) printf("Fx18 - LD ST, Vx\n");
					c8->st = c8->v[x];

					break;
				}
				case 0x1E: {
					if (c8->logging) printf("Fx1E - ADD I, Vx\n");
					c8->I += c8->v[x];

					break;
				}
				case 0x29: {
					if (c8->logging) printf("Fx29 - LD F, Vx\n");
					c8->I = c8->v[x] * 5; // todo

					break;
				}
				case 0x33: {
					if (c8->logging) printf("Fx33 - LD B, Vx\n");
					c8->mem[c8->I] = c8->v[x] / 100;
					c8->mem[c8->I + 1] = (c8->v[x] % 100) / 10;
					c8->mem[c8->I + 2] = c8->v[x] % 10;

					break;
				}
				case 0x55: {
					if (c8->logging) printf("Fx55 - LD [I], Vx\n");
					for (unsigned i = 0; i <= x; i++)
						c8->mem[c8->I + i] = c8->v[i];
					//	c8->I does not get incremented for support of modern ROMs.
					//	Older (70s-80s) might depend on it being set after each
					//	register is stored into memory. This also goes for Fx65.

					break;
				}
				case 0x65: {
					if (c8->logging) printf("Fx65 - LD Vx, [I]\n");

					for (uint8_t i = 0; i <= x; i++)
						c8->v[i] = c8->mem[c8->I + i];
					// See comment in Fx55
					
					break;
				}
				default: {
					printf("4ERROR: Could not execute opcode: %04x\n", c8->opcode);
					exit(1);
				}
			}
			break;
		}
		default: {
			printf("5ERROR: Could not execute opcode: %04x\n", c8->opcode);
			exit(1);
		}
	}

	return;
}

void chip8_decrement_timers(Chip8* c8) {
	// Decrement the timers if they are larger than 0
	if (c8->dt > 0)
		c8->dt -= 1;

	if (c8->st > 0)
		c8->st -= 1;

	return;
}

uint16_t fetch_opcode(Chip8* c8) {
	// First byte is placed in high bits, and second byte in low bits
	uint16_t opcode = c8->mem[c8->pc] << 8 | c8->mem[c8->pc + 1];

	return opcode;
}