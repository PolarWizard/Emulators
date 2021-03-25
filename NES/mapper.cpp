#include "mapper.h"

uint8_t mapper::readPRG(uint16_t addr) {
	if (c_unit.PRG_ROM_size > 16384) {
		return c_unit.readPRG(addr);
	}
	else {
		return c_unit.readPRG(addr & 0x3FFF);
	}
}

uint8_t mapper::readCHR(uint16_t addr) {
	return c_unit.readCHR(addr);
}

uint8_t mapper::mirroring() {
	return c_unit.mirroring;
}
