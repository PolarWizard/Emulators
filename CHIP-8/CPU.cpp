#include "CPU.h"

#include <iostream>
#include <iomanip>
#include <fstream>


int byte;
unsigned char* element;
unsigned int* buffer;
unsigned int* finalBuffer;
SDL_Event event;

void func_callback(void* unused, Uint8* stream, int len) {

	for (int i = 0; i < len; i++) {
		stream[i] = (uint8_t)32767*sin(i*6.28);
	}
}

void CPU::audio(int n) {
	SDL_AudioSpec beep;
	beep.freq = 20000;
	beep.format = AUDIO_S8;
	beep.samples = 40;
	beep.callback = func_callback;
	beep.userdata = NULL;
	beep.channels = 1;
	SDL_OpenAudio(&beep, NULL);
	SDL_PauseAudio(n);
};

void CPU::init() {
	pc = 0x200;
	I = 0;
	sp = 0;
	for (int i = 0; i < 2048; ++i) {
		display[i] = 0;
	}
	for (int i = 0; i < 16; ++i) {
		stack[i] = 0;
		keyboard[i] = 0;
		V[i] = 0;
	}
	for (int i = 0; i < 4096; ++i) {
		memory[i] = 0;
	}
	for (int i = 0; i < 80; ++i) {
		memory[i] = fontData[i];
	}
	DelayTimer = 0;
	SoundTimer = 0;
};

void CPU::viewMemoryContents() {
	for (int i = 0; i < byte; i++) {
		cout << hex << setw(2) << setfill('0') << buffer[i] << endl;
	}
}

void CPU::loadROM() {
	ifstream myfile;
	myfile.open("C:/Users/domin/OneDrive/Desktop/CHIP-8/ROMs/Space Invaders.ch8", ios::binary | ios::ate);
	if (!myfile.is_open()) {
		cout << "failed to open";
		return;
	}
	else {
		byte = myfile.tellg();
		myfile.seekg(0, ios::beg);
		element = new unsigned char[byte];
		myfile.read((char*)element, byte);
	}
	buffer = new unsigned int[byte];
	for (int i = 0; i < byte; ++i) {
		buffer[i] = (unsigned int)element[i];
	}
	for (int i = 0; i < 80; i++) {
		memory[i] = fontData[i];
	}
	for (int i = 0; i < byte; ++i) {
		memory[0x200 + i] = buffer[i];
	}
}

void CPU::inputHandler() {
	if (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT) {
			exit(0);
		}
		for (int i = 0; i < 16; ++i) {
			if (event.type == SDL_KEYDOWN) {
				if (event.key.keysym.sym == hexMap[i]) {
					keyboard[i] = 1;
				}
			}
		}
		for (int i = 0; i < 16; ++i) {
			if (event.type == SDL_KEYUP) {
				if (event.key.keysym.sym == hexMap[i]) {
					keyboard[i] = 0;
				}
			}
		}
	}
}

