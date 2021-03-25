#include "Cartridge.h"
#include <iostream>
#include <iomanip>
#include <fstream>
using namespace std;

uint8_t cartridge::readPRG(uint16_t addr) {
	return PRG_ROM[addr];
}

uint8_t cartridge::readCHR(uint16_t addr) {
	return CHR_ROM[addr];
}

void cartridge::loadCartridge(string path) {
	//-----------------------------------------
	//File Processing
	int byte;
	unsigned char* element;
	uint8_t* buffer;
	ifstream myfile;
	myfile.open(path, ios::binary | ios::ate);
	if (!myfile.is_open()) {
		cout << "failed to open file";
		return;
	}
	byte = myfile.tellg();
	myfile.seekg(0, ios::beg);
	element = new unsigned char[byte];
	myfile.read((char*)element, byte);
	buffer = new uint8_t[byte];
	for (int i = 0; i < byte; ++i) {
		buffer[i] = (uint8_t)element[i];
	}
	//File Processing
	//-----------------------------------------
	if (buffer[0] != 0x4E && buffer[1] != 0x45 && buffer[2] != 0x53 && buffer[3] != 0x1A) { //header != 'NES\n'
		cout << "NES header not present, cant load ROM";
		return;
	}
	else {
		iNES_format = true;
	}
	if ((buffer[7] & 0x0C) == 0x08) {
		NES20_format = true;
	}
	if (NES20_format) {
		NES20_settings(buffer);
	}
	else {
		iNES_settings(buffer);
	}
	int local = 0;
	while (local < PRG_ROM_size) {
		PRG_ROM[local] = buffer[local + 16];
		local++;
	}
	local += 16;
	for (int i = 0; i < 8192; i++) {
		CHR_ROM[i] = buffer[local];
		local++;
	}
}

void cartridge::iNES_settings(uint8_t* buffer) {
	PRG_ROM_size = 16384 * buffer[4];
	PRG_ROM = new int[PRG_ROM_size];
	CHR_ROM_size = 8192 * buffer[5];
	if (CHR_ROM_size == 0) {
		useCHR_RAM = true;
	}
	CHR_ROM = new int[CHR_ROM_size];
	mirroring = buffer[5] & 0x01;
	usePRG_RAM = (buffer[5] >> 1) & 0x01;
	if (usePRG_RAM) {
		if (buffer[8] == 0) {
			PRG_RAM = new int[8192];
		}
		else {
			PRG_RAM = new int[8192 * buffer[8]];
		}
	}
	if ((buffer[6] >> 3) & 0x01) {
		mirroring = 2;
	}
	mapperNo = (buffer[6] >> 4) | (buffer[7] & 0xF0);
}

void cartridge::NES20_settings(uint8_t* buffer) {
	//do later
	cout << "here";
}