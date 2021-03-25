#include "MOS_6502.h"
#include <iostream>
#include <iomanip>
#include <fstream>

using namespace std;

void MOS_6502::powerUp() {
	P = 0x34;
	A = 0;
	X = 0;
	Y = 0;
	S = 0xFD;
	//0x4017 = 0;
	//0x4015 = 0;
	//0x4000-400F = 0;
	//0x4010-4013 = 0;
	//noise channel LFSR = 0x0000;
	//APU frame counter reset
	PC = (read(0xFFFD) << 8) | read(0xFFFC);
	for (int i = 0; i < 2048; i++) {
		c_bus.write(i, 0);
	}
}

int MOS_6502::cyclesDone() {
	return cycles;
}

void MOS_6502::resetCycles() {
	cycles = 0;
}

uint8_t MOS_6502::read(uint16_t addr) {
	return c_bus.read(addr);
}

void MOS_6502::write(uint16_t addr, uint8_t data) {
	c_bus.write(addr, data);
}

void MOS_6502::IRQ() {
	stackPush(PC >> 8);
	stackPush(PC);
	stackPush(P | (1 << 5));
	reset_interrupt_disable();
	PC = (read(0xFFFF) << 8) | read(0xFFFE);
}

void MOS_6502::NMI() {
	stackPush(PC >> 8);
	stackPush(PC);
	stackPush(P | (1 << 5));
	reset_interrupt_disable();
	PC = (read(0xFFFB) << 8) | read(0xFFFA);
}
uint8_t buffer8 = 0;
uint16_t buffer16 = 0;
uint32_t buffer32 = 0;

void MOS_6502::execute() {
	uint8_t instruction = read(PC);
	//cout << hex << setw(4) << setfill('0') << (int)PC << "  " << (int)instruction << "  A:" << (int)A << " X:" << (int)X << " Y:" << (int)Y << " P:" << (int)P << " SP:" << (int)S;
	//cout << dec << " CYC:" << cycles << endl;
	switch (instruction) {
	//Official Opcodes
	case 0x00: BRK(); break;
	case 0x01: ORA(instruction); break;
	case 0x05: ORA(instruction); break;
	case 0x06: ASL(instruction); break;
	case 0x08: PHP(); break;
	case 0x09: ORA(instruction); break;
	case 0x0A: ASL(instruction); break;
	case 0x0D: ORA(instruction); break;
	case 0x0E: ASL(instruction); break;

	case 0x10: BPL(); break;
	case 0x11: ORA(instruction); break;
	case 0x15: ORA(instruction); break;
	case 0x16: ASL(instruction); break;
	case 0x18: CLC(); break;
	case 0x19: ORA(instruction); break;
	case 0x1D: ORA(instruction); break;
	case 0x1E: ASL(instruction); break;

	case 0x20: JSR(); break;
	case 0x21: AND(instruction); break;
	case 0x24: BIT(instruction); break;
	case 0x25: AND(instruction); break;
	case 0x26: ROL(instruction); break;
	case 0x28: PLP(); break;
	case 0x29: AND(instruction); break;
	case 0x2A: ROL(instruction); break;
	case 0x2C: BIT(instruction); break;
	case 0x2D: AND(instruction); break;
	case 0x2E: ROL(instruction); break;

	case 0x30: BMI(); break;
	case 0x31: AND(instruction); break;
	case 0x35: AND(instruction); break;
	case 0x36: ROL(instruction); break;
	case 0x38: SEC(); break;
	case 0x39: AND(instruction); break;
	case 0x3D: AND(instruction); break;
	case 0x3E: ROL(instruction); break;

	case 0x40: RTI(); break;
	case 0x41: EOR(instruction); break;
	case 0x45: EOR(instruction); break;
	case 0x46: LSR(instruction); break;
	case 0x48: PHA(); break;
	case 0x49: EOR(instruction); break;
	case 0x4A: LSR(instruction); break;
	case 0x4C: JMP(instruction); break;
	case 0x4D: EOR(instruction); break;
	case 0x4E: LSR(instruction); break;

	case 0x50: BVC(); break;
	case 0x51: EOR(instruction); break;
	case 0x55: EOR(instruction); break;
	case 0x56: LSR(instruction); break;
	case 0x58: CLI(); break;
	case 0x59: EOR(instruction); break;
	case 0x5D: EOR(instruction); break;
	case 0x5E: LSR(instruction); break;

	case 0x60: RTS(); break;
	case 0x61: ADC(instruction); break;
	case 0x65: ADC(instruction); break;
	case 0x66: ROR(instruction); break;
	case 0x68: PLA(); break;
	case 0x69: ADC(instruction); break;
	case 0x6A: ROR(instruction); break;
	case 0x6C: JMP(instruction); break;
	case 0x6D: ADC(instruction); break;
	case 0x6E: ROR(instruction); break;

	case 0x70: BVS(); break;
	case 0x71: ADC(instruction); break;
	case 0x75: ADC(instruction); break;
	case 0x76: ROR(instruction); break;
	case 0x78: SEI(); break;
	case 0x79: ADC(instruction); break;
	case 0x7D: ADC(instruction); break;
	case 0x7E: ROR(instruction); break;

	case 0x81: STA(instruction); break;
	case 0x84: STY(instruction); break;
	case 0x85: STA(instruction); break;
	case 0x86: STX(instruction); break;
	case 0x88: DEY(); break;
	case 0x8A: TXA(); break;
	case 0x8C: STY(instruction); break;
	case 0x8D: STA(instruction); break;
	case 0x8E: STX(instruction); break;

	case 0x90: BCC(); break;
	case 0x91: STA(instruction); break;
	case 0x94: STY(instruction); break;
	case 0x95: STA(instruction); break;
	case 0x96: STX(instruction); break;
	case 0x98: TYA(); break;
	case 0x99: STA(instruction); break;
	case 0x9A: TXS(); break;
	case 0x9D: STA(instruction); break;

	case 0xA0: LDY(instruction); break;
	case 0xA1: LDA(instruction); break;
	case 0xA2: LDX(instruction); break;
	case 0xA4: LDY(instruction); break;
	case 0xA5: LDA(instruction); break;
	case 0xA6: LDX(instruction); break;
	case 0xA8: TAY(); break;
	case 0xA9: LDA(instruction); break;
	case 0xAA: TAX(); break;
	case 0xAC: LDY(instruction); break;
	case 0xAD: LDA(instruction); break;
	case 0xAE: LDX(instruction); break;

	case 0xB0: BCS(); break;
	case 0xB1: LDA(instruction); break;
	case 0xB4: LDY(instruction); break;
	case 0xB5: LDA(instruction); break;
	case 0xB6: LDX(instruction); break;
	case 0xB8: CLV(); break;
	case 0xB9: LDA(instruction); break;
	case 0xBA: TSX(); break;
	case 0xBC: LDY(instruction); break;
	case 0xBD: LDA(instruction); break;
	case 0xBE: LDX(instruction); break;

	case 0xC0: CPY(instruction); break;
	case 0xC1: CMP(instruction); break;
	case 0xC4: CPY(instruction); break;
	case 0xC5: CMP(instruction); break;
	case 0xC6: DEC(instruction); break;
	case 0xC8: INY(); break;
	case 0xC9: CMP(instruction); break;
	case 0xCA: DEX(); break;
	case 0xCC: CPY(instruction); break;
	case 0xCD: CMP(instruction); break;
	case 0xCE: DEC(instruction); break;

	case 0xD0: BNE(); break;
	case 0xD1: CMP(instruction); break;
	case 0xD5: CMP(instruction); break;
	case 0xD6: DEC(instruction); break;
	case 0xD8: CLD(); break;
	case 0xD9: CMP(instruction); break;
	case 0xDD: CMP(instruction); break;
	case 0xDE: DEC(instruction); break;

	case 0xE0: CPX(instruction); break;
	case 0xE1: SBC(instruction); break;
	case 0xE4: CPX(instruction); break;
	case 0xE5: SBC(instruction); break;
	case 0xE6: INC(instruction); break;
	case 0xE8: INX(); break;
	case 0xE9: SBC(instruction); break;
	case 0xEA: NOP(); break;
	case 0xEC: CPX(instruction); break;
	case 0xED: SBC(instruction); break;
	case 0xEE: INC(instruction); break;

	case 0xF0: BEQ(); break;
	case 0xF1: SBC(instruction); break;
	case 0xF5: SBC(instruction); break;
	case 0xF6: INC(instruction); break;
	case 0xF8: SED(); break;
	case 0xF9: SBC(instruction); break;
	case 0xFD: SBC(instruction); break;
	case 0xFE: INC(instruction); break;

	//Unofficial Opcodes
	case 0x04: DOP(instruction); break;
	case 0x03: SLO(instruction); break;
	case 0x07: SLO(instruction); break;
	case 0x0C: TOP(instruction); break;
	case 0x0F: SLO(instruction); break;

	case 0x13: SLO(instruction); break;
	case 0x14: DOP(instruction); break;
	case 0x17: SLO(instruction); break;
	case 0x1A: NOP(); break;
	case 0x1B: SLO(instruction); break;
	case 0x1C: TOP(instruction); break;
	case 0x1F: SLO(instruction); break;

	case 0x23: RLA(instruction); break;
	case 0x27: RLA(instruction); break;
	case 0x2F: RLA(instruction); break;

	case 0x33: RLA(instruction); break;
	case 0x34: DOP(instruction); break;
	case 0x37: RLA(instruction); break;
	case 0x3A: NOP(); break;
	case 0x3B: RLA(instruction); break;
	case 0x3C: TOP(instruction); break;
	case 0x3F: RLA(instruction); break;

	case 0x43: SRE(instruction); break;
	case 0x44: DOP(instruction); break;
	case 0x47: SRE(instruction); break;
	case 0x4F: SRE(instruction); break;

	case 0x53: SRE(instruction); break;
	case 0x54: DOP(instruction); break;
	case 0x57: SRE(instruction); break;
	case 0x5A: NOP(); break;
	case 0x5B: SRE(instruction); break;
	case 0x5C: TOP(instruction); break;
	case 0x5F: SRE(instruction); break;

	case 0x63: RRA(instruction); break;
	case 0x64: DOP(instruction); break;
	case 0x67: RRA(instruction); break;
	case 0x6F: RRA(instruction); break;

	case 0x74: DOP(instruction); break;
	case 0x73: RRA(instruction); break;
	case 0x77: RRA(instruction); break;
	case 0x7A: NOP(); break;
	case 0x7B: RRA(instruction); break;
	case 0x7C: TOP(instruction); break;
	case 0x7F: RRA(instruction); break;

	case 0x80: DOP(instruction); break;
	case 0x82: DOP(instruction); break;
	case 0x83: SAX(instruction); break;
	case 0x87: SAX(instruction); break;
	case 0x89: DOP(instruction); break;
	case 0x8F: SAX(instruction); break;

	case 0x97: SAX(instruction); break;

	case 0xA3: LAX(instruction); break;
	case 0xA7: LAX(instruction); break;
	case 0xAF: LAX(instruction); break;

	case 0xB3: LAX(instruction); break;
	case 0xB7: LAX(instruction); break;
	case 0xBF: LAX(instruction); break;

	case 0xC2: DOP(instruction); break;
	case 0xC3: DCP(instruction); break;
	case 0xC7: DCP(instruction); break;
	case 0xCF: DCP(instruction); break;

	case 0xD3: DCP(instruction); break;
	case 0xD4: DOP(instruction); break;
	case 0xD7: DCP(instruction); break;
	case 0xDA: NOP(); break;
	case 0xDB: DCP(instruction); break;
	case 0xDC: TOP(instruction); break;
	case 0xDF: DCP(instruction); break;

	case 0xE2: DOP(instruction); break;
	case 0xE3: ISB(instruction); break;
	case 0xE7: ISB(instruction); break;
	case 0xEB: SBC(instruction); break; 
	case 0xEF: ISB(instruction); break;

	case 0xF3: ISB(instruction); break;
	case 0xF4: DOP(instruction); break;
	case 0xF7: ISB(instruction); break;
	case 0xFA: NOP(); break;
	case 0xFB: ISB(instruction); break;
	case 0xFC: TOP(instruction); break;
	case 0xFF: ISB(instruction); break;

	default: 
		cout << hex << setw(4) << setfill('0') << (int)PC << "  " << (int)instruction << "  A:" << (int)A << " X:" << (int)X << " Y:" << (int)Y << " P:" << (int)P << " SP:" << (int)S;
		cout << dec << " CYC:" << cycles << endl;
		cout << "uknown instruction" << endl;
	}
}

