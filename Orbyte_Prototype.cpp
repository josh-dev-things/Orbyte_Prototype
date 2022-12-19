// Orbyte_Prototype.cpp : This file contains the 'main' function. Program execution begins and ends there.
//  SDL + GUI library built for SDL 
// https://lazyfoo.net/tutorials/SDL/01_hello_SDL/index2.php
// https://lazyfoo.net/tutorials/SDL/index.php

//TODO: SWITCH EVERYTHING TO USING DOUBLES INSTEAD OF FLOATS >:(


#include <iostream>
#include <string>
#include <SDL.h>
#include <stdio.h> //This library makes debugging nicer, but shouldn't really be involved in user usage.
#include <vector>
#include <numeric>
#include "vec3.h"
#include "OrbitBody.h"
#include "Camera.h"
#include <sstream>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include "Orbyte_Data.h"
#include "Orbyte_Graphics.h"

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 800;
const int SCREEN_FPS = 60;
const int SCREEN_TICKS_PER_FRAME = 1000 / SCREEN_FPS;
const float km_per_pixel = 750;
const int MAX_FPS = 60;
float time_scale = 1;
bool LMB_Down = false;

//Globally used font
TTF_Font* gFont = NULL;

//Window
SDL_Window* gWindow = NULL;

//Camera
Camera gCamera({0, 0, -150000}, 1);

//The window renderer
SDL_Renderer* gRenderer = NULL;

//Graphyte
Graphyte graphyte;


//Runtime variables
bool quit = false;
SDL_Event sdl_event;

//Current time start time
Uint32 startTime = 0;
Uint32 deltaTime = 0;



////https://lazyfoo.net/tutorials/SDL/16_true_type_fonts/index.php <-- USE THIS FOR GUI

bool loadMedia()
{
	//success flag
	bool success = true;

	//Open the font
	gFont = TTF_OpenFont("SourceSerifPro-Regular.ttf", 12); //Open_My_Font
	if (gFont == NULL)
	{
		printf("Failed to load lazy font! SDL_ttf Error: %s\n", TTF_GetError());
		success = false;
	}

	return success;
}


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

	//Initialize PNG loading
	int imgFlags = IMG_INIT_PNG;
	if (!(IMG_Init(imgFlags) & imgFlags))
	{
		printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
		return false;
	}

	//Initialize SDL_ttf
	if (TTF_Init() == -1)
	{
		printf("SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError());
		return false;
	}

	//Load media
	if (!loadMedia())
	{
		printf("Failed to load media!\n");
		return false;
	}

	//Starting Graphyte
	if (!graphyte.Init(gRenderer, gFont))
	{
		printf("Graphyte could not initialize!");
		return false;
	}

	////TESTING DATA STORAGE
	OrbitBodyData body_data = OrbitBodyData("name", {1, 1, 1}, 1, {2, 2, 2}, true);
	DataController data_controller;
	data_controller.WriteDataToFile(body_data);
	//gScreenSurface = SDL_GetWindowSurface(gWindow);
	return true;
}

//Frees media and shuts down SDL
void close()
{

	TTF_CloseFont(gFont);
	gFont = NULL;


	//Destroy window
	SDL_DestroyWindow(gWindow);
	SDL_DestroyRenderer(gRenderer);
	gWindow = NULL;
	gRenderer = NULL;

	//Quit SDL subsystems
	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
}

