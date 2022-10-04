// Orbyte_Prototype.cpp : This file contains the 'main' function. Program execution begins and ends there.
//  SDL + GUI library built for SDL 
// https://lazyfoo.net/tutorials/SDL/01_hello_SDL/index2.php


#include <iostream>
#include <string>
#include <SDL.h>
#include <stdio.h> //This library makes debugging nicer, but shouldn't really be involved in user usage.

const int SCREEN_WIDTH = 700;
const int SCREEN_HEIGHT = 500;

int main(int argc, char* args[])
{
	SDL_Window* sdl_window = NULL;
	SDL_Surface* sdl_surface = NULL;

	//Initialize SDL: https://lazyfoo.net/tutorials/SDL/01_hello_SDL/index2.php
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("ERROR INITIALIZING SDL | SDL_ERROR : %s\n", SDL_GetError()); //printf = print format
		return -1;
	}

	//Now creating window
	sdl_window = SDL_CreateWindow("Orbyte Prototype", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	if (sdl_window == NULL)
	{
		printf("ERROR CREATING WINDOW | SDL_ERROR : %s\n", SDL_GetError());
		return -1;
	}

	//Creating window surface 
	sdl_surface = SDL_GetWindowSurface(sdl_window);

	//MAKE SOMETHING DISPLAY
	SDL_FillRect(sdl_surface, NULL, SDL_MapRGB(sdl_surface->format, 0xFF, 0xFF, 0xFF)); //Fills rect white, because FF for all RGB values in hex is white

	SDL_UpdateWindowSurface(sdl_window); //Update the window

	//Hack to get window to stay up
	SDL_Event e; bool quit = false; while (quit == false) { while (SDL_PollEvent(&e)) { if (e.type == SDL_QUIT) quit = true; } } //NOT MY IDEA

	//Destroy window
	SDL_DestroyWindow(sdl_window);

	//Quit SDL
	SDL_Quit();

	return 0; // all has gone well!
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
