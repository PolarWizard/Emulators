#include "Space Invaders.h"

void Space_Invaders::handle_input_data(uint8_t port) {
	switch (port) {
	case 0:
		A = iPORT0;
		break;
	case 1:
		A = iPORT1;
		break;
	case 2:
		A = iPORT2;
		break;
	case 3:
		iPORT3 = shiftRegister >> (8 - oPORT2);
		A = iPORT3;
		break;
	}
}

void Space_Invaders::handle_output_data(uint8_t port) {
	switch (port) {
	case 2:
		oPORT2 = A  & 0x07;
		break;
	case 3:
		if ((A & 0x01) && !(oPORT3 & 0x01)) {
			playSound(ufo_lowpitch);
		}
		if ((A & 0x02) && !(oPORT3 & 0x02)) {
			playSound(shoot);
		}
		if ((A & 0x04) && !(oPORT3 & 0x04)) {
			playSound(explosion);
		}
		if ((A & 0x08) && !(oPORT3 & 0x08)) {
			playSound(invaderkilled);
		}
		oPORT3 = A;
		break;
	case 4:
		shiftRegister = shiftRegister >> 8;
		shiftRegister |= ((uint16_t)A << 8);
		break;
	case 5:
		if ((A & 0x01) && !(oPORT5 & 0x01)) {
			playSound(fastinvader1);
		}
		if ((A & 0x02) && !(oPORT5 & 0x02)) {
			playSound(fastinvader2);
		}
		if ((A & 0x04) && !(oPORT5 & 0x04)) {
			playSound(fastinvader3);
		}
		if ((A & 0x08) && !(oPORT5 & 0x08)) {
			playSound(fastinvader4);
		}
		if ((A & 0x10) && !(oPORT5 & 0x10)) {
			playSound(ufo_highpitch);
		}
		oPORT5 = A;
		break;
	case 6:
		//watchdog timer port
		break;
	}
}

void Space_Invaders::runGame(Space_Invaders* i8080) {
	SDL_Init(SDL_INIT_EVERYTHING);
	SDL_Window* window = SDL_CreateWindow("T", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 224*5, 256*5, SDL_WINDOW_RESIZABLE);
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
	SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 224, 256);
	SDL_Event e;
	display60Hz = clock();
	while (true) {
		if ((clock() - display60Hz) > 17) {  //Gets us 60 FPS
			display60Hz = SDL_GetTicks();
			while (cycles <= 33334) { //2MHz clock doing 60FPS, 33,333.33 cycles/frame
				uint8_t instruction = RAM[PC];
				if (instruction == 0xDB) { //IN P
					int port = RAM[PC + 1];
					handle_input_data(port);
				}
				else if (instruction == 0xD3) { //OUT P
					int port = RAM[PC + 1];
					handle_output_data(port);
				}
				int a = i8080->execute(i8080);
			}
			if (halfscreen) {
				i8080->IRQ(1); //RST 1
				halfscreen = 0;
			}
			else {
				i8080->IRQ(2); //RST 2
				halfscreen = 1;
			}
			cycles = 0;
			loadDisplayBuffer();
			SDL_UpdateTexture(texture, NULL, displayBuffer, 896);
			SDL_RenderClear(renderer);
			SDL_RenderCopy(renderer, texture, NULL, NULL);
			SDL_RenderPresent(renderer);
		}
		if (SDL_PollEvent(&e)) {
			if (e.type == SDL_SCANCODE_ESCAPE) {
				SDL_DestroyWindow(window);
				SDL_DestroyTexture(texture);
				SDL_DestroyRenderer(renderer);
				for (int i = 0; i < 9; i++) {
					Mix_FreeChunk(soundBank[i]);
				}
				Mix_Quit();
				SDL_Quit();
				return;
			}
			else if (e.type == SDL_KEYDOWN) {
				uint32_t key = e.key.keysym.scancode;
				switch (key) {
				case SDL_SCANCODE_C:
					iPORT1 |= (1 << 0);
					break;
				case SDL_SCANCODE_2:
					iPORT1 |= (1 << 1);
					break;
				case SDL_SCANCODE_1:
					iPORT1 |= (1 << 2);
					break;
				case SDL_SCANCODE_SPACE:
					iPORT1 |= (1 << 4);
					iPORT2 |= (1 << 4);
					break;
				case SDL_SCANCODE_LEFT:
					iPORT1 |= (1 << 5);
					iPORT2 |= (1 << 5);
					break;
				case SDL_SCANCODE_RIGHT:
					iPORT1 |= (1 << 6);
					iPORT2 |= (1 << 6);
					break;
				case SDL_SCANCODE_BACKSPACE:
					iPORT2 |= (1 << 2);
					break;
				}
			}
			else if (e.type == SDL_KEYUP) {
				uint32_t key = e.key.keysym.scancode;
				switch (key) {
				case SDL_SCANCODE_C:
					iPORT1 &= ~(1 << 0);
					break;
				case SDL_SCANCODE_2:
					iPORT1 &= ~(1 << 1);
					break;
				case SDL_SCANCODE_1:
					iPORT1 &= ~(1 << 2);
					break;
				case SDL_SCANCODE_SPACE:
					iPORT1 &= ~(1 << 4);
					iPORT2 &= ~(1 << 4);
					break;
				case SDL_SCANCODE_LEFT:
					iPORT1 &= ~(1 << 5);
					iPORT2 &= ~(1 << 5);
					break;
				case SDL_SCANCODE_RIGHT:
					iPORT1 &= ~(1 << 6);
					iPORT2 &= ~(1 << 6);
					break;
				case SDL_SCANCODE_BACKSPACE:
					iPORT2 &= ~(1 << 2);
					break;
				}
			}
		}
	}
}

