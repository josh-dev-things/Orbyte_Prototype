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
#include "Icosahedron.h"
#include <sstream>

const int SCREEN_WIDTH = 700;
const int SCREEN_HEIGHT = 500;
const int SCREEN_FPS = 60;
const int SCREEN_TICKS_PER_FRAME = 1000 / SCREEN_FPS;

const int MAX_FPS = 60;

//Window
SDL_Window* gWindow = NULL;

SDL_Surface* gScreenSurface = NULL;
SDL_Surface* gStretchedSurface = NULL;

//The window renderer
SDL_Renderer* gRenderer = NULL;

//Current displayed texture
SDL_Texture* gTexture = NULL;

//Bastion Graphics Stuff
std::vector<SDL_Point> points;

//Runtime variables
bool quit = false;
SDL_Event sdl_event;

//Current time start time
Uint32 startTime = 0;

///FUNCTIONS FOR GRAPHICS https://www.youtube.com/watch?v=kdRJgYO1BJM

void rotate(vector3& point, float x = 1, float y = 1, float z = 1)
{
	float rad = 0;

	rad = x;
	point.y = std::cos(rad) * point.y - std::sin(rad) * point.z;
	point.z = std::sin(rad) * point.y + std::cos(rad) * point.z;

	rad = y;
	point.x = std::cos(rad) * point.x - std::sin(rad) * point.z;
	point.z = std::sin(rad) * point.x + std::cos(rad) * point.z;

	rad = z;
	point.x = std::cos(rad) * point.x - std::sin(rad) * point.y;
	point.y = std::sin(rad) * point.x + std::cos(rad) * point.y;
}

void pixel(float x, float y)
{
	SDL_Point _point = { x, y };
	points.emplace_back(_point);
}

void line(float x1, float y1, float x2, float y2)
{
	float dx = x2 - x1;
	float dy = y2 - y1;
	float length = std::sqrt(dx * dx + dy * dy);
	float angle = std::atan2(dy, dx);
	for (float i = 0; i < length; i++)
	{
		pixel(x1 + std::cos(angle) * i,
			y1 + std::sin(angle) * i);
	}
}

void show()
{
	SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 255);
	SDL_RenderClear(gRenderer);

	for (auto& point : points)
	{
		SDL_SetRenderDrawColor(gRenderer, 255, ((float) point.x / (float)SCREEN_WIDTH) * 255, ((float)point.y / (float)SCREEN_HEIGHT) * 255, 255);
		SDL_RenderDrawPoint(gRenderer, point.x, point.y);
	}

	SDL_RenderPresent(gRenderer);
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
			//random points
			for (int i = 0; i < 100; i++)
			{
				pixel(rand() % 640, rand() % 480);
			}

			//square
			line(100, 100, 200, 100);
			line(200, 100, 200, 200);
			line(200, 200, 100, 200);
			line(100, 200, 100, 100);

			std::vector<vector3> cube_points{
				{100,100,100},
				{200,100,100},
				{200,200,100},
				{100,200,100},

				{100,100,200},
				{200,100,200},
				{200,200,200},
				{100,200,200}

			};

			std::vector<edge> cube_edges
			{
				{0, 4},
				{1, 5},
				{2, 6},
				{3, 7},

				{0,1},
				{1,2},
				{2,3},
				{3,0},

				{4,5},
				{5,6},
				{6,7},
				{7,4}
			};

			vector3 centeroid{0,0,0};
			for (auto& p : cube_points)
			{
				centeroid.x += p.x;
				centeroid.y += p.y;
				centeroid.z += p.z;
			}
			centeroid.x /= cube_points.size();
			centeroid.y /= cube_points.size();
			centeroid.z /= cube_points.size();

			//Experimenting with ico
			/*Ico icosohedron(1.0f, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 100);
			std::vector<vector3> ico_points = icosohedron.Get_Vertices();
			std::vector<edge> ico_edges = icosohedron.Get_Edges();
			vector3 ico_centeroid{ icosohedron.x, icosohedron.y, icosohedron.z };*/


			//Mainloop time
			while (!quit)
			{
				//GRAPHICS

				for (auto& p : cube_points)
				{
					p.x -= centeroid.x;
					p.y -= centeroid.y;
					p.z -= centeroid.z;
					float rot = 0.0003f;
					rotate(p, rot, 0.0004, 0.0006);
					p.x += centeroid.x;
					p.y += centeroid.y;
					p.z += centeroid.z;
					pixel(p.x, p.y);
				} 
				for (auto& edg : cube_edges)
				{
					line(cube_points[edg.a].x,
						cube_points[edg.a].y,
						cube_points[edg.b].x,
						cube_points[edg.b].y);
				}
				show();
				points.clear();
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
							break;
						}
					}
				}

				//DELAY UNTIL END
				Uint32 delta = Update_Clock();
				float interval = 1000 / MAX_FPS;
				if (delta < (Uint32)interval)
				{
					Uint32 delay = (Uint32)interval - delta;
					SDL_Delay(delay);
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
