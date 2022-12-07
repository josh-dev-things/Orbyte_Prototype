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
#include <SDL_ttf.h>
#include <SDL_image.h>


const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 800;
const int SCREEN_FPS = 60;
const int SCREEN_TICKS_PER_FRAME = 1000 / SCREEN_FPS;
const float km_per_pixel = 7500;
const int MAX_FPS = 60;
float time_scale = 1;
bool LMB_Down = false;

//Globally used font
TTF_Font* gFont = NULL;

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

//Texture wrapper class
class LTexture
{
public:
	//Initializes variables
	LTexture()
	{
		//Initialize
		mTexture = NULL;
		mWidth = 0;
		mHeight = 0;
	}

	//Deallocates memory
	~LTexture()
	{
		//Deallocate
		free();
	}

	//Loads image at specified path
	bool loadFromFile(std::string path)
	{
		//Get rid of preexisting texture
		free();
		//The final texture
		SDL_Texture* newTexture = NULL;

		//Load image at specified path
		SDL_Surface* loadedSurface = IMG_Load(path.c_str());
		if (loadedSurface == NULL)
		{
			printf("Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError());
		}
		else
		{
			//Color key image
			//SDL_SetColorKey(loadedSurface, SDL_TRUE, SDL_MapRGB(loadedSurface->format, 0, 0xFF, 0xFF));
			//Create texture from surface pixels
			newTexture = SDL_CreateTextureFromSurface(gRenderer, loadedSurface);
			if (newTexture == NULL)
			{
				printf("Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError());
			}
			else
			{
				//Get image dimensions
				mWidth = loadedSurface->w;
				mHeight = loadedSurface->h;
			}

			//Get rid of old loaded surface
			SDL_FreeSurface(loadedSurface);
		}

		//Return success
		mTexture = newTexture;
		return mTexture != NULL;
	}

	//Creates image from font string
	bool loadFromRenderedText(std::string textureText, SDL_Color textColor)
	{
		//Get rid of preexisting texture
		free();

		//Render text surface
		SDL_Surface* textSurface = TTF_RenderText_Solid(gFont, textureText.c_str(), textColor);
		if (textSurface == NULL)
		{
			printf("Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError());
		}
		else
		{
			//Create texture from surface pixels
			mTexture = SDL_CreateTextureFromSurface(gRenderer, textSurface);
			if (mTexture == NULL)
			{
				printf("Unable to create texture from rendered text! SDL Error: %s\n", SDL_GetError());
			}
			else
			{
				//Get image dimensions
				mWidth = textSurface->w;
				mHeight = textSurface->h;
			}

			//Get rid of old surface
			SDL_FreeSurface(textSurface);
		}

		//Return success
		return mTexture != NULL;
	}

	//Deallocates texture
	void free()
	{
		//Free texture if it exists
		if (mTexture != NULL)
		{
			SDL_DestroyTexture(mTexture);
			mTexture = NULL;
			mWidth = 0;
			mHeight = 0;
		}
	}

	//Set color modulation
	//void setColor(Uint8 red, Uint8 green, Uint8 blue);


	//Renders texture at given point
	void render(int x, int y, SDL_Rect* clip = NULL, double angle = 0.0, SDL_Point* center = NULL, SDL_RendererFlip flip = SDL_FLIP_NONE)
	{
		//Set rendering space and render to screen
		SDL_Rect renderQuad = { x, y, mWidth, mHeight };
		SDL_RenderCopy(gRenderer, mTexture, NULL, &renderQuad);
	}

	//Gets image dimensions
	int getWidth()
	{
		return mWidth;
	}
	int getHeight()
	{
		return mHeight;
	}

private:
	//The actual hardware texture
	SDL_Texture* mTexture;

	//Image dimensions
	int mWidth;
	int mHeight;
};

//Rendered texture
LTexture gTextTexture;


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
	//std::cout << "Dtytytyints : " << count << "\n";

	//Make sure you render GUI!
	SDL_SetRenderDrawColor(gRenderer, 255, 255, 255, 255);
	gTextTexture.render((SCREEN_WIDTH - gTextTexture.getWidth()) / 2, (SCREEN_HEIGHT - gTextTexture.getHeight()) / 2);

	SDL_RenderPresent(gRenderer);
	points.clear();
}
///END FUNCTIONS FOR GRAPHICS



////https://lazyfoo.net/tutorials/SDL/16_true_type_fonts/index.php <-- USE THIS FOR GUI


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

	//gScreenSurface = SDL_GetWindowSurface(gWindow);
	return true;
}

bool loadMedia()
{
	//Loading success flag
	bool success = true;

	//Open the font
	gFont = TTF_OpenFont("SourceSerifPro-Regular.ttf", 6); //Open_My_Font
	if (gFont == NULL)
	{
		printf("Failed to load lazy font! SDL_ttf Error: %s\n", TTF_GetError());
		success = false;
	}
	else
	{
		//Render text
		SDL_Color textColor = { 255, 255, 255 };
		if (!gTextTexture.loadFromRenderedText("poiuytrefghjuiy7tfghu", textColor))	
		{
			printf("Failed to render text texture!\n");
			success = false;
		}
	}

	return success;
}

void writeText(LTexture& tex, std::string text_to_write, SDL_Color color)
{
	//std::cout << text_to_write;
	
	tex.loadFromRenderedText(text_to_write.c_str(), color);
}

//Frees media and shuts down SDL
void close()
{
	//SDL_Freweweface(gStretchedSurface);
	//gStretchedSurface = NULL;
	//SDL_DestroyTexture(gTexture);
	//gTexture = NUL;

	gTextTexture.free();
	TTF_CloseFont(gFont);
	gFont = NULL;

	points.clear();


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
		//Load media
		if (!loadMedia())
		{
			printf("Failed to load media!\n");
		}
		else
		{
			//Experimenting with orbit body
			vector3 SUN_POS = { 0, 0, 0 };
			std::vector<body> orbiting_bodies;
			body mercury("mercury", 0, 59000, 0, 2000, {59000, 0, 0}, SUN_POS, false);
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
				pixel(SUN_POS.x, SUN_POS.y);

				for (auto& b : orbiting_bodies)
				{

					b.Update_Body(deltaTime, time_scale); // Update body

					SDL_Color textColor = { 255, 255, 255 };
					writeText(gTextTexture, b.GetBodyData(), textColor);

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
						if (p.z > 0)
						{
							pixel(p.x, p.y);
						}
					}
					for (auto p_t : b.trail_points)
					{
						p_t = gCamera.WorldSpaceToScreenSpace(p_t, SCREEN_HEIGHT, SCREEN_WIDTH);
						//std::cout << "Debug wasds: " << p_t.x << ", " << p_t.y << ", " << p_t.z << "\n";
						if (p_t.z > 0)
						{
							pixel(p_t.x, p_t.y);
						}
					}
					for (auto& edg : test_edges)
					{
						if(test_verts[edg.a].z > 0 && test_verts[edg.b].z > 0)
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
