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
	c8->pc	= 0x200;
	c8->sp	= 0;
	c8->I	= 0;

	memset(c8->mem, 0, sizeof(c8->mem));
	memset(c8->v, 0, sizeof(c8->v));
	memset(c8->stack, 0, sizeof(c8->stack));
	memset(c8->display, 0, sizeof(c8->display));

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
					printf("8xy6 - SHR Vx {, Vy}");

					c8->v[0xF] = (c8->v[x] & 0x000F) == 1 ? 1: 0;
					c8->v[x] >>= 1; // divide by two

					break;
				case 0x0007:
					printf("8xy7 - SUBN Vx, Vy");

					c8->v[0xF] = c8->v[y] > c8->v[x] ? 1: 0;
					c8->v[x] = c8->v[y] - c8->v[x];

					break;
				case 0x000E:
					printf("8xyE - SHL Vx, {, Vy}");

					c8->v[0xF] = (c8->v[x] & 0xF000) >> 12 == 1 ? 0: 1;
					c8->v[x] <<= 1; // multiple by two

					break;
				default:
					printf("ERROR: Could not execute opcode: %04x\n", opcode);
					exit(1);
			}
		}
		case 0x9000: {
			printf("9xy0 - SNE vx, Vy");

			uint8_t x = (opcode & 0x0F00) >> 8;
			uint8_t y = (opcode & 0x00F0) >> 4;
			if (c8->v[x] != c8->v[y])
				c8->pc += 2;

			break;
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
