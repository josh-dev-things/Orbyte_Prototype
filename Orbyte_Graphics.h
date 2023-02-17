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
#include <functional>
#include <regex> //Regular Expressions!

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

		if (textureText == "")
		{
			textureText = " ";
		}
		//Render text surface
		SDL_Surface* textSurface = TTF_RenderUTF8_Solid_Wrapped(font, textureText.c_str(), textColor, 320);
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
	void render(int x, int y, int override_width = NULL, int override_height = NULL, SDL_Rect* clip = NULL, double angle = 0.0, SDL_Point* center = NULL, SDL_RendererFlip flip = SDL_FLIP_NONE)
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

			if (override_width != NULL)
			{
				renderQuad.w = override_width;
				//std::cout << "Overrided width of an icon: " << override_width;
			}

			if (override_height != NULL)
			{
				renderQuad.h = override_height;
			}
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

/*
	Encapsulates all key text functionality, such as creating text, moving and setting text.
*/
class Text
{
protected:
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

	Text(Text& T) //Copy constructor to save the day???
	{
		texture = T.texture; //How am I reading from a private attribute lol
		text = T.text;
		if (!texture.loadFromRenderedText(text, {255, 255, 255}))
		{
			printf("Failed to render text texture!\n");
		}
		pos_x = T.pos_x;
		pos_y = T.pos_y;

	}

	int Set_Text(std::string str, SDL_Color color = {255,255,255})
	{
		if (str == text)
		{
			//std::cout << "Redundant text assignment";
			return 0;
		}

		if (str == "") //Just a tiny bit of redundancy to be safe.
		{
			str = " ";
		}

		text = str;

		if (!texture.loadFromRenderedText(str, color))
		{
			printf("Failed to render text texture!\n");
			return -1;
		}
		return 0;
	}

	virtual void Set_Position(vector3 pos)
	{
		//Sets the position of the text with centering
		pos_x = pos.x - texture.getWidth() / 2;
		pos_y = pos.y + texture.getHeight() / 2;
		visible = pos.z >= 0;
	}

	virtual void Set_Position_TL(vector3 pos)
	{
		//In some cases it is more helpful to set the position of the text with the top left anchor point, what SDL uses as the pivot for textures.
		pos_x = pos.x;
		pos_y = pos.y;
		visible = pos.z >= 0;
	}

	void Set_Visibility(bool is_visible)
	{
		visible = is_visible;
	}

	GTexture& Get_Texture()
	{
		return texture;
	}

	vector3 Get_Position()
	{
		return vector3{ (double)pos_x, (double)pos_y, 0 };
	}

	vector3 Get_Dimensions()
	{
		return vector3{ (double)texture.getWidth(), (double)texture.getHeight(), 0 };
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
			texture.render(pos_x + (s_x) / 2, -pos_y + (s_y) / 2);
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
	Icon class. This includes an image that can be used for buttons.
*/
class Icon
{
private:
	GTexture texture = NULL;
public:
	int pos_x; //Position along x axis in screenspace
	int pos_y; //Position along y axis in screenspace
	std::string path_to_image;
	bool visible = true;
	std::vector<int> dimensions;

	Icon(std::string path, std::vector<int> position, std::vector<int> _dimensions, SDL_Renderer& _renderer)
		: texture(GTexture(&_renderer, NULL))
	{
		//Constructor for the text class.
		path_to_image = path;

		if (!texture.loadFromFile(path))
		{
			printf("Failed to render icon texture!\n");
			free();
		}
		pos_x = position[0];
		pos_y = position[1];
		dimensions = _dimensions;
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
			texture.render(pos_x + (s_x) / 2, -pos_y + (s_y) / 2, dimensions[0], dimensions[1]);
			//std::cout << "\nTrying to draw icon @ " << pos_x << " " << pos_y;
		}
		return 0;
	}

	void SetPosition(vector3 new_position)
	{
		vector3 my_dimensions = GetDimensions();
		pos_x = new_position.x - (my_dimensions.x / 2);
		pos_y = new_position.y + (my_dimensions.y / 2);
		//This works.
	}

	void SetDimensions(std::vector<int> new_dimensions)
	{
		dimensions = new_dimensions;
	}

	vector3 GetDimensions()
	{
		return { (double)dimensions[0], (double)dimensions[1], 0 };
	}

	void free()
	{
		texture.free();

	}
};

class TextField; //A Forward Declaration so nothing collapses

class FunctionButton; //A Forward Declaration so nothing collapses

/*
	Handles all graphics for the application. This includes all pixel writes to the screen; loading and writing to textures; rendering
*/
class Graphyte
{
private: //Private attributes & Methods