void CPU::executeOpcode() {
	uint16_t opcode = (memory[pc] << 8) | memory[pc + 1];
	cout << hex << setw(4) << setfill('0') << "opcode: " << opcode << " pc: " << pc << endl;;  //prints the current opcode
	uint16_t msb = opcode & 0xF000; //most significant bit
	switch (msb) {
	case 0x0000: {
		if (opcode == 0x00E0) {
			for (uint16_t i = 0; i < 64 * 32; ++i) {
				display[i] = 0;
			}
			pc += 2;
		}
		else if (opcode == 0x00EE) {
			pc = stack[--sp];
			pc += 2;
		}
		else {
			//cout << "Unrecognized opcode: " << hex << setw(4) << setfill('0') << opcode << endl;
			pc += 2;
		}
		break;
	}
	case 0x1000: {
		pc = opcode & 0x0FFF;
		break;
	}
	case 0x2000: {
		stack[sp++] = pc;
		pc = opcode & 0x0FFF;
		break;
	}
	case 0x3000: {
		if (V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF)) {
			pc += 4;
		}
		else {
			pc += 2;
		}
		break;
	}
	case 0x4000: {
		if (V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF)) {
			pc += 4;
		}
		else {
			pc += 2;
		}
		break;
	}
	case 0x5000: {
		if (V[(opcode & 0x0F00) >> 8] == V[(opcode & 0x00F0) >> 4]) {
			pc += 4;
		}
		else {
			pc += 2;
		}
		break;
	}
	case 0x6000: {
		V[(opcode & 0x0F00) >> 8] = (uint8_t)(opcode & 0x00FF);
		pc += 2;
		break;
	}
	case 0x7000: {
		V[(opcode & 0x0F00) >> 8] += (uint8_t)(opcode & 0x00FF);
		pc += 2;
		break;
	}
	case 0x8000: {
		uint16_t lsb = opcode & 0x000F;
		switch (lsb) {
		case 0x0000: {
			V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;
		}
		case 0x0001: { //Vx = Vx | Vy
			V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] | V[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;
		}
		case 0x0002: { //Vx = Vx & Vy
			V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] & V[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;
		}
		case 0x0003: { //Vx = Vx ^ Vy
			V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] ^ V[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;
		}
		case 0x0004: { //Vx = Vx + Vy, VF = 1 when overflow else 0
			V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x00F0) >> 4];
			if (((int)V[(opcode & 0x0F00) >> 8] + (int)V[(opcode & 0x00F0) >> 4]) > 255) {
				V[15] = (uint8_t)1;
			}
			else {
				V[15] = (uint8_t)0;
			}
			pc += 2;
			break;
		}
		case 0x0005: {//Vx = Vx - Vy, VF = 0 when borrow else 1
			V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x00F0) >> 4];
			if (V[(opcode & 0x0F00) >> 8] < V[(opcode & 0x00F0) >> 4]) {
				V[15] = (uint8_t)1;
			}
			else {
				V[15] = (uint8_t)0;
			}
			pc += 2;
			break;
		}
		case 0x0006: {//Stores lsb of Vx into VF, then shift Vx to the right by 1
			V[15] = V[(opcode & 0x0F00) >> 8] & 0x01;
			V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] >> 1;
			pc += 2;
			break;
		}
		case 0x0007: { //Vx = Vy - Vx, VF = 0 when borrow else 1
			V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] - V[(opcode & 0x0F00) >> 8];
			if (V[(opcode & 0x00F0) >> 4] >= V[(opcode & 0x0F00) >> 8]) {
				V[15] = (uint8_t)1;
			}
			else {
				V[15] = (uint8_t)0;
			}
			pc += 2;
			break;
		}
		case 0x000E: { //Stores msb of Vx into VF, then shift Vx to the left by 1
			V[15] = (V[(opcode & 0x0F00) >> 8] >> 7);
			V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] << 1;
			pc += 2;
			break;
		}
		default: {
			//cout << "unrecognized opcode: " << hex << setw(4) << setfill('0') << opcode << endl;
			break;
		}
		}
		break;
	}
	case 0x9000: {
		if (V[(opcode & 0x0F00) >> 8] != V[(opcode & 0x00F0) >> 4]) {
			pc += 4;
		}
		else {
			pc += 2;
		}
		break;
	}
	case 0xA000: {
		I = opcode & 0x0FFF;
		pc += 2;
		break;
	}
	case 0xB000: {
		pc = V[0] + (opcode & 0x0FFF);
		break;
	}
	case 0xC000: {
		V[(opcode & 0x0F00) >> 8] = (rand() % 256) & (opcode & 0x00FF);
		pc += 2;
		break;
	}
	case 0xD000: {
		int x = V[(opcode & 0x0F00) >> 8];
		int y = V[(opcode & 0x00F0) >> 4];
		int n = (opcode & 0x000F);
		V[15] = 0;
		for (int i = 0; i < n; i++) {  //tells me row
			uint8_t pixelRow = memory[I + i];
			for (int j = 0; j < 8; j++) { //tells me column
				uint8_t pixel = (pixelRow >> (7 - j)) & 0x01;
				uint16_t index = (x + j + (y + i) * 64);
				if (pixel == 1 && display[index] == 1) {
					V[15] = 1;
				}
				display[index] = display[index] ^ pixel;
				draw = true;
			}
		}
		
		pc += 2;
		break;
	}
	case 0xE000: {
		uint16_t byte = opcode & 0x00FF;
		switch (byte) {
		case 0x009E: {
			if (keyboard[V[(opcode & 0x0F00) >> 8]] == 1) {
				pc += 4;
			}
			else {
				pc += 2;
			}
			break;
		}
		case 0x00A1: {
			if (keyboard[V[(opcode & 0x0F00) >> 8]] == 0) {
				pc += 4;
			}
			else {
				pc += 2;
			}
			break;
		}
		default: {
			cout << "unrecognized opcode: " << hex << setw(4) << setfill('0') << opcode << endl;
			break;
		}
		}
	}
	case 0xF000: {
		uint8_t byte = opcode & 0x00FF;
		switch (byte) {
		case 0x0007: {
			V[(opcode & 0x0F00) >> 8] = DelayTimer;
			pc += 2;
			break;
		}
		case 0x000A: {
			bool keyPress = false;
			for (int i = 0; i < 16; ++i)
			{
				if (keyboard[i] != 0)
				{
					V[(opcode & 0x0F00) >> 8] = i;
					keyPress = true;
				}
			}
			if (!keyPress) {
				return;
			}
			pc += 2;
			break;
		}
		case 0x0015: {
			DelayTimer = V[(opcode & 0x0F00) >> 8];
			pc += 2;
			break;
		}
		case 0x0018: {
			SoundTimer = V[(opcode & 0x0F00) >> 8];
			pc += 2;
			break;
		}
		case 0x001E: {
			I = I + V[(opcode & 0x0F00) >> 8];
			pc += 2;
			break;
		}
		case 0x0029: {
			I = V[(opcode & 0x0F00) >> 8] * 5;
			pc += 2;
			break;
		}
		case 0x0033: {
			memory[I] = V[(opcode & 0x0F00) >> 8] / 100;
			memory[I + 1] = (V[(opcode & 0x0F00) >> 8] / 10) % 10;
			memory[I + 2] = V[(opcode & 0x0F00) >> 8] % 10;
			pc += 2;
			break;
		}
		case 0x0055: {
			for (int i = 0; i <= ((opcode & 0x0F00) >> 8); i++) {
				memory[I + i] = V[i];
			}
			pc += 2;
			break;
		}
		case 0x0065: {
			for (int i = 0; i <= ((opcode & 0x0F00) >> 8); i++) {
				V[i] = memory[I + i];
			}
			pc += 2;
			break;
		}
		}

	}
	default: {
		//cout << "unrecognized opcode: " << hex << setw(4) << setfill('0') << opcode << endl;
		break;
	}
	}
};


void CPU::draw2console() {
	for (int y = 0; y < 32; y++) {
		for (int x = 0; x < 64; x++) {
			if (display[x + y * 64] != 0) {
				cout << "*";
			}
			else {
				cout << " ";
			}
		}
		cout << endl;
	}
}