//instructions
void MOS_6502::ADC(uint8_t instruction) {
	uint8_t m = 0;
	switch (instruction) { //choose correct addressing mode
	case 0x69: //Immediate
		m = read(PC + 1);
		buffer16 = A + m + C;
		PC += 2;
		clkCycles 2;
		break;
	case 0x65: //Zero Page
		m = read(read(PC + 1));
		buffer16 = A + m + C;
		PC += 2;
		clkCycles 3;
		break;
	case 0x75: //Zero Page, X
		buffer16 = read(PC + 1) + X;
		if (((read(PC + 1) & 0xFF) + X) > 255) {
			buffer16 -= 0x0100;
		}
		m = read(buffer16);
		buffer16 = A + m + C;
		PC += 2;
		clkCycles 4;
		break;
	case 0x6D: //Absolute
		buffer16 = (read(PC + 2) << 8) | read(PC + 1);
		m = read(buffer16);
		buffer16 = A + m + C;
		PC += 3;
		clkCycles 4;
		break;
	case 0x7D: //Absolute, X
		buffer16 = (read(PC + 2) << 8) | read(PC + 1);
		if (((buffer16 & 0xFF) + X) > 255) {
			clkCycles 1;
		}
		m = read(buffer16 + X);
		buffer16 = A + m + C;
		PC += 3;
		clkCycles 4;
		break;
	case 0x79: //Absolute, Y
		buffer16 = (read(PC + 2) << 8) | read(PC + 1);
		if (((buffer16 & 0xFF) + Y) > 255) {
			clkCycles 1;
		}
		m = read(buffer16 + Y);
		buffer16 = A + m + C;
		PC += 3;
		clkCycles 4;
		break;
	case 0x61: //(Indirect, X)
		buffer8 = read(PC + 1) + X;
		buffer16 = (read(buffer8 + 1) << 8) | read(buffer8);
		m = read(buffer16);
		buffer16 = A + m + C;
		PC += 2;
		clkCycles 6;
		break;
	case 0x71: //(Indirect), Y
		buffer8 = read(PC + 1);
		buffer16 = (read(buffer8 + 1) << 8) | read(buffer8);
		if (((buffer16 & 0xFF) + Y) > 255) {
			clkCycles 1;
		}
		m = read(buffer16 + Y);
		buffer16 = A + m + C;
		PC += 2;
		clkCycles 5;
		break;
	}
	handle_carry(buffer16);
	handle_zero(buffer16);
	if (!((A ^ m) & 0x80) && ((A ^ buffer16) & 0x80)) set_overflow(); else reset_overflow();
	handle_negative(buffer16);
	A = buffer16;
}
void MOS_6502::AND(uint8_t instruction) {
	switch (instruction) {
	case 0x29: //Immediate
		buffer16 = A & read(PC + 1);
		PC += 2;
		clkCycles 2;
		break;
	case 0x25: //Zero Page
		buffer16 = A & read(read(PC + 1));
		PC += 2;
		clkCycles 3;
		break;
	case 0x35: //Zero Page, X
		buffer16 = read(PC + 1) + X;
		if (((read(PC + 1) & 0xFF) + X) > 255) {
			buffer16 -= 0x0100;
		}
		buffer16 = A & read(buffer16);
		PC += 2;
		clkCycles 4;
		break;
	case 0x2D: //Absolute
		buffer16 = (read(PC + 2) << 8) | read(PC + 1);
		buffer16 = A & read(buffer16);
		PC += 3;
		clkCycles 4;
		break;
	case 0x3D: //Absolute, X
		buffer16 = (read(PC + 2) << 8) | read(PC + 1);
		if (((buffer16 & 0xFF) + X) > 255) {
			clkCycles 1;
		}
		buffer16 = A & read(buffer16 + X);
		PC += 3;
		clkCycles 4;
		break;
	case 0x39: //Absolute, Y
		buffer16 = (read(PC + 2) << 8) | read(PC + 1);
		if (((buffer16 & 0xFF) + Y) > 255) {
			clkCycles 1;
		}
		buffer16 = A & read(buffer16 + Y);
		PC += 3;
		clkCycles 4;
		break;
	case 0x21: //(Indirect, X)
		buffer8 = read(PC + 1) + X;
		buffer16 = (read(buffer8 + 1) << 8) | read(buffer8);
		buffer16 = A & read(buffer16);
		PC += 2;
		clkCycles 6;
		break;
	case 0x31: //(Indirect), Y
		buffer8 = read(PC + 1);
		buffer16 = (read(buffer8 + 1) << 8) | read(buffer8);
		if (((buffer16 & 0xFF) + Y) > 255) {
			clkCycles 1;
		}
		buffer16 = A & read(buffer16 + Y);
		PC += 2;
		clkCycles 5;
		break;
	}
	handle_zero(buffer16);
	handle_negative(buffer16);
	A = buffer16;
}
void MOS_6502::ASL(uint8_t instruction) {
	buffer16 = A;
	switch (instruction) {
	case 0x0A: //Accumulator
		buffer16 = buffer16 << 1;
		A = buffer16;
		PC += 1;
		clkCycles 2;
		break;
	case 0x06: //Zero Page
		buffer16 = read(read(PC + 1)) << 1;
		write(read(PC + 1), buffer16);
		PC += 2;
		clkCycles 5;
		break;
	case 0x16: //Zero Page, X
		buffer32 = read(PC + 1) + X;
		if (((read(PC + 1) & 0xFF) + X) > 255) {
			buffer32 -= 0x0100;
		}
		buffer16 = read(buffer32) << 1;
		write(buffer32, buffer16);
		PC += 2;
		clkCycles 6;
		break;
	case 0x0E: //Absolute
		buffer32 = ((read(PC + 2) << 8) | read(PC + 1));
		buffer16 = read(buffer32) << 1;
		write(buffer32, buffer16);
		PC += 3;
		clkCycles 6;
		break;
	case 0x1E: //Absolute, X
		buffer32 = ((read(PC + 2) << 8) | read(PC + 1));
		buffer16 = read(buffer32 + X) << 1;
		write(buffer32 + X, buffer16);
		PC += 3;
		clkCycles 7;
		break;
	}
	handle_carry(buffer16);
	handle_zero(buffer16);
	handle_negative(buffer16);
}
void MOS_6502::BCC() {
	if (!C) {
		buffer16 = (PC & 0xFF) + ((read(PC + 1) + 2) & 0xFF);
		PC = PC + (int8_t)read(PC + 1) + 2;
		clkCycles 1;
	}
	else {
		PC += 2;
	}
	clkCycles 2;
}
void MOS_6502::BCS() {
	if (C) {
		buffer16 = (PC & 0xFF) + ((read(PC + 1) + 2) & 0xFF);
		PC = PC + (int8_t)read(PC + 1) + 2;
		clkCycles 1;
	}
	else {
		PC += 2;
	}
	clkCycles 2;
}
void MOS_6502::BEQ() {
	if (Z) {
		buffer16 = (PC & 0xFF) + ((read(PC + 1) + 2) & 0xFF);
		PC = PC + (int8_t)read(PC + 1) + 2;
		clkCycles 1;
	}
	else {
		PC += 2;
	}
	clkCycles 2;
}
void MOS_6502::BIT(uint8_t instruction) {
	switch (instruction) {
	case 0x24: //Zero Page
		buffer16 = read(read(PC + 1));
		handle_overflow(buffer16);
		handle_negative(buffer16);
		buffer16 = A & buffer16;
		PC += 2;
		clkCycles 3;
		break;
	case 0x2C: //Absolute
		buffer16 = ((read(PC + 2) << 8) | read(PC + 1));
		buffer16 = read(buffer16);
		handle_overflow(buffer16);
		handle_negative(buffer16);
		buffer16 = A & buffer16;
		PC += 3;
		clkCycles 4;
		break;
	}
	handle_zero(buffer16);
}
void MOS_6502::BMI() {
	if (N) {
		buffer16 = (PC & 0xFF) + ((read(PC + 1) + 2) & 0xFF);
		PC = PC + (int8_t)read(PC + 1) + 2;
		clkCycles 1;
	}
	else {
		PC += 2;
	}
	clkCycles 2;
}
void MOS_6502::BNE() {
	if (!Z) {
		buffer16 = (PC & 0xFF) + ((read(PC + 1) + 2) & 0xFF);
		PC = PC + (int8_t)read(PC + 1) + 2;
		clkCycles 1;
	}
	else {
		PC += 2;
	}
	clkCycles 2;
}
void MOS_6502::BPL() {
	if (!N) {
		PC = PC + (int8_t)read(PC + 1) + 2;
		clkCycles 1;
	}
	else {
		PC += 2;
	}
	clkCycles 2;
}
void MOS_6502::BRK() {
	stackPush(PC >> 8);
	stackPush(PC);
	stackPush(P | (3 << 4));
	reset_interrupt_disable();
	PC = (read(0xFFFF) << 8) | read(0xFFFE);
}
void MOS_6502::BVC() {
	if (!V) {
		buffer16 = (PC & 0xFF) + (((uint16_t)read(PC + 1) + 2) & 0xFF);
		PC = PC + (int8_t)read(PC + 1) + 2;
		clkCycles 1;
	}
	else {
		PC += 2;
	}
	clkCycles 2;
}
void MOS_6502::BVS() {
	if (V) {
		buffer16 = (PC & 0xFF) + (((uint16_t)read(PC + 1) + 2) & 0xFF);
		PC = PC + (int8_t)read(PC + 1) + 2;
		clkCycles 1;
	}
	else {
		PC += 2;
	}
	clkCycles 2;
}
void MOS_6502::CLC() {
	reset_carry();
	PC += 1;
	clkCycles 2;
}
void MOS_6502::CLD() {
	reset_decimal();
	PC += 1;
	clkCycles 2;
}
void MOS_6502::CLI() {
	reset_interrupt_disable();
	PC += 1;
	clkCycles 2;
}
void MOS_6502::CLV() {
	reset_overflow();
	PC += 1;
	clkCycles 2;
}
void MOS_6502::CMP(uint8_t instruction) {
	switch (instruction) {
	case 0xC9: //Immediate
		buffer16 = A - read(PC + 1);
		if (A >= read(PC + 1)) set_carry(); else reset_carry();
		PC += 2;
		clkCycles 2;
		break;
	case 0xC5: //Zero Page
		buffer16 = A - read(read(PC + 1));
		if (A >= read(read(PC + 1))) set_carry(); else reset_carry();
		PC += 2;
		clkCycles 3;
		break;
	case 0xD5: //Zero Page, X
		buffer32 = read(PC + 1) + X;
		if (((read(PC + 1) & 0xFF) + X) > 255) {
			buffer32 -= 0x0100;
		}
		buffer16 = A - read(buffer32);
		if (A >= read(buffer32)) set_carry(); else reset_carry();
		PC += 2;
		clkCycles 4;
		break;
	case 0xCD: //Absolute
		buffer16 = (read(PC + 2) << 8) | read(PC + 1);
		if (A >= read(buffer16)) set_carry(); else reset_carry();
		buffer16 = A - read(buffer16);
		PC += 3;
		clkCycles 4;
		break;
	case 0xDD: //Absolute, X
		buffer16 = (read(PC + 2) << 8) | read(PC + 1);
		if (A >= read(buffer16 + X)) set_carry(); else reset_carry();
		if (((buffer16 & 0xFF) + X) > 255) {
			clkCycles 1;
		}
		buffer16 = A - read((uint16_t)(buffer16 + X));
		PC += 3;
		clkCycles 4;
		break;
	case 0xD9: //Absolute, Y
		buffer16 = (read(PC + 2) << 8) | read(PC + 1);
		if (A >= read(buffer16 + Y)) set_carry(); else reset_carry();
		if (((buffer16 & 0xFF) + Y) > 255) {
			clkCycles 1;
		}
		buffer16 = A - read((uint16_t)(buffer16 + Y));
		PC += 3;
		clkCycles 4;
		break;
	case 0xC1: //(Indirect, X)
		buffer8 = read(PC + 1) + X;
		buffer16 = (read((uint8_t)(buffer8 + 1)) << 8) | read(buffer8);
		if (A >= read(buffer16)) set_carry(); else reset_carry();
		buffer16 = A - read(buffer16);
		PC += 2;
		clkCycles 6;
		break;
	case 0xD1: //(Indirect), Y
		buffer8 = read(PC + 1);
		buffer16 = (read((uint8_t)(buffer8 + 1)) << 8) | read(buffer8);
		if (A >= read(buffer16 + Y)) set_carry(); else reset_carry();
		if (((buffer16 & 0xFF) + Y) > 255) {
			clkCycles 1;
		}
		buffer16 = A - read((uint16_t)(buffer16 + Y));
		PC += 2;
		clkCycles 5;
		break;
	}
	handle_zero(buffer16);
	handle_negative(buffer16);
}
void MOS_6502::CPX(uint8_t instruction) {
	switch (instruction) {
	case 0xE0: //Immediate
		if (X >= read(PC + 1)) set_carry(); else reset_carry();
		buffer16 = X - read(PC + 1);
		PC += 2;
		clkCycles 2;
		break;
	case 0xE4: //Zero Page
		if (X >= read(read(PC + 1))) set_carry(); else reset_carry();
		buffer16 = X - read(read(PC + 1));
		PC += 2;
		clkCycles 3;
		break;
	case 0xEC: //Absolute
		buffer16 = ((uint16_t)read(PC + 2) << 8) | read(PC + 1);
		if (X >= read(buffer16)) set_carry(); else reset_carry();
		buffer16 = X - read(buffer16);
		PC += 3;
		clkCycles 4;
		break;
	}
	handle_zero(buffer16);
	handle_negative(buffer16);
}
void MOS_6502::CPY(uint8_t instruction) {
	switch (instruction) {
	case 0xC0: //Immediate
		if (Y >= read(PC + 1)) set_carry(); else reset_carry();
		buffer16 = Y - read(PC + 1);
		PC += 2;
		clkCycles 2;
		break;
	case 0xC4: //Zero Page
		if (Y >= read(read(PC + 1))) set_carry(); else reset_carry();
		buffer16 = Y - read(read(PC + 1));
		PC += 2;
		clkCycles 3;
		break;
	case 0xCC: //Absolute
		buffer16 = ((uint16_t)read(PC + 2) << 8) | read(PC + 1);
		if (Y >= read(buffer16)) set_carry(); else reset_carry();
		buffer16 = Y - read(buffer16);
		PC += 3;
		clkCycles 4;
		break;
	}
	handle_zero(buffer16);
	handle_negative(buffer16);
}
void MOS_6502::DEC(uint8_t instruction) {
	switch (instruction) {
	case 0xC6: //Zero Page
		buffer16 = read(read(PC + 1)) - 1;
		write(read(PC + 1), buffer16);
		PC += 2;
		clkCycles 5;
		break;
	case 0xD6: //Zero Page, X
		buffer32 = read(PC + 1) + X;
		if (((read(PC + 1) & 0xFF) + X) > 255) {
			buffer32 -= 0x0100;
		}
		buffer16 = read(buffer32) - 1;
		write(buffer32, buffer16);
		PC += 2;
		clkCycles 6;
		break;
	case 0xCE: //Absolute
		buffer32 = (read(PC + 2) << 8) | read(PC + 1);
		buffer16 = read(buffer32) - 1;
		write(buffer32, buffer16);
		PC += 3;
		clkCycles 6;
		break;
	case 0xDE: //Absolute, X
		buffer32 = (read(PC + 2) << 8) | read(PC + 1);
		buffer16 = read(buffer32 + X) - 1;
		write(buffer32 + X, buffer16);
		PC += 3;
		clkCycles 7;
		break;
	}
	handle_zero(buffer16);
	handle_negative(buffer16);
}
void MOS_6502::DEX() {
	X = X - 1;
	handle_zero(X);
	handle_negative(X);
	PC += 1;
	clkCycles 2;
}
void MOS_6502::DEY() {
	Y = Y - 1;
	handle_zero(Y);
	handle_negative(Y);
	PC += 1;
	clkCycles 2;
}
void MOS_6502::EOR(uint8_t instruction) {
	switch (instruction) {
	case 0x49: //Immediate
		buffer16 = A ^ read(PC + 1);
		PC += 2;
		clkCycles 2;
		break;
	case 0x45: //Zero Page
		buffer16 = A ^ read(read(PC + 1));
		PC += 2;
		clkCycles 3;
		break;
	case 0x55: //Zero Page, X
		buffer16 = read(PC + 1) + X;
		if (((read(PC + 1) & 0xFF) + X) > 255) {
			buffer16 -= 0x0100;
		}
		buffer16 = A ^ read(buffer16);
		PC += 2;
		clkCycles 4;
		break;
	case 0x4D: //Absolute
		buffer16 = (read(PC + 2) << 8) | read(PC + 1);
		buffer16 = A ^ read(buffer16);
		PC += 3;
		clkCycles 4;
		break;
	case 0x5D: //Absolute, X
		buffer16 = (read(PC + 2) << 8) | read(PC + 1);
		if (((buffer16 & 0xFF) + X) > 255) {
			clkCycles 1;
		}
		buffer16 = A ^ read((uint16_t)(buffer16 + X));
		PC += 3;
		clkCycles 4;
		break;
	case 0x59: //Absolute, Y
		buffer16 = (read(PC + 2) << 8) | read(PC + 1);
		if (((buffer16 & 0xFF) + Y) > 255) {
			clkCycles 1;
		}
		buffer16 = A ^ read((uint16_t)(buffer16 + Y));
		PC += 3;
		clkCycles 4;
		break;
	case 0x41: //(Indirect, X)
		buffer8 = read(PC + 1) + X;
		buffer16 = (read((uint8_t)(buffer8 + 1)) << 8) | read(buffer8);
		buffer16 = A ^ read(buffer16);
		PC += 2;
		clkCycles 6;
		break;
	case 0x51: //(Indirect), Y
		buffer8 = read(PC + 1);
		buffer16 = (read((uint8_t)(buffer8 + 1)) << 8) | read(buffer8);
		if (((buffer16 & 0xFF) + Y) > 255) {
			clkCycles 1;
		}
		buffer16 = A ^ read((uint16_t)(buffer16 + Y));
		PC += 2;
		clkCycles 5;
		break;
	}
	handle_zero(buffer16);
	handle_negative(buffer16);
	A = buffer16;
}
void MOS_6502::INC(uint8_t instruction) {
	switch (instruction) {
	case 0xE6: //Zero Page
		buffer16 = read(read(PC + 1)) + 1;
		write(read(PC + 1), buffer16);
		PC += 2;
		clkCycles 5;
		break;
	case 0xF6: //Zero Page, X
		buffer32 = read(PC + 1) + X;
		if (((read(PC + 1) & 0xFF) + X) > 255) {
			buffer32 -= 0x0100;
		}
		buffer16 = read(buffer32) + 1;
		write(buffer32, buffer16);
		PC += 2;
		clkCycles 6;
		break;
	case 0xEE: //Absolute
		buffer32 = (read(PC + 2) << 8) | read(PC + 1);
		buffer16 = read(buffer32) + 1;
		write(buffer32, buffer16);
		PC += 3;
		clkCycles 6;
		break;
	case 0xFE: //Absolute, X
		buffer32 = (read(PC + 2) << 8) | read(PC + 1);
		buffer16 = read(buffer32 + X) + 1;
		write((uint16_t)(buffer32 + X), buffer16);
		PC += 3;
		clkCycles 7;
		break;
	}
	handle_zero(buffer16);
	handle_negative(buffer16);
}
void MOS_6502::INX() {
	X = X + 1;
	handle_zero(X);
	handle_negative(X);
	PC += 1;
	clkCycles 2;
}
void MOS_6502::INY() {
	Y = Y + 1;
	handle_zero(Y);
	handle_negative(Y);
	PC += 1;
	clkCycles 2;
	clkCycles 2;
}
void MOS_6502::JMP(uint8_t instruction) {
	switch (instruction) {
	case 0x4C: //Absolute
		buffer16 = (read(PC + 2) << 8) | read(PC + 1);
		clkCycles 3;
		break;
	case 0x6C: //Indirect
		buffer32 = (read(PC + 2) << 8) | read(PC + 1);
		buffer16 = buffer32;
		if (((buffer32 & 0xFF) + 1) > 0xFF) {
			buffer32 -= 0x100;
		}
		buffer16 = (read(buffer32 + 1) << 8) | read(buffer16);
		clkCycles 5;
		break;
	}
	PC = buffer16;
}
void MOS_6502::JSR() {
	buffer16 = (read(PC + 2) << 8) | read(PC + 1);
	PC += 2;
	stackPush(PC >> 8);
	stackPush(PC);
	PC = buffer16;
	clkCycles 6;
}
void MOS_6502::LDA(uint8_t instruction) {
	switch (instruction) {
	case 0xA9: //Immediate
		buffer16 = read(PC + 1);
		PC += 2;
		clkCycles 2;
		break;
	case 0xA5: //Zero Page
		buffer16 = read(read(PC + 1));
		PC += 2;
		clkCycles 3;
		break;
	case 0xB5: //Zero Page, X
		buffer16 = read(PC + 1) + X;
		if (((read(PC + 1) & 0xFF) + X) > 255) {
			buffer16 -= 0x0100;
		}
		buffer16 = read(buffer16);
		PC += 2;
		clkCycles 4;
		break;
	case 0xAD: //Absolute
		buffer16 = (read(PC + 2) << 8) | read(PC + 1);
		buffer16 = read(buffer16);
		PC += 3;
		clkCycles 4;
		break;
	case 0xBD: //Absolute, X
		buffer16 = (read(PC + 2) << 8) | read(PC + 1);
		if (((buffer16 & 0xFF) + X) > 255) {
			clkCycles 1;
		}
		buffer16 = read((uint16_t)(buffer16 + X));
		PC += 3;
		clkCycles 4;
		break;
	case 0xB9: //Absolute, Y
		buffer16 = (read(PC + 2) << 8) | read(PC + 1);
		if (((buffer16 & 0xFF) + Y) > 255) {
			clkCycles 1;
		}
		buffer16 = read((uint16_t)(buffer16 + Y));
		PC += 3;
		clkCycles 4;
		break;
	case 0xA1: //(Indirect, X)
		buffer8 = read(PC + 1) + X;
		buffer16 = (read((uint8_t)(buffer8 + 1)) << 8) | read(buffer8);
		buffer16 = read(buffer16);
		PC += 2;
		clkCycles 6;
		break;
	case 0xB1: //(Indirect), Y
		buffer8 = read(PC + 1);
		buffer16 = (read((uint8_t)(buffer8 + 1)) << 8) | read(buffer8);
		if (((buffer16 & 0xFF) + Y) > 255) {
			clkCycles 1;
		}
		buffer16 = read((uint16_t)(buffer16 + Y));
		PC += 2;
		clkCycles 5;
		break;
	}
	handle_zero(buffer16);
	handle_negative(buffer16);
	A = buffer16;
}
void MOS_6502::LDX(uint8_t instruction) {
	switch (instruction) {
	case 0xA2: //Immediate
		buffer16 = read(PC + 1);
		PC += 2;
		clkCycles 2;
		break;
	case 0xA6: //Zero Page
		buffer16 = read(read(PC + 1));
		PC += 2;
		clkCycles 3;
		break;
	case 0xB6: //Zero Page, Y
		buffer16 = read(PC + 1) + Y;
		if (((read(PC + 1) & 0xFF) + Y) > 255) {
			buffer16 -= 0x0100;
		}
		buffer16 = read(buffer16);
		PC += 2;
		clkCycles 4;
		break;
	case 0xAE: //Absolute
		buffer16 = ((uint16_t)read(PC + 2) << 8) | read(PC + 1);
		buffer16 = read(buffer16);
		PC += 3;
		clkCycles 4;
		break;
	case 0xBE: //Absolute, Y
		buffer16 = ((uint16_t)read(PC + 2) << 8) | read(PC + 1);
		if (((buffer16 & 0xFF) + Y) > 255) {
			clkCycles 1;
		}
		buffer16 = read((uint16_t)(buffer16 + Y));
		PC += 3;
		clkCycles 4;
		break;
	}
	handle_zero(buffer16);
	handle_negative(buffer16);
	X = buffer16;
}
void MOS_6502::LDY(uint8_t instruction) {
	switch (instruction) {
	case 0xA0: //Immediate
		buffer16 = read(PC + 1);
		PC += 2;
		clkCycles 2;
		break;
	case 0xA4: //Zero Page
		buffer16 = read(read(PC + 1));
		PC += 2;
		clkCycles 3;
		break;
	case 0xB4: //Zero Page, X
		buffer16 = read(PC + 1) + X;
		if (((read(PC + 1) & 0xFF) + X) > 255) {
			buffer16 -= 0x0100;
		}
		buffer16 = read(buffer16);
		PC += 2;
		clkCycles 4;
		break;
	case 0xAC: //Absolute
		buffer16 = (read(PC + 2) << 8) | read(PC + 1);
		buffer16 = read(buffer16);
		PC += 3;
		clkCycles 4;
		break;
	case 0xBC: //Absolute, X
		buffer16 = (read(PC + 2) << 8) | read(PC + 1);
		if (((buffer16 & 0xFF) + X) > 255) {
			clkCycles 1;
		}
		buffer16 = read((uint16_t)(buffer16 + X));
		PC += 3;
		clkCycles 4;
		break;
	}
	handle_zero(buffer16);
	handle_negative(buffer16);
	Y = buffer16;
}
void MOS_6502::LSR(uint8_t instruction) {
	switch (instruction) {
	case 0x4A: //Accumulator
		if (A & 0x01) set_carry(); else reset_carry();
		buffer16 = A >> 1;
		A = buffer16;
		PC += 1;
		clkCycles 2;
		break;
	case 0x46: //Zero Page
		buffer16 = read(read(PC + 1));
		if (buffer16 & 0x01) set_carry(); else reset_carry();
		buffer16 = buffer16 >> 1;
		write(read(PC + 1), buffer16);
		PC += 2;
		clkCycles 5;
		break;
	case 0x56: //Zero Page, X
		buffer32 = read(PC + 1) + X;
		if (((read(PC + 1) & 0xFF) + X) > 255) {
			buffer32 -= 0x0100;
		}
		buffer16 = read(buffer32);
		if (buffer16 & 0x01) set_carry(); else reset_carry();
		buffer16 = buffer16 >> 1;
		write(buffer32, buffer16);
		PC += 2;
		clkCycles 6;
		break;
	case 0x4E: //Absolute
		buffer32 = ((read(PC + 2) << 8) | read(PC + 1));
		if (read(buffer32) & 0x01) set_carry(); else reset_carry();
		buffer16 = read(buffer32) >> 1;
		write(buffer32, buffer16);
		PC += 3;
		clkCycles 6;
		break;
	case 0x5E: //Absolute, X
		buffer32 = ((read(PC + 2) << 8) | read(PC + 1));
		if (read(buffer32 + X) & 0x01) set_carry(); else reset_carry();
		buffer16 = read((uint16_t)(buffer32 + X)) >> 1;
		write((uint16_t)(buffer32 + X), buffer16);
		PC += 3;
		clkCycles 7;
		break;
	}
	handle_zero(buffer16);
	handle_negative(buffer16);
}
void MOS_6502::NOP(){
	PC += 1;
	clkCycles 2;
}
void MOS_6502::ORA(uint8_t instruction) {
	switch (instruction) {
	case 0x09: //Immediate
		buffer16 = A | read(PC + 1);
		PC += 2;
		clkCycles 2;
		break;
	case 0x05: //Zero Page
		buffer16 = A | read(read(PC + 1));
		PC += 2;
		clkCycles 3;
		break;
	case 0x15: //Zero Page, X
		buffer16 = read(PC + 1) + X;
		if (((read(PC + 1) & 0xFF) + X) > 255) {
			buffer16 -= 0x0100;
		}
		buffer16 = A | read(buffer16);
		PC += 2;
		clkCycles 4;
		break;
	case 0x0D: //Absolute
		buffer16 = (read(PC + 2) << 8) | read(PC + 1);
		buffer16 = A | read(buffer16);
		PC += 3;
		clkCycles 4;
		break;
	case 0x1D: //Absolute, X
		buffer16 = (read(PC + 2) << 8) | read(PC + 1);
		if (((buffer16 & 0xFF) + X) > 255) {
			clkCycles 1;
		}
		buffer16 = A | read((uint16_t)(buffer16 + X));
		PC += 3;
		clkCycles 4;
		break;
	case 0x19: //Absolute, Y
		buffer16 = (read(PC + 2) << 8) | read(PC + 1);
		if (((buffer16 & 0xFF) + Y) > 255) {
			clkCycles 1;
		}
		buffer16 = A | read((uint16_t)(buffer16 + Y));
		PC += 3;
		clkCycles 4;
		break;
	case 0x01: //(Indirect, X)
		buffer8 = read(PC + 1) + X;
		buffer16 = (read((uint8_t)(buffer8 + 1)) << 8) | read(buffer8);
		buffer16 = A | read(buffer16);
		PC += 2;
		clkCycles 6;
		break;
	case 0x11: //(Indirect), Y
		buffer8 = read(PC + 1);
		buffer16 = (read((uint8_t)(buffer8 + 1)) << 8) | read(buffer8);
		if (((buffer16 & 0xFF) + Y) > 255) {
			clkCycles 1;
		}
		buffer16 = A | read((uint16_t)(buffer16 + Y));
		PC += 2;
		clkCycles 5;
		break;
	}
	handle_zero(buffer16);
	handle_negative(buffer16);
	A = buffer16;
}
void MOS_6502::PHA() {
	stackPush(A);
	PC += 1;
	clkCycles 3;
}
void MOS_6502::PHP() {
	buffer8 = P | (3 << 4);
	stackPush(buffer8);
	PC += 1;
	clkCycles 3;
}
void MOS_6502::PLA() {
	A = stackPop();
	handle_zero(A);
	handle_negative(A);
	PC += 1;
	clkCycles 4;
}
void MOS_6502::PLP() {
	P = stackPop();
	reset_break();
	P |= (1 << 5);
	PC += 1;
	clkCycles 4;
}
void MOS_6502::ROL(uint8_t instruction) {
	int temp = C;
	switch (instruction) {
	case 0x2A: //Accumulator
		buffer16 = A;
		buffer16 = buffer16 << 1;
		buffer16 = buffer16 | temp;
		A = buffer16;
		PC += 1;
		clkCycles 2;
		break;
	case 0x26: //Zero Page
		buffer16 = read(read(PC + 1)) << 1;
		buffer16 = buffer16 | temp;
		write(read(PC + 1), buffer16);
		PC += 2;
		clkCycles 5;
		break;
	case 0x36: //Zero Page, X
		buffer32 = read(PC + 1) + X;
		if (((read(PC + 1) & 0xFF) + X) > 255) {
			buffer32 -= 0x0100;
		}
		buffer16 = read(buffer32) << 1;
		buffer16 = buffer16 | temp;
		write(buffer32, buffer16);
		PC += 2;
		clkCycles 6;
		break;
	case 0x2E: //Absolute
		buffer32 = ((read(PC + 2) << 8) | read(PC + 1));
		buffer16 = read(buffer32) << 1;
		buffer16 = buffer16 | temp;
		write(buffer32, buffer16);
		PC += 3;
		clkCycles 6;
		break;
	case 0x3E: //Absolute, X
		buffer32 = ((read(PC + 2) << 8) | read(PC + 1));
		buffer16 = read((uint16_t)(buffer32 + X)) << 1;
		buffer16 = buffer16 | temp;
		write((uint16_t)(buffer32 + X), buffer16);
		PC += 3;
		clkCycles 7;
		break;
	}
	handle_carry(buffer16);
	handle_zero(buffer16);
	handle_negative(buffer16);
}
void MOS_6502::ROR(uint8_t instruction) {
	int temp = C;
	switch (instruction) {
	case 0x6A: //Accumulator
		buffer16 = A;
		if (buffer16 & 0x01) set_carry(); else reset_carry();
		buffer16 = buffer16 >> 1;
		buffer16 = buffer16 | (temp << 7);
		A = buffer16;
		PC += 1;
		clkCycles 2;
		break;
	case 0x66: //Zero Page
		buffer16 = read(read(PC + 1));
		if (buffer16 & 0x01) set_carry(); else reset_carry();
		buffer16 = buffer16 >> 1;
		buffer16 = buffer16 | (temp << 7);
		write(read(PC + 1), buffer16);
		PC += 2;
		clkCycles 5;
		break;
	case 0x76: //Zero Page, X
		buffer32 = read(PC + 1) + X;
		if (((read(PC + 1) & 0xFF) + X) > 255) {
			buffer32 -= 0x0100;
		}
		buffer16 = read(buffer32);
		if (buffer16 & 0x01) set_carry(); else reset_carry();
		buffer16 = buffer16 >> 1;
		buffer16 = buffer16 | (temp << 7);
		write(buffer32, buffer16);
		PC += 2;
		clkCycles 6;
		break;
	case 0x6E: //Absolute
		buffer32 = ((read(PC + 2) << 8) | read(PC + 1));
		if (read(buffer32) & 0x01) set_carry(); else reset_carry();
		buffer16 = read(buffer32) >> 1;
		buffer16 = buffer16 | (temp << 7);
		write(buffer32, buffer16);
		PC += 3;
		clkCycles 6;
		break;
	case 0x7E: //Absolute, X
		buffer32 = ((read(PC + 2) << 8) | read(PC + 1));
		if (read(buffer32 + X) & 0x01) set_carry(); else reset_carry();
		buffer16 = read(buffer32 + X) >> 1;
		buffer16 = buffer16 | (temp << 7);
		write(buffer32 + X, buffer16);
		PC += 3;
		clkCycles 7;
		break;
	}
	handle_zero(buffer16);
	handle_negative(buffer16);
}
void MOS_6502::RTI() {
	P = stackPop();
	P |= (1 << 5);
	buffer16 = stackPop();
	buffer16 |= (stackPop() << 8);
	PC = buffer16;
	clkCycles 6;
}
void MOS_6502::RTS() {
	buffer16 = stackPop();
	buffer16 |= (stackPop() << 8);
	PC = buffer16 + 1;
	clkCycles 6;
}
void MOS_6502::SBC(uint8_t instruction) {
	uint8_t m = 0;
	switch (instruction) {
	case 0xE9: //Immediate
		m = read(PC + 1);
		buffer16 = A - m - (1 - C);
		PC += 2;
		clkCycles 2;
		break;
	case 0xE5: //Zero Page
		m = read(read(PC + 1));
		buffer16 = A - m - (1 - C);
		PC += 2;
		clkCycles 3;
		break;
	case 0xF5: //Zero Page, X
		buffer16 = read(PC + 1) + X;
		if (((read(PC + 1) & 0xFF) + X) > 255) {
			buffer16 -= 0x0100;
		}
		m = read(buffer16);
		buffer16 = A - m - (1 - C);
		PC += 2;
		clkCycles 4;
		break;
	case 0xED: //Absolute
		buffer16 = (read(PC + 2) << 8) | read(PC + 1);
		m = read(buffer16);
		buffer16 = A - m - (1 - C);
		PC += 3;
		clkCycles 4;
		break;
	case 0xFD: //Absolute, X
		buffer16 = (read(PC + 2) << 8) | read(PC + 1);
		if (((buffer16 & 0xFF) + X) > 255) {
			clkCycles 1;
		}
		m = read((uint16_t)(buffer16 + X));
		buffer16 = A - m - (1 - C);
		PC += 3;
		clkCycles 4;
		break;
	case 0xF9: //Absolute, Y
		buffer16 = (read(PC + 2) << 8) | read(PC + 1);
		if (((buffer16 & 0xFF) + Y) > 255) {
			clkCycles 1;
		}
		m = read((uint16_t)(buffer16 + Y));
		buffer16 = A - m - (1 - C);
		PC += 3;
		clkCycles 4;
		break;
	case 0xE1: //(Indirect, X)
		buffer8 = read(PC + 1) + X;
		buffer16 = (read((uint8_t)(buffer8 + 1)) << 8) | read(buffer8);
		m = read(buffer16);
		buffer16 = A - m - (1 - C);
		PC += 2;
		clkCycles 6;
		break;
	case 0xF1: //(Indirect), Y
		buffer8 = read(PC + 1);
		buffer16 = (read((uint8_t)(buffer8 + 1)) << 8) | read(buffer8);
		if (((buffer16 & 0xFF) + Y) > 255) {
			clkCycles 1;
		}
		m = read((uint16_t)(buffer16 + Y));
		buffer16 = A - m - (1 - C);
		PC += 2;
		clkCycles 5;
		break;
	case 0xEB: //UNOFFICIAL Immediate
		m = read(PC + 1);
		buffer16 = A - m - (1 - C);
		PC += 2;
		clkCycles 2;
		break;
	}
	if (buffer16 < 0x100) set_carry(); else reset_carry();
	handle_zero(buffer16);
	if (((A ^ m) & 0x80) && ((A ^ buffer16) & 0x80)) set_overflow(); else reset_overflow();
	handle_negative(buffer16);
	A = buffer16;
}
void MOS_6502::SEC() {
	set_carry();
	PC += 1;
	clkCycles 2;
}
void MOS_6502::SED() {
	set_decimal();
	PC += 1;
	clkCycles 2;
}
void MOS_6502::SEI() {
	set_interrupt_disable();
	PC += 1;
	clkCycles 2;
}
void MOS_6502::STA(uint8_t instruction) {
	switch (instruction) {
	case 0x85: //Zero Page
		write(read(PC + 1), A);
		PC += 2;
		clkCycles 3;
		break;
	case 0x95: //Zero Page, X
		buffer16 = read(PC + 1) + X;
		if (((read(PC + 1) & 0xFF) + X) > 255) {
			buffer16 -= 0x0100;
		}
		write(buffer16, A);
		PC += 2;
		clkCycles 4;
		break;
	case 0x8D: //Absolute
		buffer16 = (read(PC + 2) << 8) | read(PC + 1);
		write(buffer16, A);
		PC += 3;
		clkCycles 4;
		break;
	case 0x9D: //Absolute, X
		buffer16 = (read(PC + 2) << 8) | read(PC + 1);
		write((uint16_t)(buffer16 + X), A);
		PC += 3;
		clkCycles 5;
		break;
	case 0x99: //Absolute, Y
		buffer16 = (read(PC + 2) << 8) | read(PC + 1);
		write((uint16_t)(buffer16 + Y), A);
		PC += 3;
		clkCycles 5;
		break;
	case 0x81: //(Indirect, X)
		buffer8 = read(PC + 1) + X;
		buffer16 = (read((uint8_t)(buffer8 + 1)) << 8) | read(buffer8);
		write(buffer16, A);
		PC += 2;
		clkCycles 6;
		break;
	case 0x91: //(Indirect), Y
		buffer8 = read(PC + 1);
		buffer16 = (read((uint8_t)(buffer8 + 1)) << 8) | read(buffer8);
		write(buffer16 + Y, A);
		PC += 2;
		clkCycles 6;
		break;
	}
}
void MOS_6502::STX(uint8_t instruction) {
	switch (instruction) {
	case 0x86: //Zero Page
		write(read(PC + 1), X);
		PC += 2;
		clkCycles 3;
		break;
	case 0x96: //Zero Page, Y
		buffer16 = read(PC + 1) + Y;
		if (((read(PC + 1) & 0xFF) + Y) > 255) {
			buffer16 -= 0x0100;
		}
		write(buffer16, X);
		PC += 2;
		clkCycles 4;
		break;
	case 0x8E: //Absolute
		buffer16 = (read(PC + 2) << 8) | read(PC + 1);
		write(buffer16, X);
		PC += 3;
		clkCycles 4;
		break;
	}
}
void MOS_6502::STY(uint8_t instruction) {
	switch (instruction) {
	case 0x84: //Zero Page
		write(read(PC + 1), Y);
		PC += 2;
		clkCycles 3;
		break;
	case 0x94: //Zero Page, X
		buffer16 = read(PC + 1) + X;
		if (((read(PC + 1) & 0xFF) + X) > 255) {
			buffer16 -= 0x0100;
		}
		write(buffer16, Y);
		PC += 2;
		clkCycles 4;
		break;
	case 0x8C: //Absolute
		buffer16 = (read(PC + 2) << 8) | read(PC + 1);
		write(buffer16, Y);
		PC += 3;
		clkCycles 4;
		break;
	}
}
void MOS_6502::TAX() {
	X = A;
	handle_zero(X);
	handle_negative(X);
	PC += 1;
	clkCycles 2;
}
void MOS_6502::TAY() {
	Y = A;
	handle_zero(Y);
	handle_negative(Y);
	PC += 1;
	clkCycles 2;
}
void MOS_6502::TSX() {
	X = S;
	handle_zero(X);
	handle_negative(X);
	PC += 1;
	clkCycles 2;
}
void MOS_6502::TXA() {
	A = X;
	handle_zero(A);
	handle_negative(A);
	PC += 1;
	clkCycles 2;
}
void MOS_6502::TXS() {
	S = X;
	PC += 1;
	clkCycles 2;
}
void MOS_6502::TYA() {
	A = Y;
	handle_zero(A);
	handle_negative(A);
	PC += 1;
	clkCycles 2;
}

