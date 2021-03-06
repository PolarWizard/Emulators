#ifndef INTEL_8080_H
#define INTEL_8080_H
#include <iostream>
#include <fstream>
#include <iomanip>

using namespace std;

class Intel_8080 {
protected:
	//cpu cycles/instructions
	int cycles = 0;
	//Registers
	uint8_t regs[8] = { 0x00 };
	#define B regs[0]
	#define C regs[1]
	#define D regs[2]
	#define E regs[3]
	#define H regs[4]
	#define L regs[5]
	#define A regs[7]
	uint8_t F = 0x02; //Flags: S Z x A x P x C
	#define BC (((uint16_t)B << 8) | (uint16_t)C)
	#define DE (((uint16_t)D << 8) | (uint16_t)E)
	#define HL (((uint16_t)H << 8) | (uint16_t)L)
	#define PSW (((uint16_t)A << 8) | (uint16_t)((F & 0xD7) | 0x02) 
	//Program Counter
	uint16_t PC = 0x0000;
	//Stack Pointer
	uint16_t SP = 0x0000;
	//Memory
	uint8_t RAM[65535] = { 0x00 };
	//Interrupt Flip Flop
	//1 = interrupts enabled
	//0 = interrupts disabled
	bool interrupt = 1;
public:
	Intel_8080() {};
	//handles interrupt requests sent to the CPU
	void IRQ(uint16_t PCaddr);
	//This init is used to fake the CP/M OS to run the CPU tests, for Space Invaders this is ignored
	void init();
	//Loads RAM with the program
	void loadRAM();
	//debugging purposes, shows the contents of RAM until end of program is reached
	void viewMemoryContents();
	//executes opcode, this function has every instruction mapped to it
	int execute(Intel_8080* i8080);
	//functions that handle dealing with setting/resetting flags
	void handle_sign(uint8_t arg1);
	void handle_zero(uint8_t arg1);
	void handle_aux(uint8_t arg1, uint8_t arg2, uint8_t operation);
	void handle_parity(uint8_t arg1);
	void handle_carry(uint32_t arg1);
	void handle_carry(uint16_t arg1);
};

#endif INTEL_8080_H
