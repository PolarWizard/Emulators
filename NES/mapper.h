#pragma once
#include "Cartridge.h"
#include <iostream>

class mapper {
public:
	uint8_t mirroring();
	cartridge& c_unit;
	mapper(cartridge& c) : c_unit(c) {}
	uint8_t readPRG(uint16_t addr);
	uint8_t readCHR(uint16_t addr);
};