void Space_Invaders::loadDisplayBuffer() {
	int data, px, py, baseX, baseY, pix, temp, local;
	for (int a = 0; a < 7168; a++) {
		data = RAM[a + 0x2400];
		baseX = (a % 32) * 8;
		baseY = a / 32;
		for (int i = 0; i < 8; i++) {
			pix = (data >> i) & 0x01;
			px = baseX + i;
			py = baseY;
			temp = px;
			px = py;
			py = -temp + 255;
			local = py * 224 + px;
			if (pix == 1) {
				if ((local >= 7168) && (local <= 14335)) { //RED COLOR LOCATIONS
					displayBuffer[local] = 0xffff0000;
				}
				else if ((local >= 41216) && (local <= 53759)) { //GREEN COLOR 1st LOCATIONS
					displayBuffer[local] = 0xff00ff00;
				}
				else if ((py > 239) && (px > 15) && (px < 136)) { //GREEN COLOR 2nd LOCATIONS
					displayBuffer[local] = 0xff00ff00;
				}
				else {
					displayBuffer[local] = 0xffffffff;
				}
			}
			else {
				displayBuffer[local] = 0;
			}
		}
	}
}

void Space_Invaders::audioInit() {
	Mix_OpenAudio(
		44100,
		MIX_DEFAULT_FORMAT,
		MIX_DEFAULT_CHANNELS,
		1024);
	soundBank[0] = Mix_LoadWAV("Space Invaders sound files/explosion.wav");
	soundBank[1] = Mix_LoadWAV("Space Invaders sound files/fastinvader1.wav");
	soundBank[2] = Mix_LoadWAV("Space Invaders sound files/fastinvader2.wav");
	soundBank[3] = Mix_LoadWAV("Space Invaders sound files/fastinvader3.wav");
	soundBank[4] = Mix_LoadWAV("Space Invaders sound files/fastinvader4.wav");
	soundBank[5] = Mix_LoadWAV("Space Invaders sound files/invaderkilled.wav");
	soundBank[6] = Mix_LoadWAV("Space Invaders sound files/shoot.wav");
	soundBank[7] = Mix_LoadWAV("Space Invaders sound files/ufo_highpitch.wav");
	soundBank[8] = Mix_LoadWAV("Space Invaders sound files/ufo_lowpitch.wav");
}

void Space_Invaders::playSound(int id) {
	Mix_PlayChannel(-1, soundBank[id], 0);
}