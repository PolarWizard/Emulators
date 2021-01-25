#ifndef SPACE_INVADERS_H
#define SPACE_INVADERS_H
#include "SDL.h"
#include "SDL_mixer.h"
#include "Intel 8080.h"
#include <ctime>

class Space_Invaders: public Intel_8080 {
private:
	uint8_t input_port[4] = { 0x0E, 0x04, 0x00, 0x00 };
	#define iPORT0 input_port[0] //inputs
	#define iPORT1 input_port[1] //inputs
	#define iPORT2 input_port[2] //inputs
	#define iPORT3 input_port[3] //bit shift register read
	uint8_t output_port[5] = { 0x00 };
	#define oPORT2 output_port[0] //shift amount
	#define oPORT3 output_port[1] //sound
	#define oPORT4 output_port[2] //shift data
	#define oPORT5 output_port[3] //sound
	#define oPORT6 output_port[4] //watchdog timer
	Mix_Chunk* soundBank[9];
	#define explosion 0
	#define fastinvader1 1
	#define fastinvader2 2
	#define fastinvader3 3
	#define fastinvader4 4
	#define invaderkilled 5
	#define shoot 6
	#define ufo_highpitch 7
	#define ufo_lowpitch 8
	uint16_t shiftRegister;
	unsigned int displayBuffer[57344]; //57344 bits for display
	uint32_t display60Hz = 0;
	bool halfscreen = 1;
public:
	Space_Invaders() {};
	//Function tells us how to handle data coming into ports from i8080
	void handle_output_data(uint8_t port);
	//Function tells us how to handle data going into i8080 from ports
	void handle_input_data(uint8_t port);
	//run game 
	void runGame(Space_Invaders* i8080);
	//functions that handle dealing with setting/resetting flags
	void loadDisplayBuffer();
	//audio init
	void audioInit();
	//plays specified sound
	void playSound(int id);
};

#endif SPACE_INVADERS_H