//unofficial opcodes
void MOS_6502::DCP(uint8_t instruction) {
	switch (instruction) {
	case 0xC7: //Zero Page
		buffer32 = read(PC + 1);
		buffer16 = read(buffer32) - 1;
		write(buffer32, buffer16);
		buffer16 = A - buffer16;
		PC += 2;
		clkCycles 5;
		break;
	case 0xD7: //Zero Page, X
		buffer32 = read(PC + 1) + X;
		if (((read(PC + 1) & 0xFF) + X) > 255) {
			buffer32 -= 0x100;
		}
		buffer16 = read(buffer32) - 1;
		write(buffer32, buffer16);
		buffer16 = A - buffer16;
		PC += 2;
		clkCycles 6;
		break;
	case 0xC3: //(Indirect, X)
		buffer8 = read(PC + 1) + X;
		buffer32 = (read(buffer8 + 1) << 8) | read(buffer8);
		buffer16 = read(buffer32) - 1;
		write(buffer32, buffer16);
		buffer16 = A - buffer16;
		PC += 2;
		clkCycles 8;
		break;
	case 0xD3: //(Indirect), Y
		buffer8 = read(PC + 1);
		buffer32 = (read(buffer8 + 1) << 8) | read(buffer8);
		buffer16 = read(buffer32 + Y) - 1;
		write(buffer32 + Y, buffer16);
		buffer16 = A - buffer16;
		PC += 2;
		clkCycles 8;
		break;
	case 0xCF: //Absolute
		buffer32 = (read(PC + 2) << 8) | read(PC + 1);
		buffer16 = read(buffer32) - 1;
		write(buffer32, buffer16);
		buffer16 = A - buffer16;
		PC += 3;
		clkCycles 8;
		break;
	case 0xDF: //Absolute, X
		buffer32 = (read(PC + 2) << 8) | read(PC + 1);
		buffer16 = read(buffer32 + X) - 1;
		write(buffer32 + X, buffer16);
		buffer16 = A - buffer16;
		PC += 3;
		clkCycles 7;
		break;
	case 0xDB: //Absolute, Y
		buffer32 = (read(PC + 2) << 8) | read(PC + 1);
		buffer16 = read(buffer32 + Y) - 1;
		write(buffer32 + Y, buffer16);
		buffer16 = A - buffer16;
		PC += 3;
		clkCycles 7;
		break;
	}
	handle_zero(buffer16);
	handle_negative(buffer16);
}
void MOS_6502::DOP(uint8_t instruction) {
	switch (instruction) { //choose correct addressing mode
	case 0x80: case 0x82: case 0xC2: case 0xE2: case 0x89: //immediate
		clkCycles 2; 
		break;
	case 0x04: case 0x44: case 0x64: //zero page
		clkCycles 3;
		break;
	case 0x14: case 0x34: case 0x54: case 0x74: case 0xD4: case 0xF4: //zero page, x
		clkCycles 4;
		break;
	}
	PC += 2;
}
void MOS_6502::ISB(uint8_t instruction) {
	uint8_t m = 0;
	switch (instruction) {
	case 0xE7: //Zero Page
		buffer32 = read(PC + 1);
		m = read(buffer32) + 1;
		write(buffer32, m);
		buffer16 = A - m - (1 - C);
		PC += 2;
		clkCycles 5;
		break;
	case 0xF7: //Zero Page, X
		buffer32 = read(PC + 1) + X;
		if (((read(PC + 1) & 0xFF) + X) > 255) {
			buffer32 -= 0x100;
		}
		m = read(buffer32) + 1;
		write(buffer32, m);
		buffer16 = A - m - (1 - C);
		PC += 2;
		clkCycles 6;
		break;
	case 0xE3: //(Indirect, X)
		buffer8 = read(PC + 1) + X;
		buffer32 = (read(buffer8 + 1) << 8) | read(buffer8);
		m = read(buffer32) + 1;
		write(buffer32, m);
		buffer16 = A - m - (1 - C);
		PC += 2;
		clkCycles 8;
		break;
	case 0xF3: //(Indirect), Y
		buffer8 = read(PC + 1);
		buffer32 = (read(buffer8 + 1) << 8) | read(buffer8);
		m = read(buffer32 + Y) + 1;
		write(buffer32 + Y, m);
		buffer16 = A - m - (1 - C);
		PC += 2;
		clkCycles 8;
		break;
	case 0xEF: //Absolute
		buffer32 = (read(PC + 2) << 8) | read(PC + 1);
		m = read(buffer32) + 1;
		write(buffer32, m);
		buffer16 = A - m - (1 - C);
		PC += 3;
		clkCycles 6;
		break;
	case 0xFF: //Absolute, X
		buffer32 = (read(PC + 2) << 8) | read(PC + 1);
		m = read(buffer32 + X) + 1;
		write(buffer32 + X, m);
		buffer16 = A - m - (1 - C);
		PC += 3;
		clkCycles 7;
		break;
	case 0xFB: //Absolute, Y
		buffer32 = (read(PC + 2) << 8) | read(PC + 1);
		m = read(buffer32 + Y) + 1;
		write(buffer32 + Y, m);
		buffer16 = A - m - (1 - C);
		PC += 3;
		clkCycles 7;
		break;
	}
	if (buffer16 < 0x100) set_carry(); else reset_carry();
	handle_zero(buffer16);
	if (((A ^ m) & 0x80) && ((A ^ buffer16) & 0x80)) set_overflow(); else reset_overflow();
	handle_negative(buffer16);
	A = buffer16;
}
void MOS_6502::LAX(uint8_t instruction) {
	switch (instruction) {
	case 0xA7: //Zero Page
		buffer16 =  read(read(PC + 1));
		PC += 2;
		clkCycles 4;
		break;
	case 0xB7: //Zero Page, Y
		buffer16 = read(PC + 1) + Y;
		if (((read(PC + 1) & 0xFF) + Y) > 255) {
			buffer16 -= 0x100;
		}
		buffer16 = read(buffer16);
		PC += 2;
		clkCycles 4;
		break;
	case 0xAF: //Absolute
		buffer16 = (read(PC + 2) << 8) | read(PC + 1);
		buffer16 = read(buffer16);
		PC += 3;
		clkCycles 4;
		break;
	case 0xBF: //Absolute, Y
		buffer16 = (read(PC + 2) << 8) | read(PC + 1);
		if (((buffer16 & 0xFF) + Y) > 255) {
			clkCycles 1;
		}
		buffer16 = read(buffer16 + Y);
		PC += 3;
		clkCycles 4;
		break;
	case 0xA3: //(Indirect, X)
		buffer8 = read(PC + 1) + X;
		buffer16 = (read(buffer8 + 1) << 8) | read(buffer8);
		buffer16 = read(buffer16);
		PC += 2;
		clkCycles 6;
		break;
	case 0xB3: //(Indirect), Y
		buffer8 = read(PC + 1);
		buffer16 = (read(buffer8 + 1) << 8) | read(buffer8);
		if (((buffer16 & 0xFF) + Y) > 255) {
			clkCycles 1;
		}
		buffer16 = read(buffer16 + Y);
		PC += 2;
		clkCycles 5;
		break;
	}
	handle_zero(buffer16);
	handle_negative(buffer16);
	A = buffer16;
	X = buffer16;
}
void MOS_6502::RLA(uint8_t instruction) {
	switch (instruction) {
	case 0x27: //Zero Page
		buffer32 = read(PC + 1);
		buffer16 = read(buffer32);
		buffer16 = (buffer16 << 1) | C;
		write(buffer32, buffer16);
		buffer8 = A & buffer16;
		PC += 2;
		clkCycles 5;
		break;
	case 0x37: //Zero Page, X
		buffer32 = read(PC + 1) + X;
		if (((read(PC + 1) & 0xFF) + X) > 255) {
			buffer32 -= 0x100;
		}
		buffer16 = read(buffer32);
		buffer16 = (buffer16 << 1) | C;
		write(buffer32, buffer16);
		buffer8 = A & buffer16;
		PC += 2;
		clkCycles 6;
		break;
	case 0x23: //(Indirect, X)
		buffer8 = read(PC + 1) + X;
		buffer32 = (read(buffer8 + 1) << 8) | read(buffer8);
		buffer16 = read(buffer32);
		buffer16 = (buffer16 << 1) | C;
		write(buffer32, buffer16);
		buffer8 = A & buffer16;
		PC += 2;
		clkCycles 8;
		break;
	case 0x33: //(Indirect), Y
		buffer8 = read(PC + 1);
		buffer32 = (read(buffer8 + 1) << 8) | read(buffer8);
		buffer16 = read(buffer32 + Y);
		buffer16 = (buffer16 << 1) | C;
		write(buffer32 + Y, buffer16);
		buffer8 = A & buffer16;
		PC += 2;
		clkCycles 8;
		break;
	case 0x2F: //Absolute
		buffer32 = (read(PC + 2) << 8) | read(PC + 1);
		buffer16 = read(buffer32);
		buffer16 = (buffer16 << 1) | C;
		write(buffer32, buffer16);
		buffer8 = A & buffer16;
		PC += 3;
		clkCycles 6;
		break;
	case 0x3F: //Absolute, X
		buffer32 = (read(PC + 2) << 8) | read(PC + 1);
		buffer16 = read(buffer32 + X);
		buffer16 = (buffer16 << 1) | C;
		write(buffer32 + X, buffer16);
		buffer8 = A & buffer16;
		PC += 3;
		clkCycles 7;
		break;
	case 0x3B: //Absolute, Y
		buffer32 = (read(PC + 2) << 8) | read(PC + 1);
		buffer16 = read(buffer32 + Y);
		buffer16 = (buffer16 << 1) | C;
		write(buffer32 + Y, buffer16);
		buffer8 = A & buffer16;
		PC += 3;
		clkCycles 7;
		break;
	}
	handle_carry(buffer16);
	handle_zero(buffer8);
	handle_negative(buffer8);
	A = buffer8;
}
void MOS_6502::RRA(uint8_t instruction) { //doesnt work according to nestest.nes, FIX LATER
	uint8_t m = 0;
	uint8_t temp = C;
	switch (instruction) {
	case 0x67: //Zero Page
		buffer16 = read(read(PC + 1));
		if (buffer16 & 0x01) set_carry(); else reset_carry();
		buffer16 = buffer16 >> 1;
		m = buffer16 | (temp << 7);
		write(read(PC + 1), m);
		buffer16 = A + m + C;
		PC += 2;
		clkCycles 5;
		break;
	case 0x77: //Zero Page, X
		buffer32 = read(PC + 1) + X;
		if (((read(PC + 1) & 0xFF) + X) > 255) {
			buffer32 -= 0x0100;
		}
		buffer16 = read(buffer32);
		if (buffer16 & 0x01) set_carry(); else reset_carry();
		buffer16 = buffer16 >> 1;
		m = buffer16 | (temp << 7);
		write(buffer32, m);
		buffer16 = A + m + C;
		PC += 2;
		clkCycles 6;
		break;
	case 0x63: //(Indirect, X)
		buffer8 = read(PC + 1) + X;
		buffer32 = (read(buffer8 + 1) << 8) | read(buffer8);
		buffer16 = read(buffer32);
		if (buffer16 & 0x01) set_carry(); else reset_carry();
		buffer16 = buffer16 >> 1;
		m = buffer16 | (temp << 7);
		write(buffer32, m);
		buffer16 = A + m + C;
		PC += 2;
		clkCycles 8;
		break;
	case 0x73: //(Indirect), Y
		buffer8 = read(PC + 1);
		buffer32 = (read(buffer8 + 1) << 8) | read(buffer8);
		buffer16 = read(buffer32 + Y);
		if (buffer16 & 0x01) set_carry(); else reset_carry();
		buffer16 = buffer16 >> 1;
		m = buffer16 | (temp << 7);
		write(buffer32 + Y, m);
		buffer16 = A + m + C;
		PC += 2;
		clkCycles 8;
		break;
	case 0x6F: //Absolute
		buffer32 = ((read(PC + 2) << 8) | read(PC + 1));
		if (read(buffer32) & 0x01) set_carry(); else reset_carry();
		buffer16 = read(buffer32) >> 1;
		m = buffer16 | (temp << 7);
		write(buffer32, m);
		buffer16 = A + m + C;
		PC += 3;
		clkCycles 6;
		break;
	case 0x7F: //Absolute, X
		buffer32 = ((read(PC + 2) << 8) | read(PC + 1));
		if (read(buffer32 + X) & 0x01) set_carry(); else reset_carry();
		buffer16 = read(buffer32 + X) >> 1;
		m = buffer16 | (temp << 7);
		write(buffer32 + X, m);
		buffer16 = A + m + C;
		PC += 3;
		clkCycles 7;
		break;
	case 0x7B: //Absolute, Y
		buffer32 = ((read(PC + 2) << 8) | read(PC + 1));
		if (read(buffer32 + Y) & 0x01) set_carry(); else reset_carry();
		buffer16 = read(buffer32 + Y) >> 1;
		m = buffer16 | (temp << 7);
		write(buffer32 + Y, m);
		buffer16 = A + m + C;
		PC += 3;
		clkCycles 7;
		break;
	}
	handle_carry(buffer16);
	handle_zero(buffer16);
	handle_negative(buffer16);
	A = buffer16;
}
void MOS_6502::SAX(uint8_t instruction) {
	switch(instruction){
	case 0x87: //Zero Page
		buffer16 = read(PC + 1);
		PC += 2;
		clkCycles 3;
		break;
	case 0x97: //Zero Page, Y
		buffer16 = read(PC + 1) + Y;
		if (((read(PC + 1) & 0xFF) + Y) > 255) {
			buffer16 -= 0x100;
		}
		PC += 2;
		clkCycles 3;
		break;
	case 0x83: //(Indirect, X)
		buffer8 = read(PC + 1) + X;
		buffer16 = (read(buffer8 + 1) << 8) | read(buffer8);
		PC += 2;
		clkCycles 6;
		break;
	case 0x8F: //Absolute
		buffer16 = (read(PC + 2) << 8) | read(PC + 1);
		PC += 3;
		clkCycles 4;
		break;
	}
	write(buffer16, A & X);
}
void MOS_6502::SLO(uint8_t instruction) {
	switch (instruction) {
	case 0x07: //Zero Page
		buffer32 = read(PC + 1);
		buffer16 = read(buffer32) << 1;
		handle_carry(buffer16);
		write(buffer32, buffer16);
		buffer16 = A | buffer16;
		PC += 2;
		clkCycles 5;
		break;
	case 0x17: //Zero Page, X
		buffer32 = read(PC + 1) + X;
		if (((read(PC + 1) & 0xFF) + X) > 255) {
			buffer32 -= 0x100;
		}
		buffer16 = read(buffer32) << 1;
		handle_carry(buffer16);
		write(buffer32, buffer16);
		buffer16 = A | buffer16;
		PC += 2;
		clkCycles 6;
		break;
	case 0x03: //(Indirect, X)
		buffer8 = read(PC + 1) + X;
		buffer32 = (read(buffer8 + 1) << 8) | read(buffer8);
		buffer16 = read(buffer32) << 1;
		handle_carry(buffer16);
		write(buffer32, buffer16);
		buffer16 = A | buffer16;
		PC += 2;
		clkCycles 8;
		break;
	case 0x13: //(Indirect), Y
		buffer8 = read(PC + 1);
		buffer32 = (read(buffer8 + 1) << 8) | read(buffer8);
		buffer16 = read(buffer32 + Y) << 1;
		handle_carry(buffer16);
		write(buffer32 + Y, buffer16);
		buffer16 = A | buffer16;
		PC += 2;
		clkCycles 8;
		break;
	case 0x0F: //Absolute
		buffer32 = (read(PC + 2) << 8) | read(PC + 1);
		buffer16 = read(buffer32) << 1;
		handle_carry(buffer16);
		write(buffer32, buffer16);
		buffer16 = A | buffer16;
		PC += 3;
		clkCycles 6;
		break;
	case 0x1F: //Absolute, X
		buffer32 = (read(PC + 2) << 8) | read(PC + 1);
		buffer16 = read(buffer32 + X) << 1;
		handle_carry(buffer16);
		write(buffer32 + X, buffer16);
		buffer16 = A | buffer16;
		PC += 3;
		clkCycles 7;
		break;
	case 0x1B: //Absolute, Y
		buffer32 = (read(PC + 2) << 8) | read(PC + 1);
		buffer16 = read(buffer32 + Y) << 1;
		handle_carry(buffer16);
		write(buffer32 + Y, buffer16);
		buffer16 = A | buffer16;
		PC += 3;
		clkCycles 7;
		break;
	}
	handle_zero(buffer16);
	handle_negative(buffer16);
	A = buffer16;
}
void MOS_6502::SRE(uint8_t instruction) {
	switch (instruction) {
	case 0x47: //Zero Page
		buffer16 = read(read(PC + 1));
		if (buffer16 & 0x01) set_carry(); else reset_carry();
		buffer16 >>= 1;
		write(read(PC + 1), buffer16);
		buffer16 = A ^ buffer16;
		PC += 2;
		clkCycles 5;
		break;
	case 0x57: //Zero Page, X
		buffer32 = read(PC + 1) + X;
		if (((read(PC + 1) & 0xFF) + X) > 255) {
			buffer32 -= 0x0100;
		}
		buffer16 = read(buffer32);
		if (buffer16 & 0x01) set_carry(); else reset_carry();
		buffer16 >>=  1;
		write(buffer32, buffer16);
		buffer16 = A ^ buffer16;
		PC += 2;
		clkCycles 6;
		break;
	case 0x43: //(Indirect, X)
		buffer8 = read(PC + 1) + X;
		buffer32 = (read(buffer8 + 1) << 8) | read(buffer8);
		buffer16 = read(buffer32);
		if (buffer16 & 0x01) set_carry(); else reset_carry();
		buffer16 >>= 1;
		write(buffer32, buffer16);
		buffer16 = A ^ buffer16;
		PC += 2;
		clkCycles 6;
		break;
	case 0x53: //(Indirect), Y
		buffer8 = read(PC + 1);
		buffer32 = (read(buffer8 + 1) << 8) | read(buffer8);
		buffer16 = read(buffer32 + Y);
		if (buffer16 & 0x01) set_carry(); else reset_carry();
		buffer16 >>= 1;
		write(buffer32 + Y, buffer16);
		buffer16 = A ^ buffer16;
		PC += 2;
		clkCycles 8;
		break;
	case 0x4F: //Absolute
		buffer32 = ((read(PC + 2) << 8) | read(PC + 1));
		if (read(buffer32) & 0x01) set_carry(); else reset_carry();
		buffer16 = read(buffer32) >> 1;
		write(buffer32, buffer16);
		buffer16 = A ^ buffer16;
		PC += 3;
		clkCycles 6;
		break;
	case 0x5F: //Absolute, X
		buffer32 = ((read(PC + 2) << 8) | read(PC + 1));
		if (read(buffer32 + X) & 0x01) set_carry(); else reset_carry();
		buffer16 = read(buffer32 + X) >> 1;
		write(buffer32 + X, buffer16);
		buffer16 = A ^ buffer16;
		PC += 3;
		clkCycles 7;
		break;
	case 0x5B: //Absolute, Y
		buffer32 = ((read(PC + 2) << 8) | read(PC + 1));
		if (read(buffer32 + X) & 0x01) set_carry(); else reset_carry();
		buffer16 = read(buffer32 + X) >> 1;
		write(buffer32 + X, buffer16);
		buffer16 = A ^ buffer16;
		PC += 3;
		clkCycles 7;
		break;
	}
	handle_zero(buffer16);
	handle_negative(buffer16);
	A = buffer16;
}
void MOS_6502::TOP(uint8_t instruction) {
	switch (instruction) {
	case 0x0C: //absolute
		clkCycles 4;
		break;
	case 0x1C: case 0x3C: case 0x5C: case 0x7C: case 0xDC: case 0xFC: //absolute, x
		clkCycles 5;
	}
	PC += 3;
}

