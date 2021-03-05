#include "Space Invaders.h"

using namespace std;

//invaders.concatenated

int main(int argc, char *argv[]) {
	Space_Invaders game;
	game.loadRAM();
	game.audioInit();
	game.runGame(&game);
	return 0;
}
