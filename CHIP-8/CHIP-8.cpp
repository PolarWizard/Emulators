// CHIP-8.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <ctime>
#include "SDL.h"
#include "GLFW/glfw3.h"

#include "CPU.h"

using namespace std;

int main() {
	glfwInit();
	GLFWwindow* window = glfwCreateWindow(480, 480, "win", NULL, NULL);
	if (!window) {
		glfwTerminate();
		return 0;
	}
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);
	while (!glfwWindowShouldClose(window)) {
		float ratio;
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		ratio = width / (float)height;
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT);

		glBegin(gl_trian);

		glEnd();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glfwDestroyWindow(window);
	glfwTerminate;
	return  0;
}

/*int main(int argc, char* argv[])
{
	SDL_Init(SDL_INIT_EVERYTHING);
	SDL_Window* window;
	window = SDL_CreateWindow("CHIP-8", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 320, SDL_WINDOW_RESIZABLE);
	if (window == NULL) {
		SDL_GetError();
	}
	SDL_Renderer* renderer;
	renderer = SDL_CreateRenderer(window, -1, 0);
	SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 64, 32);
	SDL_Event event;

	unsigned int pixels[2048] = { 0 };

	double timerClk, cpuClk;

	CPU Chip8;
	Chip8.init();
	Chip8.loadROM();
	//Chip8.viewMemoryContents();
	timerClk = clock();
	cpuClk = clock();
	while (true) {
		if (clock() - cpuClk > 2) {
			Chip8.executeOpcode();
			cpuClk = clock();
		}

		if (clock() - timerClk > 17) {
			if (Chip8.DelayTimer > 0) {
				Chip8.DelayTimer--;
			}
			if (Chip8.SoundTimer > 0) {
				Chip8.audio(0);
				Chip8.SoundTimer--;
			}
			else {
				Chip8.audio(1);
			}
			timerClk = clock();
		}
		if (Chip8.draw) {
			Chip8.draw = false;
			for (int i = 0; i < 2048; i++) {
				if (Chip8.display[i]) {
					pixels[i] = 0xFFFFFFFF;
				}
				else {
					pixels[i] = 0;
				}
			}
			SDL_UpdateTexture(texture, NULL, pixels, 256);
			SDL_RenderClear(renderer);
			SDL_RenderCopy(renderer, texture, NULL, NULL);
			SDL_RenderPresent(renderer);
		}
		if (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				exit(0);
			}
			for (int i = 0; i < 16; ++i) {
				if (event.type == SDL_KEYDOWN) {
					if (event.key.keysym.sym == Chip8.hexMap[i]) {
						Chip8.keyboard[i] = 1;
					}
				}
			}
			for (int i = 0; i < 16; ++i) {
				if (event.type == SDL_KEYUP) {
					if (event.key.keysym.sym == Chip8.hexMap[i]) {
						Chip8.keyboard[i] = 0;
					}
				}
			}
		}
		//Chip8.inputHandler();
	}
	return 0;
}*/
