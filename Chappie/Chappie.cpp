// Chappie.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include <iostream>
#include "chip8.h"
#include <SDL.h>
#include <sstream>

Chip8 chappie;

const Uint32 width = 64;
const Uint32 height = 32;
const int SCREEN_FPS = 60;
const int SCREEN_TICKS_PER_FRAME = 1000 / SCREEN_FPS;
int scale = 8;

SDL_Window *gWindow = nullptr;
SDL_Surface *gSurface = nullptr;

//Free resources and close SDL
void close();

// Update keypad state
void updateKeypad(unsigned char key[])
{
	// Get current state of the keyboard
	const Uint8 *keystates = SDL_GetKeyboardState(NULL);

	key[0x0] = keystates[SDL_SCANCODE_X];
	key[0x1] = keystates[SDL_SCANCODE_1];
	key[0x2] = keystates[SDL_SCANCODE_2];
	key[0x3] = keystates[SDL_SCANCODE_3];
	key[0x4] = keystates[SDL_SCANCODE_Q];
	key[0x5] = keystates[SDL_SCANCODE_W];
	key[0x6] = keystates[SDL_SCANCODE_E];
	key[0x7] = keystates[SDL_SCANCODE_A];
	key[0x8] = keystates[SDL_SCANCODE_S];
	key[0x9] = keystates[SDL_SCANCODE_D];
	key[0xA] = keystates[SDL_SCANCODE_Z];
	key[0xB] = keystates[SDL_SCANCODE_C];
	key[0xC] = keystates[SDL_SCANCODE_4];
	key[0xD] = keystates[SDL_SCANCODE_R];
	key[0xE] = keystates[SDL_SCANCODE_F];
	key[0xF] = keystates[SDL_SCANCODE_V];

}

//Render gfx onto screen
void renderSDL(unsigned char gfx[]) {
	SDL_Rect pixel = { 0, 0, scale, scale };

	// Draw
	for (int y = 0; y < 32; ++y)
	{
		pixel.y = y * scale;
		for (int x = 0; x < 64; ++x)
		{
			pixel.x = x * scale;
			if (gfx[(y * 64) + x] == 0)
				SDL_FillRect(gSurface, &pixel, SDL_MapRGB(gSurface->format, 0, 0, 0));
			else
				SDL_FillRect(gSurface, &pixel, SDL_MapRGB(gSurface->format, 255, 255, 255));
		}
	}

	SDL_UpdateWindowSurface(gWindow);
}

void close()
{
	//Deallocate surface
	SDL_FreeSurface(gSurface);
	gSurface = nullptr;

	//Destroy window
	SDL_DestroyWindow(gWindow);
	gWindow = nullptr;

	//Quit SDL subsystems
	SDL_Quit();
}


int main(int argc, char* args[])
{
	chappie.loadFile("./UFO");

	SDL_Init(SDL_INIT_EVERYTHING);

	gWindow = SDL_CreateWindow("Chappie", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, scale * width, scale * height, 0);
	if (gWindow == nullptr)
	{
		printf("SDL Window unable to be initialized. Error: %s\n", SDL_GetError());
		return 1;
	}

	gSurface = SDL_GetWindowSurface(gWindow);
	if (gSurface == nullptr)
	{
		printf("SDL Surface unable to be initialized. Error: %s\n", SDL_GetError());
		return 1;
	}

	// Main loop flag
	bool quit = false;

	// Event handler
	SDL_Event e;

	// Emulation loop
	while (!quit)
	{

		while (SDL_PollEvent(&e) != 0)
		{
			if (e.type == SDL_QUIT)
			{
				quit = true;
			}

			// If it's the first frame we're pressing a key or the first frame we've lifted up a key, update the keypad
			if (e.type == SDL_KEYUP || e.type == SDL_KEYDOWN)
			{
				updateKeypad(chappie.key);
			}
		}

		// Emulate one cycle
		chappie.emulateCycle();

		// Update keypad

		SDL_Delay(1);

		// If the draw flag is set, update the screen
		if (chappie.drawFlag)
		{
			renderSDL(chappie.gfx);
			chappie.drawFlag = false;
		}
	}

	//Free resources and close SDL
	close();

	return 0;
}