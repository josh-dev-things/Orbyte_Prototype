// Orbyte_Prototype.cpp : This file contains the 'main' function. Program execution begins and ends there.
//  SDL + GUI library built for SDL 
// https://lazyfoo.net/tutorials/SDL/01_hello_SDL/index2.php
// https://lazyfoo.net/tutorials/SDL/index.php

//Note: Skipping loading pngs since we dont want textures. Everything will be done in code.

//TODO: Lesson 08, Rendering Geometry [DONE]
//TODO: Lesson 09, Viewport Stuff


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

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 800;
const int SCREEN_FPS = 60;
const int SCREEN_TICKS_PER_FRAME = 1000 / SCREEN_FPS;
const float km_per_pixel = 750;
const int MAX_FPS = 60;
float time_scale = 1;

//Window
SDL_Window* gWindow = NULL;

//Camera

Camera gCamera({0, 0, -1000000}, 1);

//The window renderer
SDL_Renderer* gRenderer = NULL;

//Bastion Graphics Stuff
std::vector<SDL_Point> points;

//Runtime variables
bool quit = false;
SDL_Event sdl_event;

//Current time start time
Uint32 startTime = 0;
Uint32 deltaTime = 0;

//DEBUGGING


///FUNCTIONS FOR GRAPHICS https://www.youtube.com/watch?v=kdRJgYO1BJM

void pixel(float x, float y)
{
	SDL_Point _point = { x + SCREEN_WIDTH / 2, -y + SCREEN_HEIGHT / 2};

	points.emplace_back(_point);
}

void line(float x1, float y1, float x2, float y2)
{
	float dx = (x2 - x1);
	float dy = (y2 - y1);
	float length = std::sqrt(dx * dx + dy * dy);
	float angle = std::atan2(dy, dx);
	//std::cout << "Drawing line with points: " << length << "\n";
	for (int i = 0; i < length; i++)
	{
		pixel(x1 + std::cos(angle) * i,
			y1 + std::sin(angle) * i);
	}
}

void show()
{
	SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 255);
	SDL_RenderClear(gRenderer);
	int count = 0;

	for (auto& point : points)
	{
		SDL_SetRenderDrawColor(gRenderer, 255, ((float) point.x / (float)SCREEN_WIDTH) * 255, ((float)point.y / (float)SCREEN_HEIGHT) * 255, 255);
		SDL_RenderDrawPoint(gRenderer, point.x, point.y); //DRAW TO TEXTURE INSTEAD???
		count++;
	}
	//std::cout << "Draw points : " << count << "\n";
	SDL_RenderPresent(gRenderer);
	points.clear();
}
///END FUNCTIONS FOR GRAPHICS






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

	//gScreenSurface = SDL_GetWindowSurface(gWindow);
	return true;
}

bool loadMedia()
{
	//Loading success flag
	bool success = true;

	//Nothing to load
	return success;
} //I dont understand the purpose of this I'll be honest!

//Frees media and shuts down SDL
void close()
{
	//SDL_FreeSurface(gStretchedSurface);
	//gStretchedSurface = NULL;
	//SDL_DestroyTexture(gTexture);
	//gTexture = NULL;

	points.clear();


	//Destroy window
	SDL_DestroyWindow(gWindow);
	SDL_DestroyRenderer(gRenderer);
	gWindow = NULL;
	gRenderer = NULL;

	//Quit SDL subsystems
	
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
		//Load media
		if (false)
		{
			printf("Failed to load media!\n");
		}
		else
		{
			//Experimenting with orbit body
			vector3 SUN_POS = { 0, 0, 0 };
			std::vector<body> orbiting_bodies;
			body mercury(0, 59000, 0, 2000, { 0, 0, 0 }, SUN_POS);
			body venus(0, 0, 108000, 6000, { 0, 0, 0 }, SUN_POS);//108000
			body earth(0, 148000, 0, 6000, { 0, 0, 0 }, SUN_POS);
			body mars(0, 222000, 0, 3000, { 0, 0, 0 }, SUN_POS);
			body jupiter(0, 740000, 0, 70000, { 0, 0, 0 }, SUN_POS);
			orbiting_bodies.emplace_back(mercury);
			orbiting_bodies.emplace_back(venus);
			orbiting_bodies.emplace_back(earth);
			orbiting_bodies.emplace_back(mars);
			//orbiting_bodies.emplace_back(jupiter);


			//Mainloop time 
			while (!quit)
			{
				//GRAPHICS

				//render sun
				pixel(SUN_POS.x, SUN_POS.y);

				for (auto& b : orbiting_bodies)
				{

					b.Update_Body(deltaTime, time_scale); // Update body
					/*
						- Time scale now works! Achieved by multiplying the step size by the same factor that t was multiplied by.
						Why does this work? NO IDEA.
						- BREAKS FOR ANYTHING NEAR 100. I THINK TIME SCALE 10 WORKS BEST
					*/
					std::vector<vector3> test_verts = b.Get_Vertices();
					std::vector<edge> test_edges = b.Get_Edges();
					for (auto& p : test_verts)
					{
						p = gCamera.WorldSpaceToScreenSpace(p, SCREEN_HEIGHT, SCREEN_WIDTH);
						//std::cout << "Debug Points: " << p.x << ", " << p.y << ", " << p.z << "\n";
					}
					for (auto& p : test_verts)
					{
						pixel(p.x, p.y);
					}
					for (auto p_t : b.trail_points)
					{
						p_t = gCamera.WorldSpaceToScreenSpace(p_t, SCREEN_HEIGHT, SCREEN_WIDTH);
						//std::cout << "Debug Points: " << p_t.x << ", " << p_t.y << ", " << p_t.z << "\n";
						pixel(p_t.x, p_t.y);
					}
					for (auto& edg : test_edges)
					{
						line(test_verts[edg.a].x,
							test_verts[edg.a].y,
							test_verts[edg.b].x,
							test_verts[edg.b].y);
					}
					test_edges.clear();
					test_verts.clear();
				}

				show();
				//END GRAPHICS



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
							if (time_scale < 1)
							{
								time_scale = 1;
								printf("SET TIME SCALE TO 1 \n");
							}
							else if(time_scale == 1) {
								time_scale = 10;
								printf("SET TIME SCALE TO 10 \n");
							}
							else if (time_scale > 1) {
								time_scale = 0.1;
								printf("SET TIME SCALE TO 0.1 \n");
							}
							break;
						}
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
						//std::cout << (float)1000 / ((float)deltaTime + 1) << " FPS" << "\n";
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
