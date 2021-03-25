#ifndef MOS_6502_H
#define MOS_6502_H
#include <iostream>

#include "cpubus.h"

class MOS_6502 {
protected:
	//CPU clock cycles count
	int cycles = 7;
	#define clkCycles cycles += 
	//CPU Internals
	uint8_t A = 0x00; //Accumulator
	uint8_t X = 0x00; //X Index Register
	uint8_t Y = 0x00; //Y Index Register
	uint8_t P = 0x24; //Processor Status Flag Register, N V - B D I Z C
	//quick defines so I can access individual flags
	#define C (P & 0x01)
	#define Z ((P >> 1) & 0x01)
	#define I ((P >> 2) & 0x01)
	#define D ((P >> 3) & 0x01)
	#define B ((P >> 4) & 0x01)
	#define V ((P >> 6) & 0x01)
	#define N ((P >> 7) & 0x01)
	uint8_t S = 0xFD; //Stack Pointer, hardwired to mem location 0x01, address range 0100-01FF (256-511)
	//uint16_t PC = 0x0400; //Program Counter
	uint16_t PC = 0xC000; //nestest.nes

	//Instruction functions
	//Official
	void ADC(uint8_t addressing_mode); //add with carry
	void AND(uint8_t addressing_mode); //and (with accumulator)
	void ASL(uint8_t addressing_mode); //arithmetic shift left
	void BCC(); //branch on carry clear
	void BCS(); //branch on carry set
	void BEQ(); //branch on zero set
	void BIT(uint8_t addressing_mode); //bit test
	void BMI(); //branch on negative set
	void BNE(); //branch on zero clear
	void BPL(); //branch o negative clear
	void BRK(); //break/interrupt
	void BVC(); //branch on overflow clear
	void BVS(); //branch on overflow set
	void CLC(); //clear carry
	void CLD(); //clear decimal
	void CLI(); //clear interrupt disable
	void CLV(); //clear overflow
	void CMP(uint8_t addressing_mode); //compare (with accumulator)
	void CPX(uint8_t addressing_mode); //compare with X
	void CPY(uint8_t addressing_mode); //compare with Y
	void DEC(uint8_t addressing_mode); //decrement
	void DEX(); //decrement X
	void DEY(); //decrement Y
	void EOR(uint8_t addressing_mode); //XOR (with accumulator)
	void INC(uint8_t addressing_mode); //increment
	void INX(); //increment X
	void INY(); //increment Y
	void JMP(uint8_t addressing_mode); //jump
	void JSR(); //jump subroutine
	void LDA(uint8_t addressing_mode); //load accumulator
	void LDX(uint8_t addressing_mode); //load X
	void LDY(uint8_t addressing_mode); //load Y
	void LSR(uint8_t addressing_mode); //logical shift right
	void NOP(); //no operation
	void ORA(uint8_t addressing_mode); //or (with accumulator)
	void PHA(); //push accumulator
	void PHP(); //push processor status
	void PLA(); //pull accumulator
	void PLP(); //pull processor status
	void ROL(uint8_t addressing_mode); //rotate left
	void ROR(uint8_t addressing_mode); //rotate right
	void RTI(); //return from interrupt
	void RTS(); //return from subroutine
	void SBC(uint8_t addressing_mode); //subtract with carry
	void SEC(); //set carry
	void SED(); //set decimal
	void SEI(); //set interrupt disable
	void STA(uint8_t addressing_mode); //store accumulator
	void STX(uint8_t addressing_mode); //store X
	void STY(uint8_t addressing_mode); //store Y
	void TAX(); //transfer accumulator to X
	void TAY(); //transfer accumulator to Y
	void TSX(); //transfer stack pointer to X
	void TXA(); //transfer X to accumulator
	void TXS(); //transfer X to stack pointer
	void TYA(); //transfer Y to accumulator
	//Unofficial
	void DCP(uint8_t instruction); //Subtract 1 from memory(without borrow
	void DOP(uint8_t instruction); //double NOP
	void ISB(uint8_t instruction); //Increase memory by one, then subtract memory from accumulator (with borrow)
	void LAX(uint8_t instruction); //Load accumulator and X register with memory
	void RLA(uint8_t instruction); //Rotate one bit left in memory, then AND accumulator with memory
	void RRA(uint8_t instruction); //Rotate one bit right in memory, then add memory to accumulator (with carry)
	void SAX(uint8_t instruction);
	void SLO(uint8_t instruction); //Shift left one bit in memory, then OR accumulator with memory
	void SRE(uint8_t instruction); //Shift right one bit in memory, then EOR accumulator with memory
	void TOP(uint8_t instruction); //Triple NOP
	
	//stack operations
	void stackPush(uint8_t byte);
	uint8_t stackPop();

	//handle flags
	void handle_carry(uint16_t buffer16);
	void handle_zero(uint8_t buffer8);
	void handle_interrupt_disable(uint16_t buffer16);
	void handle_decimal(uint16_t buffer16);
	void handle_break(uint16_t buffer16);
	void handle_overflow(uint8_t buffer8);
	void handle_negative(uint8_t buffer8);
	//set flags
	void set_carry();
	void set_zero();
	void set_interrupt_disable();
	void set_decimal();
	void set_break();
	void set_overflow();
	void set_negative();
	//reset flags
	void reset_carry();
	void reset_zero();
	void reset_interrupt_disable();
	void reset_decimal();
	void reset_break();
	void reset_overflow();
	void reset_negative();

public:
	cpubus& c_bus;
	MOS_6502(cpubus& cb) : c_bus(cb) {}
	void readJoypad(uint8_t buttons);
	int cyclesDone();
	void resetCycles();
	uint8_t read(uint16_t addr);
	void write(uint16_t addr, uint8_t data);
	//execute instruction
	void execute();
	//interrupts
	void IRQ();
	void NMI();
	//CPU power-up
	void powerUp();
}; 

#endif MOS_6502_H

