#include "cpubus.h"

uint8_t cpubus::read(uint16_t addr) {
	if (addr < 0x2000) { //2KiB Internal RAM, range: 0000-07FF, mirrored until 1FFF
		return RAM[addr & 0x07FF];
	}
	else if (addr < 0x4000) { //PPU registers, range: 2000-2007, mirrored until 3FFF
		return pp_unit.read(addr & 0x2007);
	}
	if (addr < 0x4018) { //APU and IO registers, range: 4000-4017
		//TO DO: AUDIO
		if (addr == 0x4016) {
			shift--;
			int hold = buttons >> shift & 0x01;
			if (shift == 0) {
				shift = 8;
			}
			return hold;
		}
	}
	else if (addr < 0x401F) { //APU and IO func normally disabled, CPU Test Mode
		//??? --dont need to implement? -- ???
	}
	else if (addr < 0x6000) { //expansion ROM
		//do later
	}
	else if (addr < 0x8000) { //SRAM
		//do later
	}
	else if (addr <= 0xFFFF) { //Cartridge Space
		return m_unit.readPRG(addr & 0x7FFF);
	}
}

void cpubus::write(uint16_t addr, uint8_t data) {
	if (addr < 0x2000) { //2KiB Internal RAM, range: 0000-07FF, mirrored until 1FFF
		RAM[addr & 0x07FF] = data;
	}
	else if (addr < 0x4000) { //PPU registers, range: 2000-2007, mirrored until 3FFF
		pp_unit.write(addr & 0x2007, data);
	}
	else if (addr == 0x4014) { //OAMDMA 
		int hold = (data << 8);
		for (int i = 0; i < 0x100; i++) {
			pp_unit.write(addr, read(hold + i));
		}
	}
	else if (addr == 0x4016) {
		if (((latchIO & 0x01) == 1) && (pollJoypad == false)) {
			pollJoypad = true;
		}
		else {
			pollJoypad = false;
		}
		latchIO = data;
	}
}

void cpubus::readJoypad(uint8_t buttonsPressed) {
	buttons = buttonsPressed;
}