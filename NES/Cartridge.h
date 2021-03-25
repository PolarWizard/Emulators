#ifndef CARTRIDGE_H
#define CARTRIDGE_H

#include <iostream>
#include <string>
using namespace std;

class cartridge {
private:
	int* PRG_ROM = NULL;
	int* PRG_RAM = NULL;
	int* CHR_ROM = NULL; //pattern table
	int* CHR_RAM = NULL;

public:
	//data extracted from cartridge header
	int PRG_ROM_size = 0;
	bool usePRG_RAM = false;
	int PRG_RAM_size = 0;
	int CHR_ROM_size = 0;
	bool useCHR_RAM = false;
	int CHR_RAM_size = 0;
	uint8_t mapper = 0;
	int mirroring = 0; //0:horizontal, 1:vertical, 2:4-Screen VRAM

	bool iNES_format = false;
	bool NES20_format = false;
	int mapperNo = 0;

	cartridge() {}
	uint8_t readPRG(uint16_t addr);
	uint8_t readCHR(uint16_t addr);
	void loadCartridge(string path);
	void iNES_settings(uint8_t* buffer);
	void NES20_settings(uint8_t* buffer);
};

#endif CARTRIDGE_H
