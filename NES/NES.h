#pragma once
#include "SDL.h"
#include "Cartridge.h"
#include "mapper.h"
#include "PPU.h"
#include "cpubus.h"
#include "MOS_6502.h"

class NES {
private:
	//SDL Initialization Stuff
	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_Texture* texture;
	SDL_Event e;

	int* screen;
	//controller
	uint8_t joypad = 0x00;
	uint8_t button[8] = { 0 };
	MOS_6502 cpu;
	cpubus c_bus;
	PPU ppu;
	mapper map;
	cartridge c;
public:
	NES() : cpu(c_bus), c_bus(map, ppu), ppu(map), map(c){}
	void insertCartridge(string path);
	void powerUp();
	void run();
	void joypadInput();
	void screenInit();
	void drawScreen();
};
