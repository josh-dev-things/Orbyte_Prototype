// Orbyte_Prototype.cpp : This file contains the 'main' function. Program execution begins and ends there.
//  SDL + GUI library built for SDL 
// https://lazyfoo.net/tutorials/SDL/01_hello_SDL/index2.php
// https://lazyfoo.net/tutorials/SDL/index.php

//Note: Skipping loading pngs since we dont want textures. Everything will be done in code.

//TODO: Lesson 08, Rendering Geometry [DONE]


#include <iostream>
#include <string>
#include <SDL.h>
#include <stdio.h> //This library makes debugging nicer, but shouldn't really be involved in user usage.

//Open GL
#include <GL\glew.h>
#include <SDL_opengl.h>
#include <GL\glu.h>

const int SCREEN_WIDTH = 700;
const int SCREEN_HEIGHT = 500;

//Window
SDL_Window* gWindow = NULL;

SDL_Surface* gScreenSurface = NULL;
SDL_Surface* gStretchedSurface = NULL;

//The window renderer
SDL_Renderer* gRenderer = NULL;

//Current displayed texture
SDL_Texture* gTexture = NULL;


SDL_Surface* gHelloWorld = NULL; // "... An SDL surface is just an image data type that contains the pixels of an image along with all data needed to render it"

//Runtime variables
bool quit = false;
SDL_Event sdl_event;

//Initialize SDL and window
bool init()
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("ERROR INITIALIZING SDL | SDL_ERROR : %s\n", SDL_GetError());
		return false;
	}

	//Creating the window
	gWindow = SDL_CreateWindow("Orbyte Prototype", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	if (gWindow == NULL)
	{
		printf("ERROR CREATING WINDOW | SDL_ERROR: %s\n", SDL_GetError());
		return false;
	}

	//Create the SDL Renderer
	gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);
	if (gRenderer == NULL)
	{
		printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
		return false;
	}
	else
	{
		//Initialize renderer color
		SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
	} //Not loading PNGs because that'd be a pain to implement.

	gScreenSurface = SDL_GetWindowSurface(gWindow);
	return true;
}

bool loadMedia()
{
	//Loading success flag
	bool success = true;

	//Nothing to load
	return success;
} //I dont understand the purpose of this I'll be honest!

//Loads media
SDL_Surface* loadSurface(std::string path)
{
	//The final optimized image
	SDL_Surface* optimizedSurface = NULL;

	//Load image at specified path
	SDL_Surface* loadedSurface = SDL_LoadBMP(path.c_str());
	if (loadedSurface == NULL)
	{
		printf("Unable to load image %s! SDL Error: %s\n", path.c_str(), SDL_GetError());
	}
	else
	{
		//Convert surface to screen format
		optimizedSurface = SDL_ConvertSurface(loadedSurface, gScreenSurface->format, 0);
		if (optimizedSurface == NULL)
		{
			printf("Unable to optimize image %s! SDL Error: %s\n", path.c_str(), SDL_GetError());
		}

		//Get rid of old loaded surface
		SDL_FreeSurface(loadedSurface);
	}

	return optimizedSurface;
}

//Frees media and shuts down SDL
void close()
{
	//Deallocate surface
	SDL_FreeSurface(gHelloWorld);
	gHelloWorld = NULL;
	SDL_FreeSurface(gStretchedSurface);
	gStretchedSurface = NULL;
	SDL_DestroyTexture(gTexture);
	gTexture = NULL;


	//Destroy window
	SDL_DestroyWindow(gWindow);
	SDL_DestroyRenderer(gRenderer);
	gWindow = NULL;
	gRenderer = NULL;

	//Quit SDL subsystems
	
	SDL_Quit();
}

int main(int argc, char* args[])
{
	//Start up SDL and create window
	if (!init())
	{
		printf("Failed to initialize!\n");
	}
	else
	{
		//Load media
		if (false)
		{
			printf("Failed to load media!\n");
		}
		else
		{

			//Apply the image
			/*gStretchedSurface = loadSurface("images/Orbyte.bmp");
			SDL_Rect stretchRect;
			stretchRect.x = 0;
			stretchRect.y = 0;
			stretchRect.w = SCREEN_WIDTH;
			stretchRect.h = SCREEN_HEIGHT;
			SDL_BlitScaled(gStretchedSurface, NULL, gScreenSurface, &stretchRect);*/

			//Mainloop time
			while (!quit)
			{
				//SDL_UpdateWindowSurface(gWindow); //Update surface

				//Clear screen
				SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
				SDL_RenderClear(gRenderer);
				//Render red filled quad
				SDL_Rect fillRect = { SCREEN_WIDTH / 4, SCREEN_HEIGHT / 4, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };
				SDL_SetRenderDrawColor(gRenderer, 0xFF, 0x00, 0x00, 0xFF);
				SDL_RenderFillRect(gRenderer, &fillRect);
				//Render green outlined quad
				SDL_Rect outlineRect = { SCREEN_WIDTH / 6, SCREEN_HEIGHT / 6, SCREEN_WIDTH * 2 / 3, SCREEN_HEIGHT * 2 / 3 };
				SDL_SetRenderDrawColor(gRenderer, 0x00, 0xFF, 0x00, 0xFF);
				SDL_RenderDrawRect(gRenderer, &outlineRect);
				//Draw blue horizontal line
				SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0xFF, 0xFF);
				SDL_RenderDrawLine(gRenderer, 0, SCREEN_HEIGHT / 2, SCREEN_WIDTH, SCREEN_HEIGHT / 2);
				//Draw vertical line of yellow dots
				SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0x00, 0xFF);
				for (int i = 0; i < SCREEN_HEIGHT; i += 4)
				{
					SDL_RenderDrawPoint(gRenderer, SCREEN_WIDTH / 2, i);
				}

				//Update screen
				SDL_RenderPresent(gRenderer);
				//Handle events
				while (SDL_PollEvent(&sdl_event) != 0)
				{
					//User requests Quit
					if (sdl_event.type == SDL_QUIT)
					{
						quit = true;
					}
					//User Presses a Key
					else if (sdl_event.type == SDL_KEYDOWN)
					{
						switch (sdl_event.key.keysym.sym)
						{
						case SDLK_SPACE:
							printf("User Pressed The Space Bar\n");
							break;
						}
					}
				}
			}
		}

	}
	//Free resources and close SDL
	close();

	return 0;
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
