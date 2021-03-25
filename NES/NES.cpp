#include "NES.h"

void NES::drawScreen() {
	screen = ppu.drawScreen();
	SDL_UpdateTexture(texture, NULL, screen, 1024);
	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, texture, NULL, NULL);
	SDL_RenderPresent(renderer);
}

void NES::screenInit() {
	SDL_Init(SDL_INIT_EVERYTHING);
	window = SDL_CreateWindow("NES", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1024, 960, SDL_WINDOW_RESIZABLE);
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 256, 240);
}

void NES::powerUp() {
	cpu.powerUp();
	ppu.powerUp();
}

void NES::insertCartridge(string path) {
	c.loadCartridge(path);
}

void NES::joypadInput() {
	if (e.type == SDL_KEYDOWN) {
		uint32_t key = e.key.keysym.scancode;
		switch (key) {
		case SDL_SCANCODE_J: //A
			joypad |= 0x80;
			break;
		case SDL_SCANCODE_K: //B
			joypad |= 0x40;
			break;
		case SDL_SCANCODE_Q: //Select
			joypad |= 0x20;
			break;
		case SDL_SCANCODE_RETURN: //Start
			joypad |= 0x10;
			break;
		case SDL_SCANCODE_W: //Up
			joypad |= 0x08;
			break;
		case SDL_SCANCODE_S: //Down
			joypad |= 0x04;
			break;
		case SDL_SCANCODE_A: //Left
			joypad |= 0x02;
			break;
		case SDL_SCANCODE_D: //Right
			joypad |= 0x01;
			break;
		}
	}
	if (e.type == SDL_KEYUP) {
		uint32_t key = e.key.keysym.scancode;
		switch (key) {
		case SDL_SCANCODE_J: //A
			joypad &= ~0x80;
			break;
		case SDL_SCANCODE_K: //B
			joypad &= ~0x40;
			break;
		case SDL_SCANCODE_Q: //Select
			joypad &= ~0x20;
			break;
		case SDL_SCANCODE_RETURN: //Start
			joypad &= ~0x10;
			break;
		case SDL_SCANCODE_W: //Up
			joypad &= ~0x08;
			break;
		case SDL_SCANCODE_S: //Down
			joypad &= ~0x04;
			break;
		case SDL_SCANCODE_A: //Left
			joypad &= ~0x02;
			break;
		case SDL_SCANCODE_D: //Right
			joypad &= ~0x01;
			break;
		}
	}
	c_bus.readJoypad(joypad);
}

void NES::run() {
	screenInit();
	while (1) {
		int c_cycles = 0;
		int l_cycles = 0;
		while (cpu.cyclesDone() < 29781) {
			if (SDL_PollEvent(&e)) {
				if (e.type == SDL_QUIT) {
					return;
				}
				joypadInput();
			}
			if (!ppu.sendNMI()) {
				cpu.NMI();
			}
			cpu.execute();
			//----not perfectly accurate, but this implementation feels the best----
			int c_cycles = cpu.cyclesDone();
			for (int i = c_cycles - l_cycles; i >= 0; i--) {
				ppu.execute2();
				ppu.execute2();
			}
			l_cycles = c_cycles;
			//----------------------------------------------------------------------
		}
		cpu.resetCycles();
		drawScreen();
	}
}
