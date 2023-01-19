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

const int SCREEN_WIDTH = 1200;
const int SCREEN_HEIGHT = 800;
const int SCREEN_FPS = 60;
const int SCREEN_TICKS_PER_FRAME = 1000 / SCREEN_FPS;
const int MAX_FPS = 120;
double time_scale = 1;
bool LMB_Down = false;

//Globally used font
TTF_Font* gFont = NULL;

//Window
SDL_Window* gWindow = NULL;

//Camera
Camera gCamera({0, 0, -1.5E9}, 1);

//The window renderer
SDL_Renderer* gRenderer = NULL;

//Graphyte
Graphyte graphyte;
TextField* active_text_field = NULL; //This pointer will be used to edit text fields
std::vector<TextField*> text_fields;


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
	if (!graphyte.Init(*gRenderer, *gFont, {SCREEN_WIDTH, SCREEN_HEIGHT, 0}))
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

void commit_to_text_field()
{
	if (active_text_field != NULL)
	{
		active_text_field->Commit();
	}
}

void click(int mX, int mY)
{
	std::cout << "\n" << mX << " " << mY << "\n";
	if (active_text_field != NULL)
	{
		active_text_field->Disable();
	}
	active_text_field = NULL;

	for (TextField* tf : text_fields)
	{
		if (tf->CheckForClick(mX, mY))
		{
			active_text_field = tf;
			printf("\n YOU CLICKED A THING \n");
			return;
		}
	}
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
		CentralBody Sun = CentralBody();
		std::vector<Body> orbiting_bodies;
		
		Body earth = Body("Earth", {0, 1.49E11, 0}, 6.37E6, { 30000, 0, 0 }, Sun, graphyte, false); //152000000000 metres. That number is too large so we have a problem
		Body mercury = Body("Mercury", { 0, 5.06E10, 0 }, 2.44E6, { 47000, 0, 0 }, Sun, graphyte, false);
		Body venus = Body("Venus", { 0, 1E11, 0 }, 6E6, { 35000, 0, 0 }, Sun, graphyte, false);


		orbiting_bodies.emplace_back(earth); 
		orbiting_bodies.emplace_back(mercury);
		orbiting_bodies.emplace_back(venus);

		//DEBUG
		GUI_Block Simulation_Parameters(vector3{ -SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 0});

		Text* text_pm = graphyte.CreateText("__________________\nPERFORMANCE METRICS\n__________________", 24);
		Simulation_Parameters.Add_Stacked_Element(text_pm);

		Text* text_FPS_Display = graphyte.CreateText("TESTING TEXT DEBUG", 10);
		Simulation_Parameters.Add_Stacked_Element(text_FPS_Display);

		Text* text_Vertex_Count_Display = graphyte.CreateText("Vertices", 10);
		Simulation_Parameters.Add_Stacked_Element(text_Vertex_Count_Display);

		Text* text_sp = graphyte.CreateText("__________________\nSIMULATION PARAMETERS\n__________________", 24);
		Simulation_Parameters.Add_Stacked_Element(text_sp);

		//Testing input fields I guess
		Simulation_Parameters.Add_Stacked_Element(graphyte.CreateText("Text Field: ", 10));
		DoubleFieldValue TimeScaleFV(&time_scale);
		TextField* tf = new TextField({ 10,10,0 }, TimeScaleFV, graphyte, std::to_string(time_scale));
		text_fields.push_back(tf);
		Simulation_Parameters.Add_Inline_Element(tf);

		//Mainloop time 
		while (!quit)    
		{ 
			//GRAPHICS 

			//render sun
			Sun.Draw(graphyte, gCamera);

			for (auto& b : orbiting_bodies)
			{

				b.Update_Body(deltaTime, time_scale); // Update body
				//std::cout<<b.Get_Position().Debug()<<"\n"; 
				//b.Calculate_Period();
				b.Draw(graphyte, gCamera);
			}

			double debug_no_pixels = graphyte.Get_Number_Of_Points();
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

				case SDL_TEXTINPUT:
					printf("User is typing: %s\n", sdl_event.text.text);
					if (active_text_field != NULL)
					{
						active_text_field -> Add_Character(sdl_event.text.text);
					}
					break;

				case SDL_KEYDOWN:
					switch (sdl_event.key.keysym.sym)
					{
						case SDLK_BACKSPACE:
							printf("User Pressed the Backspace\n");
							if (active_text_field != NULL)
							{
								active_text_field->Backspace();
							}
							break;

						case SDLK_SPACE:
							printf("User Pressed The Space Bar\n");
							if (time_scale < 1)
							{
								time_scale = 1;
								printf("SET TIME SCALE TO 1 \n");
							}
							else if (time_scale == 1) {
								time_scale = 86400;
								printf("SET TIME SCALE TO 86400 \n");
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

				case SDL_MOUSEBUTTONDOWN:
					if (sdl_event.button.button == SDL_BUTTON_LEFT)
					{
						int mX = 0;
						int mY = 0;
						SDL_GetMouseState(&mX, &mY);
						click(mX - SCREEN_WIDTH / 2, -mY + SCREEN_HEIGHT / 2);
					}
					break;
					
				case SDL_MOUSEWHEEL:
					if (sdl_event.wheel.y > 0) //Scroll up
					{
						//Zoom in
						gCamera.position.z *= 1.4;
						std::cout << "Moved Camera to new pos: " << gCamera.position.z << "\n";
					}
					if (sdl_event.wheel.y < 0) //Scroll down
					{
						//zoom out
						gCamera.position.z /= 1.4;
						std::cout << "Moved Camera to new pos: " << gCamera.position.z << "\n";
					}
					break;
				}
			}

			//DELAY UNTIL END
			deltaTime = Update_Clock();
			float interval = (float)1000 / MAX_FPS;
			if (deltaTime < (Uint32)interval)
			{
				Uint32 delay = (Uint32)interval - deltaTime;
				if (delay > 0)
				{
					SDL_Delay(delay);
				}
			}

			/*DEBUG*/
			float debug_fps = (float)1000 / ((float)deltaTime + 1);
			if (debug_fps > MAX_FPS)
			{
				debug_fps = MAX_FPS;
			}
			text_FPS_Display->Set_Text("FPS: " + std::to_string(debug_fps));
			text_Vertex_Count_Display->Set_Text("Vertex Count: " + std::to_string(debug_no_pixels));
		}	

	}
	//Free resources and close SDL
	close();

	return 0;
}