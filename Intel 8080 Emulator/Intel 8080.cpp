#include <iostream>
#include <iomanip>
#include "Intel 8080.h"

void Intel_8080::init() {
	//CP/M init
	//Required when running CPU tests that require CP/M OS
	RAM[0x0000] = 0xD3;
	RAM[0x0001] = 0x00;
	RAM[0x0005] = 0xD3;
	RAM[0x0006] = 0x01;
	RAM[0x0007] = 0xC9;
	PC = 0x100;
}

//Functions to handle flags
void Intel_8080::handle_sign(uint8_t arg1) {
	uint8_t sign_bit = arg1 >> 7;
	if (sign_bit == 1) {
		F |= (1 << 7);
	}
	else {
		F &= ~(1 << 7);
	}
	F |= 0x02; //make sure bit 1 is set to 1 so: xxxx xx1x 
	F &= 0xD7; //make sure bit3+5 are 0 so: xxxx xx1x & 1101 0111 = xx0x 0x1x
}

void Intel_8080::handle_zero(uint8_t arg1) {
	if (arg1 == 0) {
		F |= (1 << 6);
	}
	else {
		F &= ~(1 << 6);
	}
	F |= 0x02;
	F &= 0xD7;
}

void Intel_8080::handle_aux(uint8_t arg1, uint8_t arg2, uint8_t operation) {
	arg1 = arg1 & 0x0F;
	arg2 = arg2 & 0x0F;
	switch (operation) {
	case 0: //ADD 
		if ((arg1 + arg2) > 0x0F) {
			F |= 0x10;
		}
		else {
			F &= 0xEF;
		}
		break;
	case 1: //ADD with carry
		if ((arg1 + arg2) >= 0x0F) {
			F |= 0x10;
		}
		else {
			F &= 0xEF;
		}
		break;
	case 2: //SUB
		if (arg2 <= arg1) {
			F |= 0x10;
		}
		else {
			F &= 0xEF;
		}
		break;
	case 3: //SUB with carry
		if (arg2 < arg1) {
			F |= 0x10;
		}
		else {
			F &= 0xEF;
		}
		break;
	}
	F |= 0x02;
	F &= 0xD7;
}

void Intel_8080::handle_parity(uint8_t arg1) {
	int count = 0;
	for (int i = 0; i < 8; i++) {
		if (((arg1 >> i) & 0x01) == 1) {
			count++;
		}
	}
	if (count % 2) { //ODD parity
		F &= ~(1 << 2);
	}
	else { //EVEN parity
		F |= (1 << 2);
	}
	F |= 0x02;
	F &= 0xD7;
}

void Intel_8080::handle_carry(uint32_t arg1) {
	if (arg1 > 65535) {
		F |= 0x01;
	}
	else {
		F &= 0xFE;
	}
	F |= 0x02;
	F &= 0xD7;
}

void Intel_8080::handle_carry(uint16_t arg1) {
	if (arg1 > 255) {
		F |= 0x01;
	}
	else {
		F &= 0xFE;
	}
	F |= 0x02;
	F &= 0xD7;
}

//breaking up instruction to valauble bits
uint8_t bit2_0;
uint8_t bit3;
uint8_t bit5_3;
uint8_t bit5_4;  //Register Pair
uint8_t bit7_6;

uint8_t LB, HB;
uint32_t buffer16bit;
uint16_t buffer8bit;

int cycles;

