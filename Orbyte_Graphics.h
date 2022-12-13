#pragma once
#ifndef ORBYTE_GRAPHICS_H
#define ORBYTE_GRAPHICS_H

#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <vector>
#include <string>

/*
	Handles all graphics for the application. This includes all pixel writes to the screen; loading and writing to textures; rendering
*/
class Graphyte
{
	private: //Private attributes & Methods

	const float km_per_pixel = 750; //The number of kilometres per pixel on screen.
	const int SCREEN_WIDTH = 800; //What it says on the tin.
	const int SCREEN_Height = 800;

	SDL_Renderer* Renderer = NULL; //Renderer.
	TTF_Font* Font = NULL; //True Type Font. Needs to be loaded at init.
	Texture TextTexture;

	std::vector<SDL_Point> points; //Vector of points to be drawn to the screen. Iterate through & draw each point to screen as a pixel.

	public:  //Public attributes & Methods

	//Constructor. 
	Graphyte(SDL_Renderer* _renderer, TTF_Font* _font)
	{
		Renderer = _renderer;
		Font = _font;
		TextTexture = Texture(Renderer, Font);
	}

	bool LoadMedia()
	{
		//success flag
		bool success = true;

		//Open the font
		Font = TTF_OpenFont("SourceSerifPro-Regular.ttf", 12); //Open_My_Font
		if (Font == NULL)
		{
			printf("Failed to load lazy font! SDL_ttf Error: %s\n", TTF_GetError());
			success = false;
		}
		else
		{
			//Render text
			SDL_Color textColor = { 255, 255, 255 };
			if (!TextTexture.loadFromRenderedText("Text Is Working Correctly", textColor))
			{
				printf("Failed to render text texture!\n");
				success = false;
			}
		}

		return success;
	}
};

class Text
{
private:
	Texture texture;

public:
	Text(std::string str, int font_size, SDL_Color color = { 255, 255, 255 })
	{
		//Constructor for the text class.
	}
};

/*
	A "Texture" class is a way of encapsulating the rendering of more complex graphics. Images, fonts etc. would be loaded to a texture.
	Implementation heavily guided by this resource: https://lazyfoo.net/tutorials/SDL/ A series of tutorials regarding creating an application
	using SDL.

	Largely used for fonts in this application.
*/
class Texture
{
private:
	//The actual hardware texture
	SDL_Texture* mTexture;

	//The renderer
	SDL_Renderer* renderer = NULL;

	//The font
	TTF_Font* font = NULL;

	//Image dimensions
	int mWidth;
	int mHeight;

public:
	//Constructor
	Texture(SDL_Renderer* _renderer = NULL, TTF_Font* _font = NULL)
	{
		//Initialize
		renderer = _renderer;
		font = _font;

		mTexture = NULL;
		mWidth = 0;
		mHeight = 0;
		
	}

	//Deallocates memory
	~Texture()
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

#endif /*ORBYTE_GRAPHICS_H*/