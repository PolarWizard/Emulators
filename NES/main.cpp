#include "SDL.h"
#include <iomanip>
#include <iostream>
#include "Cartridge.h"
#include "mapper.h"
#include "PPU.h"
#include "MOS_6502.h"
#include "NES.h"
#include <fstream>
using namespace std;

int main(int argc, char* argv[]) {
	NES nes;
	nes.insertCartridge("C:/Users/domin/OneDrive/Desktop/Donkey Kong.nes");
	nes.powerUp();
	nes.run();
	return 0;
}