using namespace std;
int Intel_8080::execute(Intel_8080* i8080) {
	uint8_t instruction = RAM[PC];
	//cout << hex << setw(4) << setfill('0') << PC << ": ";
	//cout << (int)RAM[PC] << endl;
	bit7_6 = instruction >> 6;
	bit5_4 = (instruction >> 4) & 0x03;
	bit5_3 = (instruction >> 3) & 0x07;
	bit3 = (instruction >> 3) & 0x01;
	bit2_0 = instruction & 0x07;
	switch (bit7_6) {
	case 0x00:
		switch (bit2_0) {
		case 0x00:
			//START--NOP
			PC += 1;
			//END--NOP
			break;
		case 0x01:
			if (bit3 == 0x00) {
				//START--LXI RP, #
				LB = RAM[PC + 1];
				HB = RAM[PC + 2];
				if (bit5_4 == 0x00) {
					B = HB;
					C = LB;
				}
				else if (bit5_4 == 0x01) {
					D = HB;
					E = LB;
				}
				else if (bit5_4 == 0x02) {
					H = HB;
					L = LB;
				}
				else if (bit5_4 == 0x03) {
					SP = ((uint16_t)HB << 8) | (uint16_t)LB;
				}
				PC += 3;
				//END--LXI RP, #
			}
			else if (bit3 == 0x01) {
				//START--DAD RP
				buffer16bit = HL;
				if (bit5_4 == 0x00) {
					buffer16bit += (uint32_t)BC;
				}
				else if (bit5_4 == 0x01) {
					buffer16bit += (uint32_t)DE;
				}
				else if (bit5_4 == 0x02) {
					buffer16bit += (uint32_t)HL;
				}
				else if (bit5_4 == 0x03) {
					buffer16bit += (uint32_t)SP;
				}
				H = (uint8_t)(buffer16bit >> 8);
				L = (uint8_t)buffer16bit;
				i8080->handle_carry(buffer16bit);
				PC += 1;
				//END--DAD RP
			}
			break;
		case 0x02:
			if (bit5_3 == 0x04) {
				//START--SHLD a
				LB = RAM[PC + 1];
				HB = RAM[PC + 2];
				buffer16bit = ((uint16_t)HB << 8) | (uint16_t)LB;
				RAM[buffer16bit] = L;
				RAM[buffer16bit + 1] = H;
				PC += 3;
				//END--SHLD a
			}
			else if (bit5_3 == 0x05) {
				//START--LHLD a
				LB = RAM[PC + 1];
				HB = RAM[PC + 2];
				buffer16bit = ((uint16_t)HB << 8) | (uint16_t)LB;
				L = RAM[buffer16bit];
				H = RAM[buffer16bit + 1];
				PC += 3;
				//END--LHLD a
			}
			else if (bit5_3 == 0x06) {
				//START--STA a
				LB = RAM[PC + 1];
				HB = RAM[PC + 2];
				buffer16bit = ((uint16_t)HB << 8) | (uint16_t)LB;
				RAM[buffer16bit] = A;
				PC += 3;
				//END--STA a
			}
			else if (bit5_3 == 0x07) {
				//START--LDA a
				LB = RAM[PC + 1];
				HB = RAM[PC + 2];
				buffer16bit = ((uint16_t)HB << 8) | (uint16_t)LB;
				A = RAM[buffer16bit];
				PC += 3;
				//END--LDA a
			}
			else {
				if (bit3 == 0x00) {
					//START--STAX RP
					if (bit5_4 == 0x00) {
						RAM[BC] = A;
					}
					else if (bit5_4 == 0x01) {
						RAM[DE] = A;
					}
					PC += 1;
					//END--STAX RP
				}
				else {
					//START--LDAX RP
					if (bit5_4 == 0x00) {
						A = RAM[BC];
					}
					else if (bit5_4 == 0x01) {
						A = RAM[DE];
					}
					PC += 1;
					//END--LDAX RP
				}
			}
			break;
		case 0x03:
			if (bit3 == 0x00) {
				//START--INX RP
				if (bit5_4 == 0x00) {
					buffer16bit = BC + 1;
					B = buffer16bit >> 8;
					C = buffer16bit;
				}
				else if (bit5_4 == 0x01) {
					buffer16bit = DE + 1;
					D = buffer16bit >> 8;
					E = buffer16bit;
				}
				else if (bit5_4 == 0x02) {
					buffer16bit = HL + 1;
					H = buffer16bit >> 8;
					L = buffer16bit;
				}
				else if (bit5_4 == 0x03) {
					SP++;
				}
				PC += 1;
				//END--INX RP
			}
			else if (bit3 == 0x01) {
				//START--DCX RP
				if (bit5_4 == 0x00) {
					buffer16bit = BC - 1;
					B = buffer16bit >> 8;
					C = buffer16bit;
				}
				else if (bit5_4 == 0x01) {
					buffer16bit = DE - 1;
					D = buffer16bit >> 8;
					E = buffer16bit;
				}
				else if (bit5_4 == 0x02) {
					buffer16bit = HL - 1;
					H = buffer16bit >> 8;
					L = buffer16bit;
				}
				else if (bit5_4 == 0x03) {
					SP--;
				}
				PC += 1;
				//END--DCX RP
			}
			break;
		case 0x04:
			//START--INR D
			if (bit5_3 == 0x06) {
				buffer8bit = RAM[HL] + 1;
				i8080->handle_aux(RAM[HL], 1, 0);
				i8080->handle_sign(buffer8bit);
				i8080->handle_zero(buffer8bit);
				i8080->handle_parity(buffer8bit);
				RAM[HL] = buffer8bit;
			}
			else {
				buffer8bit = regs[bit5_3] + 1;
				i8080->handle_aux(regs[bit5_3], 1, 0);
				i8080->handle_sign(buffer8bit);
				i8080->handle_zero(buffer8bit);
				i8080->handle_parity(buffer8bit);
				regs[bit5_3] = buffer8bit;
			}
			PC += 1;
			//END--INR D
			break;
		case 0x05:
			//START--DCR D
			if (bit5_3 == 0x06) {
				i8080->handle_aux(RAM[HL], 1, 2);
				RAM[HL]--;
				i8080->handle_sign(RAM[HL]);
				i8080->handle_zero(RAM[HL]);
				i8080->handle_parity(RAM[HL]);
			}
			else {
				i8080->handle_aux(regs[bit5_3], 1, 2);
				regs[bit5_3]--;
				i8080->handle_sign(regs[bit5_3]);
				i8080->handle_zero(regs[bit5_3]);
				i8080->handle_parity(regs[bit5_3]);
			}
			PC += 1;
			//END--DCR D
			break;
		case 0x06:
			//START--MVI D,#
			if (bit5_3 == 0x06) {
				RAM[HL] = RAM[PC + 1];
			}
			else {
				regs[bit5_3] = RAM[PC + 1];
			}
			PC += 2;
			//END--MVI D,#
			break;
		case 0x07:
			if (bit5_3 == 0x00) {
				//START--RLC
				if (A & 0x80) {
					F |= 0x01;
				}
				else {
					F &= ~(0x01);
				}
				A = (A >> 7) | (A << 1);
				PC += 1;
				//START--RLC
			}
			else if (bit5_3 == 0x01) {
				//START--RRC
				if (A & 0x01) {
					F |= 0x01;
				}
				else {
					F &= ~(0x01);
				}
				A = (A << 7) | (A >> 1);
				PC += 1;
				//END--RRC
			}
			else if (bit5_3 == 0x02) {
				//START--RAL
				buffer8bit = (F & 0x01);
				if (A & 0x80) {
					F |= 0x01;
				}
				else {
					F &= ~(0x01);
				}
				A = (A << 1) | buffer8bit;
				PC += 1;
				//END--RAL
			}
			else if (bit5_3 == 0x03) {
				//START--RAR
				buffer8bit = (F & 0x01);
				if (A & 0x01) {
					F |= 0x01;
				}
				else {
					F &= ~(0x01);
				}
				A = (A >> 1) | (buffer8bit << 7);
				PC += 1;
				//END--RAR
			}
			else if (bit5_3 == 0x04) {
				//START--DAA
				buffer8bit = A;
				if (((F & 0x10) == 0x10) || ((buffer8bit & 0x0F) > 0x09)) {
					i8080->handle_aux(buffer8bit, 0x06, 0);
					buffer8bit += 0x06;
					if (buffer8bit > 0xFF) {
						F |= 0x01;
					}
				}
				if (((F & 0x01) == 1) || ((buffer8bit & 0xF0) > 0x90)) {
					buffer8bit += 0x60;
					if (buffer8bit > 0xFF) {
						F |= 0x01;
					}
				}
				i8080->handle_sign((uint8_t)buffer8bit);
				i8080->handle_zero((uint8_t)buffer8bit);
				i8080->handle_parity((uint8_t)buffer8bit);
				A = buffer8bit;
				PC += 1;
				//END--DAA
			}
			else if (bit5_3 == 0x05) {
				//START--CMA
				A = ~A;
				PC += 1;
				//END--CMA
			}
			else if (bit5_3 == 0x06) {
				//START--STC
				F |= 1;
				PC += 1;
				//END--STC
			}
			else if (bit5_3 == 0x07) {
				//START--CMC
				F ^= 0x01;
				PC += 1;
				//END--CMC
			}
			break;
		}
		break;
	case 0x01:
		if ((bit5_3 == 0x06) && (bit2_0 == 0x06)) {
			//START--HALT: stop execution until interrupt occurs
			//
			PC += 1;
			return 1;
			//
			//END--HALT
		}
		else {
			//START--MOV
			if (bit5_3 == 0x06) {
				RAM[HL] = regs[bit2_0];
			}
			else if (bit2_0 == 0x06) {
				regs[bit5_3] = RAM[HL];
			}
			else {
				regs[bit5_3] = regs[bit2_0];
			}
			PC += 1;
			//END--MOV
		}
		break;

	case 0x02:
		if (bit5_3 == 0x00) {
			//START--ADD S
			if (bit2_0 == 0x06) {
				i8080->handle_aux(A, RAM[HL], 0);
				buffer8bit = A + RAM[HL];
			}
			else {
				i8080->handle_aux(A, regs[bit2_0], 0);
				buffer8bit = A + regs[bit2_0];
			}
			i8080->handle_sign(buffer8bit);
			i8080->handle_zero(buffer8bit);
			i8080->handle_parity(buffer8bit);
			i8080->handle_carry(buffer8bit);
			A = buffer8bit;
			PC += 1;
			//END--ADD S
		}
		else if (bit5_3 == 0x01) {
			//START--ADC S
			if (bit2_0 == 0x06) {
				buffer8bit = A + RAM[HL] + (F & 0x01);
				if ((F & 0x01) == 1) {
					i8080->handle_aux(A, RAM[HL], 1);
				}
				else {
					i8080->handle_aux(A, RAM[HL], 0);
				}
			}
			else {
				buffer8bit = A + regs[bit2_0] + (F & 0x01);
				if ((F & 0x01) == 1) {
					i8080->handle_aux(A, regs[bit2_0], 1);
				}
				else {
					i8080->handle_aux(A, regs[bit2_0], 0);
				}
			}
			i8080->handle_sign(buffer8bit);
			i8080->handle_zero(buffer8bit);
			i8080->handle_parity(buffer8bit);
			i8080->handle_carry(buffer8bit);
			A = buffer8bit;
			PC += 1;
			//END--ADC S
		}
		else if (bit5_3 == 0x02) {
			//START--SUB S
			if (bit2_0 == 0x06) {
				i8080->handle_aux(A, RAM[HL], 2);
				buffer8bit = A - RAM[HL];
			}
			else {
				i8080->handle_aux(A, regs[bit2_0], 2);
				buffer8bit = A - regs[bit2_0];
			}
			i8080->handle_sign(buffer8bit);
			i8080->handle_zero(buffer8bit);
			i8080->handle_parity(buffer8bit);
			i8080->handle_carry(buffer8bit);
			A = buffer8bit;
			PC += 1;
			//END--SUB S
		}
		else if (bit5_3 == 0x03) {
			//START--SBB S
			if (bit2_0 == 0x06) {
				buffer8bit = A - RAM[HL] - (F & 0x01);
				if ((F & 0x01) == 1) {
					i8080->handle_aux(A, RAM[HL], 3);
				}
				else {
					i8080->handle_aux(A, RAM[HL], 2);
				}
				if (((buffer8bit & 0xFF) >= A) && (RAM[HL] | (F & 0x01))) { //handles carry
					F |= 0x01;
				}
				else {
					F &= ~(0x01);
				}
			}
			else {
				buffer8bit = A - regs[bit2_0] - (F & 0x01);
				if ((F & 0x01) == 1) {
					i8080->handle_aux(A, regs[bit2_0], 3);
				}
				else {
					i8080->handle_aux(A, regs[bit2_0], 2);
				}
				if (((buffer8bit & 0xFF) >= A) && (regs[bit2_0] | (F & 0x01))) { //handles carry
					F |= 0x01;
				}
				else {
					F &= ~(0x01);
				}
			}
			i8080->handle_sign(buffer8bit);
			i8080->handle_zero(buffer8bit);
			i8080->handle_parity(buffer8bit);
			A = buffer8bit;
			PC += 1;
			//END--SBB S
		}
		else if (bit5_3 == 0x04) {
			//START--ANA S
			if (bit2_0 == 0x06) {
				if ((A | RAM[HL]) & 0x08) { //handle aux
					F |= (1 << 4);
				}
				else {
					F &= ~(0x01 << 4);
				}
				A &= RAM[HL];
			}
			else {
				if ((A | regs[bit2_0]) & 0x08) { //handle aux
					F |= (1 << 4);
				}
				else {
					F &= ~(0x01 << 4);
				}
				A &= regs[bit2_0];
			}
			i8080->handle_sign(A);
			i8080->handle_zero(A);
			i8080->handle_parity(A);
			F &= 0xFE; //make sure to clear carry;
			PC += 1;
			//END--ANA S
		}
		else if (bit5_3 == 0x05) {
			//START--XRA S
			if (bit2_0 == 0x06) {
				A ^= RAM[HL];
			}
			else {
				A ^= regs[bit2_0];
			}
			i8080->handle_sign(A);
			i8080->handle_zero(A);
			i8080->handle_parity(A);
			F &= 0xEF; //make sure to clear aux
			F &= 0xFE; //make sure to clear carry
			PC += 1;
			//END--XRA S
		}
		else if (bit5_3 == 0x06) {
			//START--ORA S
			if (bit2_0 == 0x06) {
				A |= RAM[HL];
			}
			else {
				A |= regs[bit2_0];
			}
			i8080->handle_sign(A);
			i8080->handle_zero(A);
			i8080->handle_parity(A);
			F &= 0xEF; //make sure to clear aux
			F &= 0xFE; //make sure to clear carry
			PC += 1;
			//END--ORA S
		}
		else if (bit5_3 == 0x07) {
			//START--CMP S
			if (bit2_0 == 0x06) {
				i8080->handle_aux(A, RAM[HL], 2);
				buffer8bit = A - RAM[HL];
			}
			else {
				i8080->handle_aux(A, regs[bit2_0], 2);
				buffer8bit = A - regs[bit2_0];
			}
			i8080->handle_sign(buffer8bit);
			i8080->handle_zero(buffer8bit);
			i8080->handle_parity(buffer8bit);
			i8080->handle_carry(buffer8bit);
			PC += 1;
			//END--CMP S
		}
		break;

	case 0x03:
		if (bit2_0 == 0x00) {
			//START--Rccc
			switch (bit5_3) {
			case 0x00:
				if (((F >> 6) & 0x01) == 0) {
					PC = RAM[SP++];
					PC |= ((uint16_t)RAM[SP++] << 8);
				}
				else {
					PC += 1;
				}
				break;
			case 0x01:
				if (((F >> 6) & 0x01) == 1) {
					PC = RAM[SP++];
					PC |= ((uint16_t)RAM[SP++] << 8);
				}
				else {
					PC += 1;
				}
				break;
			case 0x02:
				if ((F & 0x01) == 0) {
					PC = RAM[SP++];
					PC |= ((uint16_t)RAM[SP++] << 8);
				}
				else {
					PC += 1;
				}
				break;
			case 0x03:
				if ((F & 0x01) == 1) {
					PC = RAM[SP++];
					PC |= ((uint16_t)RAM[SP++] << 8);
				}
				else {
					PC += 1;
				}
				break;
			case 0x04:
				if (((F >> 2) & 0x01) == 0) {
					PC = RAM[SP++];
					PC |= ((uint16_t)RAM[SP++] << 8);
				}
				else {
					PC += 1;
				}
				break;
			case 0x05:
				if (((F >> 2) & 0x01) == 1) {
					PC = RAM[SP++];
					PC |= ((uint16_t)RAM[SP++] << 8);
				}
				else {
					PC += 1;
				}
				break;
			case 0x06:
				if (((F >> 7) & 0x01) == 0) {
					PC = RAM[SP++];
					PC |= ((uint16_t)RAM[SP++] << 8);
				}
				else {
					PC += 1;
				}
				break;
			case 0x07:
				if (((F >> 7) & 0x01) == 1) {
					PC = RAM[SP++];
					PC |= ((uint16_t)RAM[SP++] << 8);
				}
				else {
					PC += 1;
				}
				break;
			}
			//END--Rccc
		}
		else if (bit2_0 == 0x01) {
			if (bit3 == 0x01) {
				if (bit5_4 == 0x00) {
					//START--RET
					PC = RAM[SP++];
					PC |= ((uint16_t)RAM[SP++] << 8);
					//END--RET
				}
				else if (bit5_4 == 0x02) {
					//START--PCHL
					PC = HL;
					//END--PCHL
				}
				else if (bit5_4 == 0x03) {
					//START--SPHL
					SP = HL;
					PC += 1;
					//END--SPHL
				}
			}
			else {
				//START--POP RP
				if (bit5_4 == 0x00) {
					C = RAM[SP++];
					B = RAM[SP++];
				}
				else if (bit5_4 == 0x01) {
					E = RAM[SP++];
					D = RAM[SP++];
				}
				else if (bit5_4 == 0x02) {
					L = RAM[SP++];
					H = RAM[SP++];
				}
				else if (bit5_4 == 0x03) {
					F = (RAM[SP++] & 0xD7) | 0x02;
					A = RAM[SP++];
				}
				PC += 1;
				//END--POP RP
			}
		}
		else if (bit2_0 == 0x02) {
			//START--Jccc A
			switch (bit5_3) {
			case 0x00://Zero flag not set
				if (((F >> 6) & 0x01) == 0) {
					PC = ((uint16_t)RAM[PC + 2] << 8) | (uint16_t)RAM[PC + 1];
				}
				else {
					PC += 3;
				}
				break;
			case 0x01://Zero flag set
				if (((F >> 6) & 0x01) == 1) {
					PC = ((uint16_t)RAM[PC + 2] << 8) | (uint16_t)RAM[PC + 1];
				}
				else {
					PC += 3;
				}
				break;
			case 0x02://Carry flag not set
				if ((F & 0x01) == 0) {
					PC = ((uint16_t)RAM[PC + 2] << 8) | (uint16_t)RAM[PC + 1];
				}
				else {
					PC += 3;
				}
				break;
			case 0x03://Carry flag  set
				if ((F & 0x01) == 1) {
					PC = ((uint16_t)RAM[PC + 2] << 8) | (uint16_t)RAM[PC + 1];
				}
				else {
					PC += 3;
				}
				break;
			case 0x04://Parity flag not set
				if (((F >> 2) & 0x01) == 0) {
					PC = ((uint16_t)RAM[PC + 2] << 8) | (uint16_t)RAM[PC + 1];
				}
				else {
					PC += 3;
				}
				break;
			case 0x05://Parity flag set
				if (((F >> 2) & 0x01) == 1) {
					PC = ((uint16_t)RAM[PC + 2] << 8) | (uint16_t)RAM[PC + 1];
				}
				else {
					PC += 3;
				}
				break;
			case 0x06://Sign flag not set
				if (((F >> 7) & 0x01) == 0) {
					PC = ((uint16_t)RAM[PC + 2] << 8) | (uint16_t)RAM[PC + 1];
				}
				else {
					PC += 3;
				}
				break;
			case 0x07://Sign flag set
				if (((F >> 7) & 0x01) == 1) {
					PC = ((uint16_t)RAM[PC + 2] << 8) | (uint16_t)RAM[PC + 1];
				}
				else {
					PC += 3;
				}
				break;
			}
			//END--Jccc A
		}
		else if (bit2_0 == 0x03) {
			if (bit5_3 == 0x00) {
				//START--JMP a
				PC = ((uint16_t)RAM[PC + 2] << 8) | (uint16_t)RAM[PC + 1];
				//END--JMP a
			}
			else if (bit5_3 == 0x02) {
				//START--OUT P
				//START--USED WHEN RUNNING CP/M OS CPU DIAGNOSTICS TESTS!!!
				if (PC == 0x00) {
					return 1;
				}
				if (PC == 0x05) {
					if (C == 0x09) {
						int address = DE;
						while (RAM[address] != '$') {
							std::cout << RAM[address];
							address++;
						}
					}
					else if (C == 0x02) {
						std::cout << E;
					}
				}
				//END--USED WHEN RUNNING CP / M OS CPU DIAGNOSTICS TESTS!!!
				PC += 2;
				//END--OUT P
			}
			else if (bit5_3 == 0x03) {
				//START--IN P
				PC += 2;
				//END--IN P
			}
			else if (bit5_3 == 0x04) {
				//START--XTHL
				buffer16bit = HL;
				H = RAM[SP + 1];
				L = RAM[SP];
				RAM[SP + 1] = buffer16bit >> 8;
				RAM[SP] = buffer16bit;
				PC += 1;
				//END--XTHL
			}
			else if (bit5_3 == 0x05) {
				//START--XCHG
				buffer16bit = HL;
				H = D;
				L = E;
				D = buffer16bit >> 8;
				E = buffer16bit;
				PC += 1;
				//END--XCHG
			}
			else if (bit5_3 == 0x06) {
				//START--EI
				interrupt = 1;
				PC += 1;
				//END--EI
			}
			else if (bit5_3 == 0x07) {
				//START--DI
				interrupt = 0;
				PC += 1;
				//END--EI
			}
		}
		else if (bit2_0 == 0x04) {
			//START--Cccc A
			switch (bit5_3) {
			case 0x00:
				if (((F >> 6) & 0x01) == 0) {
					buffer16bit = ((uint16_t)RAM[PC + 2] << 8) | RAM[PC + 1];
					RAM[--SP] = (PC + 3) >> 8;
					RAM[--SP] = (PC + 3);
					PC = buffer16bit;
				}
				else {
					PC += 3;
				}
				break;
			case 0x01:
				if (((F >> 6) & 0x01) == 1) {
					buffer16bit = ((uint16_t)RAM[PC + 2] << 8) | RAM[PC + 1];
					RAM[--SP] = (PC + 3) >> 8;
					RAM[--SP] = (PC + 3);
					PC = buffer16bit;
				}
				else {
					PC += 3;
				}
				break;
			case 0x02:
				if ((F & 0x01) == 0) {
					buffer16bit = ((uint16_t)RAM[PC + 2] << 8) | RAM[PC + 1];
					RAM[--SP] = (PC + 3) >> 8;
					RAM[--SP] = (PC + 3);
					PC = buffer16bit;
				}
				else {
					PC += 3;
				}
				break;
			case 0x03:
				if ((F & 0x01) == 1) {
					buffer16bit = ((uint16_t)RAM[PC + 2] << 8) | RAM[PC + 1];
					RAM[--SP] = (PC + 3) >> 8;
					RAM[--SP] = (PC + 3);
					PC = buffer16bit;
				}
				else {
					PC += 3;
				}
				break;
			case 0x04:
				if (((F >> 2) & 0x01) == 0) {
					buffer16bit = ((uint16_t)RAM[PC + 2] << 8) | RAM[PC + 1];
					RAM[--SP] = (PC + 3) >> 8;
					RAM[--SP] = (PC + 3);
					PC = buffer16bit;
				}
				else {
					PC += 3;
				}
				break;
			case 0x05:
				if (((F >> 2) & 0x01) == 1) {
					buffer16bit = ((uint16_t)RAM[PC + 2] << 8) | RAM[PC + 1];
					RAM[--SP] = (PC + 3) >> 8;
					RAM[--SP] = (PC + 3);
					PC = buffer16bit;
				}
				else {
					PC += 3;
				}
				break;
			case 0x06:
				if (((F >> 7) & 0x01) == 0) {
					buffer16bit = ((uint16_t)RAM[PC + 2] << 8) | RAM[PC + 1];
					RAM[--SP] = (PC + 3) >> 8;
					RAM[--SP] = (PC + 3);
					PC = buffer16bit;
				}
				else {
					PC += 3;
				}
				break;
			case 0x07:
				if (((F >> 7) & 0x01) == 1) {
					buffer16bit = ((uint16_t)RAM[PC + 2] << 8) | RAM[PC + 1];
					RAM[--SP] = (PC + 3) >> 8;
					RAM[--SP] = (PC + 3);
					PC = buffer16bit;
				}
				else {
					PC += 3;
				}
				break;
				//END--Cccc A
			}
		}
		else if (bit2_0 == 0x05) {
			if (bit3 == 0) {
				//START--PUSH RP
				if (bit5_4 == 0x00) {
					RAM[--SP] = B;
					RAM[--SP] = C;
				}
				else if (bit5_4 == 0x01) {
					RAM[--SP] = D;
					RAM[--SP] = E;
				}
				else if (bit5_4 == 0x02) {
					RAM[--SP] = H;
					RAM[--SP] = L;
				}
				else if (bit5_4 == 0x03) {
					RAM[--SP] = A;
					RAM[--SP] = F;
				}
				PC += 1;
				//END-PUSH RP
			}
			else {
				//START--CALL A
				buffer16bit = ((uint16_t)RAM[PC + 2] << 8) | RAM[PC + 1];
				RAM[--SP] = (PC + 3) >> 8;
				RAM[--SP] = (PC + 3);
				PC = buffer16bit;
				//END--CALL A
			}
		}
		else if (bit2_0 == 0x06) {
			if (bit5_3 == 0x00) {
				//START--ADI #
				i8080->handle_aux(A, RAM[PC + 1], 0);
				buffer8bit = A + RAM[PC + 1];
				i8080->handle_sign(buffer8bit);
				i8080->handle_zero(buffer8bit);
				i8080->handle_parity(buffer8bit);
				i8080->handle_carry(buffer8bit);
				A = buffer8bit;
				PC += 2;
				//END--ADI #
			}
			else if (bit5_3 == 0x01) {
				//START--ACI #
				if ((F & 0x01) == 1) {
					i8080->handle_aux(A, RAM[PC + 1] + (F & 0x01), 0);
				}
				else {
					i8080->handle_aux(A, RAM[PC + 1], 0);
				}
				buffer8bit = A + RAM[PC + 1] + (F & 0x01);
				i8080->handle_sign(buffer8bit);
				i8080->handle_zero(buffer8bit);
				i8080->handle_parity(buffer8bit);
				i8080->handle_carry(buffer8bit);
				A = buffer8bit;
				PC += 2;
				//END--ACI #
			}
			else if (bit5_3 == 0x02) {
				//START--SUI #
				i8080->handle_aux(A, RAM[PC + 1], 2);
				buffer8bit = A - RAM[PC + 1];
				if (((buffer8bit & 0xFF) >= A) && RAM[PC + 1]) { //handles carry
					F |= 0x01;
				}
				else {
					F &= ~(0x01);
				}
				i8080->handle_sign(buffer8bit);
				i8080->handle_zero(buffer8bit);
				i8080->handle_parity(buffer8bit);
				A = buffer8bit;
				PC += 2;
				//END--SUI #
			}
			else if (bit5_3 == 0x03) {
				//START--SBI #
				buffer8bit = A - RAM[PC + 1] - (F & 0x01);
				if ((F & 0x01) == 1) {
					i8080->handle_aux(A, RAM[PC + 1] + (F & 0x01), 2);
				}
				else {
					i8080->handle_aux(A, RAM[PC + 1], 2);
				}
				if (((buffer8bit & 0xFF) >= A) && (RAM[PC + 1] | (F & 0x01))) { //handles carry
					F |= 0x01;
				}
				else {
					F &= ~(0x01);
				}
				i8080->handle_sign(buffer8bit);
				i8080->handle_zero(buffer8bit);
				i8080->handle_parity(buffer8bit);
				A = buffer8bit;
				PC += 2;
				//END--SBI #
			}
			else if (bit5_3 == 0x04) {
				//START--ANI #
				if ((A | RAM[PC+1]) & 0x08) { //handle aux
					F |= (1 << 4);
				}
				else {
					F &= ~(0x01 << 4);
				}
				A &= RAM[PC + 1];
				i8080->handle_sign(A);
				i8080->handle_zero(A);
				i8080->handle_parity(A);
				F &= 0xFE; //make sure to clear carry
				PC += 2;
				//END--ANI #
			}
			else if (bit5_3 == 0x05) {
				//START--XRI #
				F &= 0xEF; //make sure to clear aux
				A ^= RAM[PC + 1];
				i8080->handle_sign(A);
				i8080->handle_zero(A);
				i8080->handle_parity(A);
				F &= 0xFE; //make sure to clear carry
				PC += 2;
				//END--XRI #
			}
			else if (bit5_3 == 0x06) {
				//START--ORI #
				F &= 0xEF; //make sure to clear aux
				A |= RAM[PC + 1];
				i8080->handle_sign(A);
				i8080->handle_zero(A);
				i8080->handle_parity(A);
				F &= 0xFE; //make sure to clear carry
				PC += 2;
				//END--ORI #
			}
			else if (bit5_3 == 0x07) {
				//START--CPI #
				i8080->handle_aux(A, RAM[PC + 1], 2);
				buffer8bit = A - RAM[PC + 1];
				if (((buffer8bit & 0xFF) >= A) && RAM[PC + 1]) { //handles carry
					F |= 0x01;
				}
				else {
					F &= ~(0x01);
				}
				i8080->handle_sign(buffer8bit);
				i8080->handle_zero(buffer8bit);
				i8080->handle_parity(buffer8bit);
				PC += 2;
				//END--CPI #
			}
		}
		else if (bit2_0 == 0x07) {
			//START--RST N
			RAM[--SP] = PC >> 8;
			RAM[--SP] = (uint8_t)PC;
			PC = bit5_3 << 3;
			//END--RST N
		}
		break;
	}
	return 0;
}