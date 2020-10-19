#ifndef CHIP8_H
#define CHIP8_8

#include <stdint.h>
#include <stdbool.h>

typedef struct c8 {

	uint8_t  mem[0x1000];			// 4096 bytes of RAM
	uint8_t  v[0x10];				// 16 8-bit registers

	uint16_t stack[0x10];			// 16 16-bit stack
	uint8_t  sp;					// 8-bit stack pointer

	uint16_t pc;					// 16-bit program counter
	uint16_t I;						// 16-bit memory address register

	uint8_t  screen[0x40][0x20];	// 64x32 monochromatic display
	bool     screen_update;			// screen update flag

	bool     keypad[0x10];			// 16-key hexadecimal keypad

	uint8_t  dt;					// delay timer register
	uint8_t  st;					// sound timer register
	uint8_t  cycles;				// cycle count

	uint16_t opcode;				// the executing opcode

	bool     logging;				// prints executing opcode

} Chip8;

void chip8_initialize(Chip8* c8);
void chip8_load_rom(Chip8* c8, const char* filename);

void chip8_tick(Chip8* c8);
void chip8_execute(Chip8* c8);
void chip8_decrement_timers(Chip8* c8);

uint16_t fetch_opcode(Chip8* c8);

#endif