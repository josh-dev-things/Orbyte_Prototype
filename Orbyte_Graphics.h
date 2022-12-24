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
	SDL_Texture* mTexture;

	//The renderer
	SDL_Renderer* renderer;

	//The font
	TTF_Font* font;

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

	GTexture(const GTexture& source) : GTexture{&*source.renderer, &*source.font}
	{
		std::cout << "Copy constructor";
	}

	GTexture(GTexture&& source) : mTexture{ source.mTexture }, font{ source.font }, renderer{ source.renderer }, mWidth{ source.mWidth }, mHeight{source.mHeight}
	{
		std::cout << "Move constructor apparently";
		source.mTexture = nullptr;
		source.font = nullptr;
		source.renderer = nullptr;
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
		////Get rid of preexisting texture
		reset_texture();

		//Render text surface
		SDL_Surface* textSurface = TTF_RenderText_Solid(font, textureText.c_str(), textColor);
		if (textSurface == NULL)
		{
			printf("Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError());
			if (font == NULL)
			{
				printf("Font pointer was null\n");
			}
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

	void reset_texture()
	{
		if (mTexture != NULL)
		{
			SDL_DestroyTexture(mTexture);
			mTexture = NULL;
			mWidth = 0;
			mHeight = 0;
		}
	}

	//Deallocates texture
 	void free()
 	{
		//Free texture if it exists
		if (mTexture != NULL)
		{
			//printf("FREE TEXTURE\n");
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
		if (mTexture != NULL)
		{
			SDL_Rect dst;
			dst.x = 0;
			dst.y = 0;
			//Query the texture to get its width and height to use

			SDL_QueryTexture(mTexture, NULL, NULL, &dst.w, &dst.h);
			//printf("Problems accessing text: %s\n", SDL_GetError());
		}
		if (SDL_RenderCopy(renderer, mTexture, NULL, &renderQuad) == -1) {
			//mTexture seems to be a problem => https://stackoverflow.com/questions/25738096/c-sdl2-error-when-trying-to-render-sdl-texture-invalid-texture FIXED
			printf("Problems rendering text: %s\n", SDL_GetError());
		}
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
	GTexture texture = NULL;

public:
	int pos_x; //Position along x axis in screenspace
	int pos_y; //Position along y axis in screenspace
	std::string text;
	bool visible = true;

	Text(std::string str, int font_size, std::vector<int> position, SDL_Renderer& _renderer, TTF_Font& _font, SDL_Color color = { 255, 255, 255 })
		: texture(GTexture(&_renderer, &_font))
	{
		//Constructor for the text class.
		text = str;

		if (!texture.loadFromRenderedText(str, color))
		{
			printf("Failed to render text texture!\n");
		}
		pos_x = position[0];
		pos_y = position[1];
	}

	int Set_Text(std::string str, SDL_Color color = {255,255,255})
	{
		text = str;
		if (!texture.loadFromRenderedText(str, color))
		{
			printf("Failed to render text texture!\n");
			return -1;
		}
		return 0;
	}

	void Set_Position(vector3 pos)
	{
		//std::cout << "Setting label pos: " << pos.Debug() << "\n";
		pos_x = pos.x;
		pos_y = pos.y;
		visible = pos.z >= 0;
	}

	GTexture& Get_Texture()
	{
		return texture;
	}

	int Render(const vector3 screen_dimensions)
	{
		int s_x = screen_dimensions.x;
		int s_y = screen_dimensions.y;
		
		if (pos_x < s_x && pos_y < s_y && visible)
		{
			//t->Debug();
			//x + SCREEN_WIDTH / 2, -y + SCREEN_HEIGHT / 2
			//std::cout << "Trying to render @: " << pos_x << "," << pos_y<<"\n";
			texture.render(pos_x + (s_x - texture.getWidth()) / 2, -pos_y + (s_y - texture.getHeight()) / 2);
		}
		return 0;
	}

	void Debug()
	{
		std::cout << text<<" | "<<pos_x << "\n";
	}

	~Text()
	{
		free(); //THIS IS GETTING CALLED... PROBABLY BECAUSE YOU ARE AN IDIOT
	}

	void free()
	{
		texture.free();
	}
};
/*
	Handles all graphics for the application. This includes all pixel writes to the screen; loading and writing to textures; rendering
*/
class Graphyte
{
	private: //Private attributes & Methods

	const float km_per_pixel = 750; //The number of kilometres per pixel on screen.
	const double SCREEN_WIDTH = 800; //What it says on the tin.
	const double SCREEN_HEIGHT = 800;

	SDL_Renderer* Renderer = NULL; //Renderer.
	TTF_Font* Font = NULL; //True Type Font. Needs to be loaded at init.

	std::vector<Text*> texts; //Vector of text elements to be drawn to the screen.
	std::vector<SDL_Point> points; //Vector of points to be drawn to the screen. Iterate through & draw each point to screen as a pixel.

	public:  //Public attributes & Methods

	bool Init(SDL_Renderer& _renderer, TTF_Font& _font)
	{
		Renderer = &_renderer;
		Font = &_font;
		if (Renderer == NULL || Font == NULL)
		{
			return false;
		}

		return true;
	}

	//This method instantiates a new Text object and returns it. The new text object will be added to the array of text objects: texts.
	Text* CreateText(std::string str, int font_size, SDL_Color color = { 255, 255, 255 })
	{
		Text* newText = new Text(str, font_size, { 0, 0 }, *Renderer, *Font, color);
		std::cout << "Created new text: " << newText->text << "\n";
		texts.push_back(newText);
		return newText;
	}

	vector3 Get_Screen_Dimensions()
	{
		return { (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT, 0 };
	}

	void pixel(float x, float y)
	{
		if (x < SCREEN_WIDTH && y < SCREEN_HEIGHT)
		{
			SDL_Point _point = { x + SCREEN_WIDTH / 2, -y + SCREEN_HEIGHT / 2 };

			points.emplace_back(_point);
		}
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
		for (Text* t : texts)
		{
			t->Render({ SCREEN_WIDTH, SCREEN_HEIGHT, 0 });
		}

		SDL_RenderPresent(Renderer);
		points.clear();
	}

	void free()
	{
		for (auto& t : texts)
		{
			t->free();
		}
		texts.clear();
		points.clear();
	}
};

//GUI Helpers
struct GUI_Block //"Blocks" are collections of text elements to help with positioning them on screen. It is objectively awesome that its possible for me to do this off the framework I've created.
{
	std::vector<Text*> elements;
};
#endif /*ORBYTE_GRAPHICS_H*/