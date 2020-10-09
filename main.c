#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

typedef struct c8 {

	uint8_t  mem[0x1000];			// 4096 bytes of RAM
	uint8_t  v[0x10];				// 16 8-bit registers
	uint16_t stack[0x10];			// 16 16-bit stack

	uint16_t pc;					// 16-bit program counter
	uint16_t I;						// 16-bit memory address register
	uint8_t  sp;					// 8-bit stack pointer

	uint8_t  display[0x40][0x20];	// 64x32 monochromatic display
	char     keypad[0x10];			// 16-key hexadecimal keypad

	uint8_t  dt;					// delay timer register
	uint8_t  st;					// sound timer register
	uint8_t  rate;					// Hz which decrement both timers

} Chip8;

void chip8_init(Chip8 *c8) {

	printf("/*----------initializing chip8----------*\\\n");

	printf("\tmem[0x1000]\t%ld bytes\n", sizeof(c8->mem));
	printf("\tv[0x10]\t\t%ld bytes\n", sizeof(c8->v));
	printf("\tstack[0x10]\t%ld bytes\n\n", sizeof(c8->stack));

	printf("\tpc\t\t%ld bytes\n", sizeof(c8->pc));
	printf("\tsp\t\t%ld byte\n\n", sizeof(c8->sp));
	printf("\tI\t\t%ld bytes\n", sizeof(c8->I));

	// Should be 2048 bits not bytes?
	printf("\tdisplay[0x800]\t%ld bytes\n", sizeof(c8->display));
	printf("/*--------------------------------------*\\\n\n");


	// initialize values and set memory
	c8->pc	 = 0x200;
	c8->sp	 = 0;
	c8->I	 = 0;
	c8->dt   = 0;
	c8->st   = 0;
	c8->rate = 60;

	memset(c8->mem, 0, sizeof(c8->mem));
	memset(c8->v, 0, sizeof(c8->v));
	memset(c8->stack, 0, sizeof(c8->stack));
	memset(c8->display, 0, sizeof(c8->display));
	memset(c8->keypad, 0, sizeof(c8->keypad));

	// load in font data
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

	for (int i=0; i<80; i++) {
		c8->mem[i] = font_data[i];

		// display font char
		/*
		for (int j=0; j<4; j++)
			printf("%c", (c8->mem[i] & (0x80) >> j) ? '*':' ');
		printf("\n");
		if ((i+1)%5 == 0)
			printf("\n");
		*/
	}

	return;
}

void chip8_debug(Chip8 *c8) {
	printf("/*----chip8 state----*\\\n");
	printf("\tpc:\t%d\n", c8->pc);
	printf("\tsp:\t%d\n", c8->sp);
	printf("\tI:\t%d\n", c8->I);

	return;
}

uint16_t fetch_opcode(Chip8 *c8) {
	/*
		Fetch the two byte instruction (16-bits) pointed to by the PC
		Increment the PC by two
		Return the two byte instruction
	*/

	// put first byte in the high bits and second byte in low bytes
	uint16_t opcode = c8->mem[c8->pc] << 8 | c8->mem[c8->pc+1];

	// increment the program counter by 2
	c8->pc += 2;

	return opcode;
}


