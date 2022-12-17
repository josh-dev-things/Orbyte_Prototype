#pragma once
#ifndef ORBYTE_GRAPHICS_H
#define ORBYTE_GRAPHICS_H

#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <vector>
#include <string>
#include <numeric>
#include <iostream>

/*
	A "Texture" class is a way of encapsulating the rendering of more complex graphics. Images, fonts etc. would be loaded to a texture.
	Implementation heavily guided by this resource: https://lazyfoo.net/tutorials/SDL/ A series of tutorials regarding creating an application
	using SDL.

	Largely used for fonts in this application.
*/
class GTexture
{
private:
	//The actual hardware texture
	SDL_Texture* mTexture = NULL;

	//The renderer
	SDL_Renderer* renderer = NULL;

	//The font
	TTF_Font* font = NULL;

	//Image dimensions
	int mWidth;
	int mHeight;

public:
	//Constructor
	GTexture(SDL_Renderer* _renderer = NULL, TTF_Font* _font = NULL)
	{
		//Initialize
		renderer = _renderer;
		font = _font;

		mTexture = NULL;
		mWidth = 0;
		mHeight = 0;

	}

	//Deallocates memory
	~GTexture()
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
			newTexture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
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
		SDL_Surface* textSurface = TTF_RenderText_Solid(font, textureText.c_str(), textColor);
		if (textSurface == NULL)
		{
			printf("Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError());
		}
		else
		{
			//Create texture from surface pixels
			mTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
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
			renderer = NULL;
			font = NULL;
			mWidth = 0;
			mHeight = 0;
		}
	}

	//Renders texture at given point
	void render(int x, int y, SDL_Rect* clip = NULL, double angle = 0.0, SDL_Point* center = NULL, SDL_RendererFlip flip = SDL_FLIP_NONE)
	{
		//Set rendering space and render to screen
		SDL_Rect renderQuad = { x, y, mWidth, mHeight };
		SDL_RenderCopy(renderer, mTexture, NULL, &renderQuad);
	}

	//Accessor Methods for retrieving dimensions
	int getWidth()
	{
		return mWidth;
	}

	int getHeight()
	{
		return mHeight;
	}
};

class Text
{
private:
	GTexture texture;

public:
	int pos_x;
	int pos_y;

	Text(std::string str, int font_size, std::vector<int> position, SDL_Renderer* _renderer, TTF_Font* _font, SDL_Color color = { 255, 255, 255 })
	{
		//Constructor for the text class.
		GTexture textTexture(_renderer, _font);
		texture = textTexture;

		if (!texture.loadFromRenderedText("Text Is Working Correctly", color))
		{
			printf("Failed to render text texture!\n");
		}
		pos_x = position[0];
		pos_y = position[1];
	}
};

/*
	Handles all graphics for the application. This includes all pixel writes to the screen; loading and writing to textures; rendering
*/
class Graphyte
{
	private: //Private attributes & Methods

	const float km_per_pixel = 750; //The number of kilometres per pixel on screen.
	const int SCREEN_WIDTH = 800; //What it says on the tin.
	const int SCREEN_HEIGHT = 800; //TOM IS A TWIT

	SDL_Renderer* Renderer = NULL; //Renderer.
	TTF_Font* Font = NULL; //True Type Font. Needs to be loaded at init.
	GTexture TextTexture;

	//std::vector<GTexture> texts; //Vector of text elements to be drawn to the screen.
	std::vector<SDL_Point> points; //Vector of points to be drawn to the screen. Iterate through & draw each point to screen as a pixel.

	public:  //Public attributes & Methods

	bool Init(SDL_Renderer* _renderer, TTF_Font* _font)
	{
		Renderer = _renderer;
		Font = _font;
		if (Renderer == NULL || Font == NULL)
		{
			return false;
		}
		TextTexture = GTexture(Renderer, Font);
		return true;
	}

	//This method instantiates a new Text object and returns it. The new text object will be added to the array of text objects: texts.
	Text CreateText(std::string str, int font_size, SDL_Color color = { 255, 255, 255 })
	{
		Text newText(str, font_size, {0, 0}, Renderer, Font, color);
		//texts.emplace_back(newText);
		return newText;
	}

	void pixel(float x, float y)
	{
		SDL_Point _point = { x + SCREEN_WIDTH / 2, -y + SCREEN_HEIGHT / 2 };

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

	//Draw everything to the screen. Called AFTER all points added to the render queue
	void draw()
	{
		SDL_SetRenderDrawColor(Renderer, 0, 0, 0, 255);
		SDL_RenderClear(Renderer);
		int count = 0;

		for (auto& point : points)
		{
			SDL_SetRenderDrawColor(Renderer, 255, ((float)point.x / (float)SCREEN_WIDTH) * 255, ((float)point.y / (float)SCREEN_HEIGHT) * 255, 255);
			SDL_RenderDrawPoint(Renderer, point.x, point.y); //DRAW TO TEXTURE INSTEAD???
			count++;
		}

		//Make sure you render GUI!
		SDL_SetRenderDrawColor(Renderer, 255, 255, 255, 255);
		TextTexture.render((SCREEN_WIDTH - TextTexture.getWidth()) / 2, (SCREEN_HEIGHT - TextTexture.getHeight()) / 2);

		SDL_RenderPresent(Renderer);
		points.clear();
	}

	void free()
	{
		TextTexture.free();
		//texts.clear();
		points.clear();
	}
};


#endif /*ORBYTE_GRAPHICS_H*/