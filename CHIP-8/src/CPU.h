#pragma once
#ifndef CPU_H
#define CPU_H

#include "SDL.h"
#include <iostream>

using namespace std;

class CPU {
private:
	//hardware
	uint8_t memory[4096] = { 0 }; //CHIP-8 Memory block, where PC starts at 0x200
	uint8_t V[16] = { 0 };        //Register V0-VF
	uint16_t I = 0;			  //Address Register I
	uint16_t stack[16] = { 0 };   //stack with 24 nested levels
	uint8_t sp = 0;			  //stack pointer
	uint16_t pc = 0x200;		  //program counter

	//font data
	uint8_t fontData[80] = {
		0xF0, 0x90, 0x90, 0x90, 0xF0, //0
		0x20, 0x60, 0x20, 0x20, 0x70, //1
		0xF0, 0x10, 0xF0, 0x80, 0xF0, //2
		0xF0, 0x10, 0xF0, 0x10, 0xF0, //3
		0x90, 0x90, 0xF0, 0x10, 0x10, //4
		0xF0, 0x80, 0xF0, 0x10, 0xF0, //5
		0xF0, 0x80, 0xF0, 0x90, 0xF0, //6
		0xF0, 0x10, 0x20, 0x40, 0x40, //7
		0xF0, 0x90, 0xF0, 0x90, 0xF0, //8
		0xF0, 0x90, 0xF0, 0x10, 0xF0, //9
		0xF0, 0x90, 0xF0, 0x90, 0x90, //A
		0xE0, 0x90, 0xE0, 0x90, 0xE0, //B
		0xF0, 0x80, 0x80, 0x80, 0xF0, //C
		0xE0, 0x90, 0x90, 0x90, 0xE0, //D
		0xF0, 0x80, 0xF0, 0x80, 0xF0, //E
		0xF0, 0x80, 0xF0, 0x80, 0x80  //F
	};

public:
	//Timers
	uint8_t DelayTimer = 0;
	uint8_t SoundTimer = 0;

	//display
	uint8_t display[64 * 32] = { 0 }; //32 rows, 64 columns

		//keyboard Input
	uint8_t keyboard[16] = { 0 };  //hex keyboard 0-F inputs
	// 1 2 3 C -> 1 2 3 4
	// 4 5 6 D -> Q W E R
	// 7 8 9 E -> A S D F
	// A 0 B F -> Z X C V

	//hex map for hex keyboard
	uint8_t hexMap[16] = {
		SDLK_x,
		SDLK_1,
		SDLK_2,
		SDLK_3,
		SDLK_q,
		SDLK_w,
		SDLK_e,
		SDLK_a,
		SDLK_s,
		SDLK_d,
		SDLK_z,
		SDLK_c,
		SDLK_4,
		SDLK_r,
		SDLK_f,
		SDLK_v
	};

	CPU() {}; //constructor

	bool draw = false;

	void init();
	void executeOpcode();
	void loadROM();
	void viewMemoryContents();
	void draw2console();
	void inputHandler();
	void audio(int n);
};

#endif CPU_H
