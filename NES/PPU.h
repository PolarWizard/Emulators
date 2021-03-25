#ifndef PPU_H
#define PPU_H
#include "SDL.h"
#include "mapper.h"
#include <iostream>

using namespace std;

class PPU {
private:
	//References: rendering constants  
	const int cycles = 341;
	const int scanlines = 262;
	int renderState = 0;
	//rendering
	int c_cycle;
	int c_scanline;
	bool NMI = true;

	//color palette
	uint32_t colorPalette[64] = {
		0xFF808080, 0xFF003DA6, 0xFF0012B0, 0xFF440096, 0xFFA1005E, 0xFFC7002B, 0xFFBA0600, 0xFFBC1700,
		0xFF5C2F00, 0xFF104500, 0xFF054A00, 0xFF00472E, 0xFF004166, 0xFF000000, 0xFF050505, 0xFF050505,
		0xFFC7C7C7, 0xFF0077FF, 0xFF2155FF, 0xFF8237FA, 0xFFEB2FB5, 0xFFFF2950, 0xFFFF2200, 0xFFD63200,
		0xFFC46200, 0xFF358000, 0xFF058F00, 0xFF008A55, 0xFF0099CC, 0xFF212121, 0xFF090909, 0xFF090909,
		0xFFFFFFFF, 0xFF0FD7FF, 0xFF69A2FF, 0xFFD480FF, 0xFFFF45F3, 0xFFFF618B, 0xFFFF8833, 0xFFFF9C12,
		0xFFFABC20, 0xFF9FE30E, 0xFF2BF035, 0xFF0CF0A4, 0xFF05FBFF, 0xFF5E5E5E, 0xFF0D0D0D, 0xFF0D0D0D,
		0xFFFFFFFF, 0xFFA6FCFF, 0xFFB3ECFF, 0xFFDAABEB, 0xFFFFA8F9, 0xFFFFABB3, 0xFFFFD2B0, 0xFFFFEFA6,
		0xFFFFF79C, 0xFFD7E895, 0xFFA6EDAF, 0xFFA2F2DA, 0xFF99FFFC, 0xFFDDDDDD, 0xFF111111, 0xFF111111
	};

	//PPU write byte initial state
	bool firstWrite = true;
	//PPU registers exposed to CPU
	uint8_t PPUCTRL;
	uint8_t PPUMASK;
	uint8_t PPUSTATUS;
	uint8_t OAMADDR;
	uint8_t OAMDATA;
	uint16_t PPUSCROLL;
	uint16_t PPUADDR = 0;
	uint16_t PPUDATA;
	uint8_t OAMDMA;
	//PPU Internal Registers for scrolling
	uint16_t v = 0; //current VRAM address (15 bits)
	uint16_t t = 0; //temporary VRAM address (15 bits)
	uint8_t x = 0; //fine X scroll (3 bits)
	bool w = 1; //first or second write toggle (1 bit)
	//Shift Registers
	uint8_t nt_byte;
	//used for pattern table data
	uint16_t sReg16_0;
	uint16_t sReg16_1;
	uint16_t bg_latch0;
	uint16_t bg_latch1;
	//used for attribute table
	uint8_t sReg8_0;
	uint8_t sReg8_1;
	uint8_t bgPaletteNo;
	//BG pixel
	uint8_t bg_pixel;
	//screen buffer
	int* screenBuffer = new int[61440]; //256x240
	//PPU coordinates
	//attribute table x,y
	int atX = 0;
	int atY = 0;
	//general tiles x,y
	int tileX = 0;
	int tileY = 0;
	//x,y coor of pixels in tile put into screenbuffer
	int X = 0;
	int Y = 0;
	//VRAM
	//PPU address range: 0x2000-0x2C00
	//4 nametables total each 1024/0x400 bytes big,
	//NES can only store 2 nametables and rest are mirrored
	//nametable itself is 960/0x3C0 bytes big,
	//attribute table (controls palette assignment to each 2x2 tile grid) comes after: 64/0x40
	//0x3C0 + 0x40 = 0x400
	uint8_t ciRAM[0x800] = { 0 };
	uint8_t paletteRAM[32] = { 0 };
	//Object Attribute Memory (OAM)
	uint8_t primaryOAM[256] = { 0 };
	uint8_t secondaryOAM[40] = { 0 };
	bool sprite0 = false;
	uint16_t sprReg16_0 = 0;
	uint16_t sprReg16_1 = 0;
	uint8_t sprPixel = 0;
	uint8_t sprPaletteNo = 0;
	uint8_t sprY = 0;
	uint8_t sprX = 0;
	uint8_t c_index = 0;
	uint8_t c_attri = 0;
public:
	mapper& m_unit;
	PPU(mapper& m) : m_unit(m) {};
	uint8_t IOread(uint16_t addr);
	void IOwrite(uint16_t addr, uint8_t data);
	void powerUp();
	void spriteEval();
	void getSpritePixel();
	uint8_t getPaletteColor(uint8_t paletteNo);
	void execute();
	void execute2();
	int getNT(int inc);
	void incCX();
	void incCY();
	bool sendNMI();
	int* drawScreen();
	//debugging
	void drawPatternTable();
	void drawNameTable();
};


#endif PPU_H
