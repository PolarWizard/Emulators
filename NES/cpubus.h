#pragma once
#include "PPU.h"
#include "mapper.h"
#include <iostream>

class cpubus {
private:
	//onboard RAM used by CPu
	uint8_t RAM[0x0800] = { 0x00 };
	//IO Stuff
	uint8_t latchIO = 0;
	uint8_t shift = 8;
	uint8_t buttons = 0;
	bool pollJoypad = false;
public:
	//mapper& m_unit;
	PPU& pp_unit;
	mapper& m_unit;
	cpubus(mapper& m, PPU& p) : m_unit(m), pp_unit(p) {};
	uint8_t read(uint16_t addr);
	void write(uint16_t addr, uint8_t data);
	void readJoypad(uint8_t buttonsPressed);
};