	const float km_per_pixel = 750; //The number of kilometres per pixel on screen.
	double SCREEN_WIDTH = 0; //What it says on the tin.
	double SCREEN_HEIGHT = 0; //These dimensions are not const values because they need to be set in the Init() method.

	SDL_Renderer* Renderer = NULL; //Renderer.
	TTF_Font* Font = NULL; //True Type Font. Needs to be loaded at init.

	std::vector<Text*> texts; //Vector of text elements to be drawn to the screen.
	std::vector<Icon*> icons; //Vectorr of icon elements to be drawn to the screen.
	std::vector<SDL_Point> points; //Vector of points to be drawn to the screen. Iterate through & draw each point to screen as a pixel.

public:  //Public attributes & Methods
	TextField* active_text_field = NULL; //This pointer will be used to edit text fields
	std::vector<TextField*> text_fields;
	std::vector<FunctionButton*> function_buttons; //It is possible to handle the input methods in a tidier way, but alas this is all I have time for.

	bool Init(SDL_Renderer& _renderer, TTF_Font& _font, vector3 _screen_dimensions)
	{
		Renderer = &_renderer;
		Font = &_font;

		SCREEN_WIDTH = _screen_dimensions.x;
		SCREEN_HEIGHT = _screen_dimensions.y;

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

	Icon* CreateIcon(std::string path, std::vector<int> dimensions)
	{
		Icon* newIcon = new Icon(path, {0, 0}, dimensions, *Renderer);
		std::cout << "\nCreated new Icon: " << path << "\n";
		AddIconToRenderQueue(newIcon);
		return newIcon;
	}

	Text* GetTextParams(std::string str, int font_size, SDL_Color color = { 255, 255, 255 }) // There is a nuance between these two methods. See textfield
	{
		Text* newText = new Text(str, font_size, { 0, 0 }, *Renderer, *Font, color);
		std::cout << "Created new text: " << newText->text << "\n";
		return newText;
	}

	void AddTextToRenderQueue(Text* newText)
	{
		texts.push_back(newText);
	}

	void AddIconToRenderQueue(Icon* icon)
	{
		icons.push_back(icon);
	}

	vector3 Get_Screen_Dimensions()
	{
		return { (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT, 0 };
	}

	double Get_Number_Of_Points()
	{
		return points.size();
	}

	void pixel(int x, int y)
	{
		if (std::abs(x) < (float)(SCREEN_WIDTH / 2) && std::abs(y) < (float)(SCREEN_HEIGHT / 2)) //abs value for negative values... that took so long to find.
		{
			SDL_Point _point = { x + SCREEN_WIDTH / 2, -y + SCREEN_HEIGHT / 2 };
			points.emplace_back(_point);
		}
	}

	void line(float x1, float y1, float x2, float y2)
	{
		int dx = (x2 - x1);
		int dy = (y2 - y1);
		int length = std::sqrt(dx * dx + dy * dy);
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

		//pixel(100, 100);

		for (auto& point : points)
		{
			SDL_SetRenderDrawColor(Renderer, 255, ((float)point.x / (float)SCREEN_WIDTH) * 255, ((float)point.y / (float)SCREEN_HEIGHT) * 255, 255);
			SDL_RenderDrawPoint(Renderer, point.x, point.y);
			count++;
		}

		//Make sure you render GUI!
		for (Text* t : texts)
		{
			t->Render({ SCREEN_WIDTH, SCREEN_HEIGHT, 0 });
		}

		for (Icon* i : icons)
		{
			i->Render({ SCREEN_WIDTH, SCREEN_HEIGHT, 0 });
			
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
		SDL_StopTextInput();
	}
};

class Arrow
{
public:
	void Draw(vector3 position, vector3 direction, double magnitude, int heads, Graphyte& graphyte)
	{
		//These are all 2D vectors.
		vector3 start = position;
		vector3 end = position + (direction * magnitude);
		vector3 perp_dir = vector3{ direction.y, -direction.x, 0 };
		double arrow_head_size = magnitude / 10;

		for (int i = 0; i < heads; i++)
		{
			vector3 ah1 = end + (direction * arrow_head_size);
			vector3 ah2 = end + (perp_dir * arrow_head_size);
			vector3 ah3 = end + (perp_dir * -arrow_head_size);

			graphyte.line(start.x, start.y, end.x, end.y);
			graphyte.line(ah1.x, ah1.y, ah2.x, ah2.y);
			/*graphyte.line(ah2.x, ah2.y, ah3.x, ah3.y);*/
			graphyte.line(ah3.x, ah3.y, ah1.x, ah1.y);

			end = ah1;
		}
	}
};

class Button
{
private:
	vector3 position;
	int width;
	int height;
	int left_wall_offset;
	std::function<void()> function = NULL;

protected:
	void CallFunction()
	{
		if (function != NULL)
		{
			function();
		}
	}

	void AttachFunction(std::function<void()> f)
	{
		function = f;
	}
	
public:
	Button(vector3 pos, vector3 dimensions)
	{
		position = pos;
		width = dimensions.x;
		height = dimensions.y;
		left_wall_offset = width / 2;
	}

	void SetDimensions(vector3 dimensions)
	{
		width = dimensions.x;
		height = dimensions.y;
	}

	void SetPosition(vector3 pos)
	{
		position = pos;
	}

	bool Clicked(int x, int y)
	{
		// (0<AM.AB<AB.AB) ^ (0<AM.AD<AD.AD) Where M is a point we're checking
		vector3 A = { position.x - left_wall_offset, position.y + height / 2, 0 };
		vector3 B = { position.x - left_wall_offset + width, position.y + height / 2, 0 };
		vector3 D = { position.x - left_wall_offset, position.y - height / 2, 0 };
		vector3 C = { position.x - left_wall_offset + width, position.y - height / 2, 0 }; //All the vertices
		std::cout << "\n Dimensions" << width << "|"<< height;
		vector3 M = { x, y, 0 };

		vector3 AM = M - A;
		vector3 AB = B - A;
		vector3 BC = C - B;
		vector3 BM = M - A;

		bool in_area = 0 <= AB*AM && AB*AM <= AB*AB && 0 <= BC*BM && BC*BM <= BC*BC;
		std::cout << "\n RESULT: " << in_area;
		return in_area;
	}
};

class FunctionButton : public Button
{
private:
	Icon* icon = NULL;
public:
	/// <summary>
	/// Constructor
	/// </summary>
	FunctionButton(std::function<void()> f, vector3 pos, vector3 dimensions, Graphyte& g, std::string path_to_icon) : Button(pos, dimensions)
	{
		std::cout << "\n Instantiated a function button. Method present?: " << (bool)f << "\n";
		if (path_to_icon != "") //Never accessed by user therefor no need for complex regular expressions or validation.
		{
			icon = g.CreateIcon(path_to_icon, {(int)dimensions.x, (int)dimensions.y, 0});
			icon->SetPosition(pos);
		}
		AttachFunction(f);
	}

	bool CheckForClick(int x, int y)
	{
		if (Clicked(x, y))
		{
			//Button has been clicked => 
			CallFunction();
			return true;
		}
		return false;
	}
};

class FieldValue
{
public:
	virtual void ReadField(std::string content) = 0;
};

class DoubleFieldValue : public FieldValue
{
private:
	double* value = NULL; // This is the pointer to the variable the input field is associated with. E.g. time step or object mass
	std::function<void()> read_f = NULL;
	bool ValidateValue(std::string content)
	{
		try
		{
			std::regex dbl_regex("(-+)?([0-9]*\.[0-9]+|[0-9]+)");
			if (std::regex_match(content, dbl_regex))
			{
				double test_validity = atof(content.c_str());
				std::cout << content << "=>" << test_validity;
				if (test_validity == NULL) // Will not allow zero!
				{
					throw(content); // Knew I was tired when I found the "throw" and "catch" system the funniest thing ever invented
				}
				else {
					return true;
				}
			}
			else {
				throw(content);
			}
		}
		catch (std::string bad)
		{
			std::cout << "\n Bad input recieved: " << bad; // Its bad
			return false;
		}
	}
public:
	DoubleFieldValue(double* write_to, std::function<void()> f = NULL)
	{
		value = write_to;
		if (f)
		{
			read_f = f;
		}
	}

	void ReadField(std::string content) override
	{
		if (ValidateValue(content))
		{
			std::cout << "\n Valid field content";
			if (value != NULL)
			{
				double new_value = atof(content.c_str());
				*value = new_value;
				std::cout << "\n Successfully wrote to value from input field!  \n"<< new_value;

				if (read_f != NULL)
				{
					read_f();
				}
			}
		}
	}
};

class StringFieldValue : public FieldValue
{
private:
	std::string* value = NULL; // This is the pointer to the variable the input field is associated with. E.g. object name
	std::function<void()> read_f = NULL;
	bool ValidateValue(std::string content)
	{
		try
		{
			std::regex dbl_regex("(([A-Z]|[a-z])|[ ])+");
			if (std::regex_match(content, dbl_regex))
			{
				return true;
			}
			else {
				throw(content);
			}
		}
		catch (std::string bad)
		{
			std::cout << "\n Bad input recieved: " << bad; // Its bad
			return false;
		}
	}
public:
	StringFieldValue(std::string* write_to, std::function<void()> f = NULL)
	{
		value = write_to;
		if (f)
		{
			read_f = f;
		}
	}

	void ReadField(std::string content) override
	{
		if (ValidateValue(content))
		{
			std::cout << "\n Valid field content";
			if (value != NULL)
			{
				*value = content;
				std::cout << "\n Successfully wrote to value from input field!  \n" << content;

				if (read_f != NULL)
				{
					read_f();
				}
			}
		}
	}
};

class TextField : public Text
{
private:
	SDL_Color text_color = { 255, 255, 255, 0xFF };
	std::string input_text = " ";
	bool enabled = false;
	Button* button = NULL;
	FieldValue &fvalue;

	void Update_Text()
	{
		if (input_text != "")
		{
			Set_Text(input_text, text_color);
		}
		else {
			Set_Text(input_text, text_color);
		}
	}

	void update_button_dimensions()
	{
		vector3 dimensions = { Get_Texture().getWidth(), Get_Texture().getHeight(), 0 };
		button->SetDimensions(dimensions);
	}

	void write_value()
	{
		fvalue.ReadField(text); //ACCESS VIOLATION!
	}
public:
	TextField(vector3 position, FieldValue& writeto, Graphyte& g, std::string default_text = "This Is An Input Field") : 
		 Text(*g.GetTextParams(default_text, 16, text_color)), fvalue(writeto)
	{
		vector3 dimensions = { Get_Texture().getWidth(), Get_Texture().getHeight(), 0 };
		input_text = default_text;
		button = new Button(position, dimensions);
		Set_Position({ position.x, position.y, 10 });
		g.AddTextToRenderQueue(this); //Beautiful
	}

	void Set_Position(vector3 position) override //THIS IS AN OVERRIDE SO THAT BUTTON POS CAN BE SET AS WELL
	{
		//Sets the position of the text with centering
		pos_x = position.x - texture.getWidth() / 2;
		pos_y = position.y + texture.getHeight() / 2;
		visible = position.z >= 0;

		button->SetPosition(position); //There's a bug here [Edit: I dont know when I wrote this so am not sure if its fixed tbh]
	}

	void Set_Position_TL(vector3 pos) override
	{
		//In some cases it is more helpful to set the position of the text with the top left anchor point, what SDL uses as the pivot for textures.
		pos_x = pos.x;
		pos_y = pos.y;
		visible = pos.z >= 0;
		
		button->SetPosition({ pos.x + texture.getWidth() / 2 , pos.y - texture.getHeight() / 2, 0 });

		std::cout << vector3{ pos.x + texture.getWidth() / 2, pos.y - texture.getHeight() / 2, 0 }.Debug();
	}

	void Backspace()
	{
		if (input_text.length() > 0 && enabled)
		{
			input_text.pop_back();
			Update_Text();
		}
	}

	void Add_Character(char* chr)
	{
		if (enabled)
		{
			input_text += chr;
			Update_Text();
		}
	}

	bool CheckForClick(int x, int y)
	{
		if (button->Clicked(x, y))
		{
			//Button has been clicked => 
			Enable();
			return true;
		}
		return false;
	}

	void Commit()
	{
		std::cout << "\n Committing to text field value! \n";
		write_value();
		update_button_dimensions();
		// We do this so that the button resizes after this new text commit.
	}

	void Enable()
	{
		SDL_StartTextInput();
		enabled = true;
	}

	void Disable()
	{
		SDL_StopTextInput();
		enabled = false;
		Commit();
		std::cout << "Disabled text";
	}

	~TextField()
	{
		free();
		Disable();
		//Should be safely destroyed now.
	}
};

//GUI Helpers
struct GUI_Block //"Blocks" are collections of text elements to help with positioning them on screen. It is objectively awesome that its possible for me to do this off the framework I've created.
{
	//This struct is going to facilitate pretty much all application GUI. Lol.
	std::vector<Text*> elements;
	vector3 position;
	bool is_visible = true;

	GUI_Block(vector3 _position = {0, 0, 0})
	{
		position = _position;
	}

	~GUI_Block() //Deconstructor... I love c++
	{
		elements.clear();
	}

	void Add_Floating_Element(Text* text, vector3 relative_position)
	{
		elements.push_back(text);
		text->Set_Position_TL(position + relative_position);
	}

	void Hide()
	{
		for (Text* t : elements)
		{
			t->Set_Visibility(false);
		}
		is_visible = false;
	}

	void Show()
	{
		for (Text* t : elements)
		{
			t->Set_Visibility(true);
		}
		is_visible = true;
	}

	void Add_Stacked_Element(Text* text) //This method adds the text to the bottom of the block.
	{
		vector3 new_pos = position;

		if (elements.size() > 0)
		{
			Text* above_this = elements.back();
			new_pos.y  = above_this->Get_Position().y - (above_this->Get_Texture().getHeight());
		}

		//std::cout<< position.Debug() << "=>" << new_pos.Debug() << " WIDTH IS: " << text->Get_Texture().getWidth() / 2 << "\n";

		text->Set_Position_TL(new_pos);
		elements.push_back(text);
	}

	void Add_Inline_Element(Text* text)
	{
		vector3 new_pos = position;
		if (elements.size() > 0)
		{
			Text* left = elements.back();
			new_pos.y = left->Get_Position().y;
			new_pos.x = left->Get_Position().x + left->Get_Texture().getWidth();
		}
		text->Set_Position_TL(new_pos);
		elements.push_back(text);
	}

	
};

#endif /*ORBYTE_GRAPHICS_H*/