void chip8_exec(Chip8 *c8, uint16_t opcode) {
	printf("opcode:\t%04x\n", opcode);

	switch(opcode & 0xF000) {
		case 0x0000:
			switch(opcode & 0x00FF) {
				case 0x00E0: // CLS
					// todo
					printf("0x00E0 - CLS\n");
					break;
				case 0x00EE: // RET
					printf("0x00EE - RET\n");

					c8->pc=c8->stack[c8->sp];
					c8->sp--;

					break;
				default:
					printf("ERROR: Could not execute opcode: %04x\n", opcode);
					exit(1);
			}
		case 0x1000:
			printf("0x1nnn - JP addr\n");

			c8->pc = opcode & 0x0FFF;

			break;
		case 0x2000:
			printf("0x2nnn - CALL addr\n");

			c8->stack[++c8->sp] = c8->pc;
			c8->pc = opcode & 0x0FFF;

			break;
		case 0x3000: {
			printf("0x3xkk - SE Vx, byte\n");

			uint8_t x = (opcode & 0x0F00) >> 8;
			uint8_t kk = opcode & 0x00FF;
			if (c8->v[x] == kk)
				c8->pc += 2;

			break;
		}
		case 0x4000: {
			printf("0x4xkk - SNE Vx, byte\n");

			uint8_t x = (opcode & 0x0F00) >> 8;
			uint8_t kk = opcode & 0x00FF;
			if (c8->v[x] != kk)
				c8->pc += 2;

			break;
		}
		case 0x5000: {
			printf("0x5xy0 - SE Vx, Vy\n");

			uint8_t x = (opcode & 0x0F00) >> 8;
			uint8_t y = (opcode & 0x00F0) >> 4;
			if (c8->v[x] == c8->v[y])
				c8->pc += 2;

			break;
		}
		case 0x6000: {
			printf("6xkk - LD Vx, byte\n");

			uint8_t x = (opcode & 0x0F00) >> 8;
			uint8_t kk = opcode & 0x00FF;
			c8->v[x] = kk;

			break;
		}
		case 0x7000: {
			printf("7xkk - ADD Vx, byte\n");

			uint8_t x = (opcode & 0x0F00) >> 8;
			uint8_t kk = opcode & 0x00FF;
			c8->v[x] += kk;

			break;
		}
		case 0x8000: {
			uint8_t x = (opcode & 0x0F00) >> 8;
			uint8_t y = (opcode & 0x00F0) >> 4;

			switch (opcode & 0x000F) {
				case 0x0000:
					printf("8xy0 - LD Vx, Vy\n");

					c8->v[x] = c8->v[y];

					break;
				case 0x0001:
					printf("8xy1 - OR Vx, Vy\n");

					c8->v[x] |= c8->v[y];

					break;
				case 0x0002:
					printf("8xy2 - AND Vx, Vy\n");

					c8->v[x] &= c8->v[y];

					break;
				case 0x0003:
					printf("8xy3 - XOR Vx, Vy\n");

					c8->v[x] ^= c8->v[y];

					break;
				case 0x0004: {
					printf("8xy4 - ADD Vx, Vy\n");

					uint8_t result = c8->v[x] + c8->v[y];
					c8->v[0xF] = result > 255 ? 0: 1;
					c8->v[x] = result & 0x000F;

					break;
				}
				case 0x0005:
					printf("8xy5 - SUB Vx, Vy\n");

					c8->v[0xF] = c8->v[x] > c8->v[y] ? 1: 0;
					c8->v[x] -= c8->v[y];

					break;
				case 0x0006:
					printf("8xy6 - SHR Vx {, Vy}\n");

					c8->v[0xF] = (c8->v[x] & 0x000F) == 1 ? 1: 0;
					c8->v[x] >>= 1; // divide by two

					break;
				case 0x0007:
					printf("8xy7 - SUBN Vx, Vy\n");

					c8->v[0xF] = c8->v[y] > c8->v[x] ? 1: 0;
					c8->v[x] = c8->v[y] - c8->v[x];

					break;
				case 0x000E:
					printf("8xyE - SHL Vx, {, Vy}\n");

					c8->v[0xF] = (c8->v[x] & 0xF000) >> 12 == 1 ? 0: 1;
					c8->v[x] <<= 1; // multiple by two

					break;
				default:
					printf("ERROR: Could not execute opcode: %04x\n", opcode);
					exit(1);
			}
		}
		case 0x9000: {
			printf("9xy0 - SNE vx, Vy\n");

			uint8_t x = (opcode & 0x0F00) >> 8;
			uint8_t y = (opcode & 0x00F0) >> 4;
			if (c8->v[x] != c8->v[y])
				c8->pc += 2;

			break;
		}
		case 0xA000:
			printf("Annn - LD I, addr\n");

			c8->I = opcode & 0x0FFF;

			break;
		case 0xB000:
			printf("Bnnn - JP V0, addr\n");

			c8->pc = (opcode & 0x0FFF) + c8->v[0x0];

			break;
		case 0xC000: {
			printf("Cxkk - RND Vc, byte\n");

			uint8_t x = (opcode & 0x0F00) >> 8;
			uint8_t kk = opcode & 0x00FF;
			c8->v[x] = (rand() % 256) & kk;

			break;
		}
		case 0xD000: {
			printf("Dxyn - DRW Vx, Vy, nibble\n");

			uint8_t x0 = c8->v[(opcode & 0x0F00) >> 8] % 0x40;
			uint8_t y0 = c8->v[(opcode & 0x00F0) >> 4] % 0x20;
			uint8_t n = opcode & 0x000F; // height of sprite

			// todo - implment a redraw screen flag 
			c8->v[0xF] = 0; // unset collision bit

			for (uint8_t i=y0; i < y0+n && i < 0x20; i++) {
				uint8_t byte = c8->mem[c8->I + i];

				for (uint8_t k=0; k < 8; k++) {
					if (((byte >> k) & 0x1) == 1) {
						if (c8->display[x0 + k][y0 + i] == 1)
							c8->v[0xF] = 1;

						c8->display[x0 + k][y0 + i] ^= 1;
					}
				}
			}

			break;
		}
		case 0xE000: {
			uint8_t x = (opcode & 0x0F00) >> 8;

			switch (opcode & 0x00FF) {
				case 0x009E:
					printf("Ex9E - SKP Vx\n");

					if (c8->keypad[x] == 1)
						c8->pc += 2;

					break;
				case 0x00A1:
					printf("ExA1 - SKNP Vx\n");

					if (c8->keypad[x] == 0)
						c8->pc += 2;

					break;
				default:
					printf("ERROR: Could not execute opcode: %04x\n", opcode);
					exit(1);
			}
		}
		case 0xF000: {
			uint8_t x = (opcode & 0x0F000) >> 8;

			switch (opcode & 0x00FF) {
				case 0x0007:
					printf("Fx07 - LD Vx, DT\n");

					c8->v[x] = c8->dt;

					break;
				case 0x000A:
					printf("Fx0A - LD Vx, K\n");

					for (uint8_t k=0x0; k<=0x10; k++) {
						if (c8->keypad[k] == 1) {
							c8->v[x] = k;
							break;
						}
					}

					c8->pc -= 2; // decrement PC by two bytes to create a spin lock

					break;
				case 0x0015:
					printf("Fx15 - LD DT, Vx\n");

					c8->dt = c8->v[x];

					break;
				case 0x0018:
					printf("Fx18 - LD ST, Vx\n");

					c8->st = c8->v[x];

					break;
				case 0x001E:
					printf("Fx1E - ADD I, Vx\n");

					c8->I += c8->v[x];

					break;
				case 0x0029:
					printf("0xFx29 - LD F, Vx\n");

					c8->I = c8->v[x] * 5;

					break;
				case 0x0033:
					printf("0xFx33 - LD B, Vx\n");
					break;
				case 0x0055:
					printf("Fx55 - LD [I], Vx\n");
					break;
				case 0x0065:
					printf("Fx65 - LD Vx, [I]\n");
					break;
				default:
					printf("ERROR: Could not execute opcode: %04x\n", opcode);
					exit(1);
			}
		}
		default:
			printf("ERROR: Could not execute opcode: %04x\n", opcode);
			exit(1);
	}

	return;
}


int main(int argc, char* argv[]) {

	Chip8 c8;

	chip8_init(&c8);

	//for (;;) {

	c8.mem[0x200] = 0x00;
	c8.mem[0x201] = 0xE0;
	uint16_t opcode = fetch_opcode(&c8);
	chip8_exec(&c8, opcode);
	//}

	return 0;
}