Uint32 Update_Clock() //https://lazyfoo.net/tutorials/SDL/25_capping_frame_rate/index.php
{
	Uint32 current_time = SDL_GetTicks(); //milliseconds
	Uint32 delta = current_time - startTime;
	startTime = current_time;
	return delta; //Put this in a seperate time class. Time.deltaTime etc.
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
		
		//Experimenting with orbit body
		vector3 SUN_POS = { 0, 0, 0 };
		std::vector<Body> orbiting_bodies;
		Body mercury("mercury", {0, 59000, 0}, 2000, { 59000, 0, 0 }, SUN_POS, true);
		Satellite merc_sat("mercury_sat", mercury, { 1200, 0, 0 }, 0.1, {0,0,0}, true);
		mercury.Add_Satellite(merc_sat);
		//mercury.GetBodyData()
		//body venus(0, 0, 108000, 12000, {0, 0, 0}, SUN_POS);//108000
		//body earth(0, 148000, 0, 6000, { 0, 0, 0 }, SUN_POS);
		orbiting_bodies.emplace_back(mercury);
		//orbiting_bodies.emplace_back(venus);
		//orbiting_bodies.emplace_back(earth);
		/*orbiting_bodies.emplace_back(mars);
		orbiting_bodies.emplace_back(jupiter);*/


		//Mainloop time 
		while (!quit)
		{
			//GRAPHICS

			//render sun
			vector3 _ss_sun_pos = gCamera.WorldSpaceToScreenSpace(SUN_POS, SCREEN_HEIGHT, SCREEN_WIDTH);
			if (_ss_sun_pos.z > 0) { graphyte.pixel(_ss_sun_pos.x, _ss_sun_pos.y); }

			for (auto& b : orbiting_bodies)
			{

				b.Update_Body(deltaTime, time_scale); // Update body

				SDL_Color textColor = { 255, 255, 255 };
				//writeText(gTextTexture, b.GetBodyData(), textColor);
				/*writeText(gTextTexture, "_", textColor);*/

				/*
					- Time scale now works! Achieved by multiplying the step size by the same factor that t was multiplied by.
					Why does this work? NO IDEA.
					- BREAKS FOR ANYTHING NEAR 100. I THINK TIME SCALE 10 WORKS BEST
				*/
				b.Draw(graphyte, gCamera);
			}

			graphyte.draw();
			//END GRAPHICS


			int current_mouse_x = 0;
			int current_mouse_y = 0;
			//Handle events
			while (SDL_PollEvent(&sdl_event) != 0)
			{
				switch (sdl_event.type)
				{
				default:
					break;

				case SDL_QUIT:
					quit = true;
					break;

				case SDL_KEYDOWN:
					switch (sdl_event.key.keysym.sym)
					{
						case SDLK_SPACE:
							printf("User Pressed The Space Bar\n");
							if (time_scale < 1)
							{
								time_scale = 1;
								printf("SET TIME SCALE TO 1 \n");
							}
							else if (time_scale == 1) {
								time_scale = 10;
								printf("SET TIME SCALE TO 10 \n");
							}
							else if (time_scale > 1) {
								time_scale = 0.1;
								printf("SET TIME SCALE TO 0.1 \n");
							}
							break;

						case SDLK_UP:
							//Rotate Up
							gCamera.RotateCamera({ 0.01, 0, 0 });
							break;

						case SDLK_DOWN:
							gCamera.RotateCamera({ -0.01, 0, 0 });
							break;

						case SDLK_LEFT:
							//Rotate Up
							gCamera.RotateCamera({ 0, -0.01, 0 });
							break;

						case SDLK_RIGHT:
							gCamera.RotateCamera({ 0, 0.01, 0 });
							break;
					}
					break;

				/*case SDL_MOUSEMOTION:
					SDL_GetMouseState(&current_mouse_x, &current_mouse_y);
					break;

				case SDL_MOUSEBUTTONDOWN:
					if (sdl_event.button.button == SDL_BUTTON_LEFT)
					{
						LMB_Down = true;
						gCamera.Start_Rotate(current_mouse_x, current_mouse_y);
					}
					
				case SDL_MOUSEBUTTONUP:
					if (sdl_event.button.button == SDL_BUTTON_LEFT)
					{
						LMB_Down = false;
						gCamera.End_Rotate(current_mouse_x, current_mouse_y);
					}*/
					
				case SDL_MOUSEWHEEL:
					if (sdl_event.wheel.y > 0) //Scroll up
					{
						//Zoom in
						gCamera.position.z += 10000;
					}
					if (sdl_event.wheel.y < 0) //Scroll down
					{
						//zoom out
						gCamera.position.z -= 10000;
					}
					break;
				}
			}

			/*if (LMB_Down)
			{
				gCamera.End_Rotate(current_mouse_x, current_mouse_y);
			}*/

			//DELAY UNTIL END
			deltaTime = Update_Clock();
			float interval = (float)1000 / MAX_FPS;
			if (deltaTime < (Uint32)interval)
			{
				Uint32 delay = (Uint32)interval - deltaTime;
				if (delay > 0)
				{
					SDL_Delay(delay);
					//std::cout << (float)1000 / ((float)deltaTime + 1) << " FPS" << "\n";
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