//stack operations
void MOS_6502::stackPush(uint8_t byte) {
	write(0x0100 + S, byte);
	if (S == 0x00) S = 0xFF; else S -= 1;
}
uint8_t MOS_6502::stackPop() {
	if (S == 0xFF) S = 0x00; else S += 1;
	return read(0x0100 + S);
}

//handle flags
void MOS_6502::handle_carry(uint16_t buffer16) {
	if (buffer16 >> 8) set_carry(); else reset_carry();
}
void MOS_6502::handle_zero(uint8_t buffer8) {
	if (buffer8 == 0) set_zero(); else reset_zero();
}
void MOS_6502::handle_interrupt_disable(uint16_t buffer16) {
	//do later
}
void MOS_6502::handle_decimal(uint16_t buffer16) {
	//do later
}
void MOS_6502::handle_break(uint16_t buffer16) {
	//do later
}
void MOS_6502::handle_overflow(uint8_t buffer8) {
	if ((buffer8 >> 6) & 0x01) set_overflow(); else reset_overflow();
}
void MOS_6502::handle_negative(uint8_t buffer8) {
	if (buffer8 >> 7) set_negative(); else reset_negative();
}

//set flags
void MOS_6502::set_carry() {
	P |= 0x01;
}
void MOS_6502::set_zero() {
	P |= 0x02;
}
void MOS_6502::set_interrupt_disable() {
	P |= 0x04;
}
void MOS_6502::set_decimal() {
	P |= 0x08;
}
void MOS_6502::set_break() {
	P |= 0x10;
}
void MOS_6502::set_overflow() {
	P |= 0x40;
}
void MOS_6502::set_negative() {
	P |= 0x80;
}

//reset flags
void MOS_6502::reset_carry() {
	P &= ~(0x01);
}
void MOS_6502::reset_zero() {
	P &= ~(0x02);
}
void MOS_6502::reset_interrupt_disable() {
	P &= ~(0x04);
}
void MOS_6502::reset_decimal() {
	P &= ~(0x08);
}
void MOS_6502::reset_break() {
	P &= ~(0x10);
}
void MOS_6502::reset_overflow() {
	P &= ~(0x40);
}
void MOS_6502::reset_negative() {
	P &= ~(0x80);
}
