// Orbyte_Prototype.cpp : This file contains the 'main' function. Program execution begins and ends there.
//  SDL + GUI library built for SDL 
// https://lazyfoo.net/tutorials/SDL/01_hello_SDL/index2.php
// https://lazyfoo.net/tutorials/SDL/index.php


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
#include "utils.h"

class Simulation
{
private:
	const double SCREEN_WIDTH = 1200;
	const double SCREEN_HEIGHT = 800;
	const int SCREEN_FPS = 500;
	const int SCREEN_TICKS_PER_FRAME = 1000 / SCREEN_FPS;
	const int MAX_FPS = 500;
	double time_scale = 1;

	//Globally used font
	TTF_Font* gFont = NULL;

	//Window
	SDL_Window* gWindow = NULL;

	//Camera
	Camera gCamera;

	//The window renderer
	SDL_Renderer* gRenderer = NULL;

	//Graphyte
	Graphyte graphyte;

	//Data Controller
	DataController data_controller;

	//Orbit Bodies
	std::vector<Body*> orbiting_bodies;

	//CB
	CentralBody Sun;

	//Runtime variables
	bool quit = false;
	SDL_Event sdl_event;

	//Current time start time
	Uint32 startTime = 0;
	Uint32 deltaTime = 0; // delta time in milliseconds

	//Path source for Orbyte Files
	std::string path_source;

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
		if (!graphyte.Init(*gRenderer, *gFont, { SCREEN_WIDTH, SCREEN_HEIGHT, 0 }))
		{
			printf("Graphyte could not initialize!");
			return false;
		}
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
		if (graphyte.active_text_field != NULL)
		{
			graphyte.active_text_field->Commit();
		}
	}

	void close_planet_inspectors() //This is a janky implementation, but its all I have time for.
	{
		for (Body* b : orbiting_bodies)
		{
			b->HideBodyInspector();
		}
	}

	void click(int mX, int mY)
	{
		std::cout << "\nChecking for clickable @: " << mX << ", " << mY << "\n";
		if (graphyte.active_text_field != NULL)
		{
			graphyte.active_text_field->Disable();
		}
		graphyte.active_text_field = NULL;

		for (TextField* tf : graphyte.text_fields) // TODO: I dont want to iterate through text fields, I want to iterate through all buttons. Figure out how to do this.
		{
			if (tf->CheckForClick(mX, mY))
			{
				graphyte.active_text_field = tf;
				printf("\n YOU CLICKED A THING \n");
				return;
			}
		}

		for (FunctionButton* fb : graphyte.function_buttons)
		{
			if (fb->CheckForClick(mX, mY))
			{
				return;
			}
		}

		close_planet_inspectors();
	}

	Uint32 Update_Clock()
	{
		Uint32 current_time = SDL_GetTicks(); //milliseconds
		Uint32 delta = current_time - startTime;
		startTime = current_time;
		return delta;
	}

	void clean_orbit_queue()
	{
		int length = orbiting_bodies.size();
		for (int i = 0; i < length; i += 0) //This is an odd loop
		{
			if (orbiting_bodies[i]->to_delete)
			{
				orbiting_bodies.erase(orbiting_bodies.begin() + i);
				length -= 1;
			}
			else {
				i++;
			}
		}
	}

	vector3 calculate_centre_of_mass(CentralBody cb)
	{
		//A Level Further Maths: Mechanics
		double total_mass = cb.mass;
		vector3 com = cb.position;

		for (auto& b : orbiting_bodies)
		{
			double mass = b->Get_Mass();

			com = com + (b->Get_Position() * mass);
			total_mass += mass;
		}

		com = com * ((double)1 / total_mass);
		return com;
	}

	void toggle_pause()
	{
		if (time_scale == 0)
		{
			time_scale = 1;
		}
		else {
			time_scale = 0;
		}
		return;
	}

	void add_specific_orbit(OrbitBodyData data)
	{
		orbiting_bodies.push_back(new Body(data.name, data.center, data.mass, data.scale, data.velocity, Sun.mu, graphyte, false));
	}

	void add_orbit_body()
	{
		orbiting_bodies.push_back(new Body("New", { 0, 5.8E10, 0 }, 3.285E23, 2.44E6, { 47000, 0, 0 }, Sun.mu, graphyte, false));
	}

	void save()
	{
		OrbitBodyCollection obc;
		std::vector<std::string> to_save;

		for (Body* b : orbiting_bodies)
		{
			if (!b->to_delete)
			{
				to_save.push_back(b->name);
				obc.AddBodyData(b->GetOrbitBodyData());
			}
		}

		SimulationData sd = { Sun.mass, Sun.scale, obc, gCamera.position };
		data_controller.WriteDataToFile(sd, to_save, path_source);
	}

	void open()
	{
		std::cout << "\nOpening File";
		SimulationData new_sd = data_controller.ReadDataFromFile(path_source);
		time_scale = 0;

		Sun.mass = new_sd.cb_mass;
		std::cout << "\n Sun mass: " << Sun.mass;
		Sun.scale = new_sd.cb_scale;
		gCamera.position = new_sd.c_pos;
		for (Body* old_orbit : orbiting_bodies)
		{
			old_orbit->Delete();
		}
		std::cout << "\n obc: " << new_sd.obc.GetAllOrbits().size();
		OrbitBodyCollection obc = new_sd.obc;
		std::vector<OrbitBodyData> orbits = obc.GetAllOrbits();
		for (OrbitBodyData orbit : orbits)
		{
			add_specific_orbit(orbit);
		}
	}

	void regenerate_center_body_vertices()
	{
		Sun.RegenerateVertices();
	}

	void recalculate_center_body_mu()
	{
		Sun.RecalculateMu();
		for (Body* orbit : orbiting_bodies)
		{
			orbit->Set_Mu(Sun.mu);
		}
	}
public:

	int run(int argc, char* args[])
	{
		//Start up SDL and create window
		if (!init())
		{
			printf("Failed to initialize!\n");
		}
		else
		{
			// Name, Pos, mass, Radius, Velocity
			//Body mercury = Body("Mercury", { 0, 5.8E10, 0 }, 3.285E23, 2.44E6, { 47000, 0, 0 }, Sun, graphyte, false);
			//Body venus = Body("Venus", { 0, 1E11, 0 }, 6E6, { 35000, 0, 0 }, Sun, graphyte, false);
			Body earth = Body("Earth", {0, 1.49E11, 0}, 5.97E24, 6.37E6, { 30000, 0, 0 }, Sun.mu, graphyte, false); //152000000000 metres. That number is too large so we have a problem
			//Body mars = Body("Mars", { 0, 2.4E11, 0 }, 3.4E6, { 24000, 0, 0 }, Sun, graphyte, false);
			//Body jupiter = Body("Jupiter", { 0, 7.4E11, 0 }, 7E7, { 13000, 0, 0 }, Sun, graphyte, false);
			//Body saturn = Body("Saturn", { 0, 1.4E12, 0 }, 5.8E7, { 9680, 0, 0 }, Sun, graphyte, false);
			//Body uranus = Body("Uranus", { 0, 2.8E12, 0 }, 2.5E7, { 6800, 0, 0 }, Sun, graphyte, false);
			//Body neptune = Body("Neptune", { 0, 4.47E12, 0 }, 2.5E7, { 5430, 0, 0 }, Sun, graphyte, false);

			orbiting_bodies.emplace_back(&earth);
			//orbiting_bodies.emplace_back(&mercury);
			/*orbiting_bodies.emplace_back(venus);  
			orbiting_bodies.emplace_back(earth);
			orbiting_bodies.emplace_back(mars);
			orbiting_bodies.emplace_back(jupiter);
			orbiting_bodies.emplace_back(saturn);
			orbiting_bodies.emplace_back(uranus);
			orbiting_bodies.emplace_back(neptune); */


			/*
				SIMULATION PARAMETERS GUI INITIALIZATION
			*/
			GUI_Block Simulation_Parameters(vector3{ -SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 0 });

			Text* text_pm = graphyte.CreateText("__________________\nPERFORMANCE METRICS\n__________________", 24);
			Simulation_Parameters.Add_Stacked_Element(text_pm);

			Text* text_FPS_Display = graphyte.CreateText("FPS", 10);
			Simulation_Parameters.Add_Stacked_Element(text_FPS_Display);


			Text* text_Vertex_Count_Display = graphyte.CreateText("Vertices", 10);
			Simulation_Parameters.Add_Stacked_Element(text_Vertex_Count_Display);

			Text* text_sp = graphyte.CreateText("__________________\nSIMULATION PARAMETERS\n__________________", 24);
			Simulation_Parameters.Add_Stacked_Element(text_sp);

			//Testing input fields I guess
			Simulation_Parameters.Add_Stacked_Element(graphyte.CreateText("Time Scale [0.1 | 1 | 86400]: ", 10));
			DoubleFieldValue TimeScaleFV(&time_scale);
			TextField* tf = new TextField({ 10,10,0 }, TimeScaleFV, graphyte, std::to_string(time_scale));
			graphyte.text_fields.push_back(tf);
			Simulation_Parameters.Add_Inline_Element(tf);
			
			Simulation_Parameters.Add_Stacked_Element(graphyte.CreateText("Center Body Mass: ", 10));
			DoubleFieldValue CentreMassFV(&Sun.mass, [this]() { this->recalculate_center_body_mu(); });
			tf = new TextField({ 0, 0, 0 }, CentreMassFV, graphyte, std::to_string(Sun.mass));
			graphyte.text_fields.push_back(tf);
			Simulation_Parameters.Add_Inline_Element(tf);

			Simulation_Parameters.Add_Stacked_Element(graphyte.CreateText("Center Body Scale: ", 10));
			DoubleFieldValue CentreScaleFV(&Sun.scale, [this]() { this->regenerate_center_body_vertices(); });
			tf = new TextField({ 0, 0, 0 }, CentreScaleFV, graphyte, std::to_string(Sun.scale));
			graphyte.text_fields.push_back(tf);
			Simulation_Parameters.Add_Inline_Element(tf);


			/*
				PATH TO OPEN FROM FILE
			*/
			GUI_Block path_gui;
			path_gui.position = { (-SCREEN_WIDTH / 2), (-SCREEN_HEIGHT / 2) + 36, 0 };
			Text* path_input_prompt = graphyte.CreateText("[SAVE/READ] Enter path here: ", 12);
			path_gui.Add_Stacked_Element(path_input_prompt);
			StringFieldValue path_to_file(&path_source, NULL, "(([A-Z]|[a-z]|(_))|[ ])+\.orbyte"); //Custom regex
			TextField* path_input = new TextField({ ( - SCREEN_WIDTH / 2), (-SCREEN_HEIGHT / 2), 0}, path_to_file, graphyte, "solar_system.orbyte");
			path_source = "solar_system.orbyte"; //default
			graphyte.text_fields.push_back(path_input);
			path_gui.Add_Inline_Element(path_input);

			/*
				INSPECTOR PARAMETERS GUI INITIALIZATION
			*/

			//Create Buttons
			FunctionButton Pause([this]() { this->toggle_pause(); }, { (SCREEN_WIDTH / 2) - 25, (SCREEN_HEIGHT / 2) - 25, 0 }, { 25, 25, 0 }, graphyte, "icons/stop.png");
			graphyte.function_buttons.push_back(&Pause);

			FunctionButton Add([this]() { this->add_orbit_body(); }, { (SCREEN_WIDTH / 2) - 25, (SCREEN_HEIGHT / 2) - 60, 0 }, { 25, 25, 0 }, graphyte, "icons/add.png");
			graphyte.function_buttons.push_back(&Add);

			FunctionButton Save([this]() { this->save(); }, { (SCREEN_WIDTH / 2) - 25, (SCREEN_HEIGHT / 2) - 95, 0 }, { 25, 25, 0 }, graphyte, "icons/save.png");
			graphyte.function_buttons.push_back(&Save);

			FunctionButton Open([this]() { this->open(); }, { (SCREEN_WIDTH / 2) - 25, (SCREEN_HEIGHT / 2) - 130, 0 }, { 25, 25, 0 }, graphyte, "icons/open.png");
			graphyte.function_buttons.push_back(&Open);

			//Mainloop time 
			while (!quit)
			{
				//GRAPHICS 
				//gCamera.position = { earth.Get_Position().x, earth.Get_Position().y, gCamera.position.z };
				//render sun
				Sun.Draw(graphyte, gCamera);
				clean_orbit_queue();
				//vector3 com = calculate_centre_of_mass(Sun);
				//std::cout << "\n" << com.Debug();
				/*vector3 debug_com = gCamera.WorldSpaceToScreenSpace(com, SCREEN_HEIGHT, SCREEN_WIDTH);
				graphyte.pixel(debug_com.x, debug_com.y);*/
				for (auto& b : orbiting_bodies)
				{
					b->Update_Body(deltaTime, time_scale); // Update body
					//std::cout<<b.Get_Position().Debug()<<"\n"; 
					//b.Calculate_Period();
					b->Draw(graphyte, gCamera);
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
						if (graphyte.active_text_field != NULL)
						{
							graphyte.active_text_field->Add_Character(sdl_event.text.text);
						}
						break;

					case SDL_KEYDOWN:
						switch (sdl_event.key.keysym.sym)
						{
						case SDLK_BACKSPACE:
							printf("User Pressed the Backspace\n");
							if (graphyte.active_text_field != NULL)
							{
								graphyte.active_text_field->Backspace();
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

						case SDLK_RETURN:
							commit_to_text_field();
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
						deltaTime += delay;
					}
				}

				/*DEBUG*/
				float debug_fps = (float)1000 / ((float)deltaTime + 1);
				text_FPS_Display->Set_Text("FPS: " + std::to_string(debug_fps));
				text_Vertex_Count_Display->Set_Text("Vertex Count: " + std::to_string(debug_no_pixels));
			}

		}
		//Free resources and close SDL
		close();

		return 0;
	}
};

int main(int argc, char* args[])
{
	std::cout << "\n___________________________________\nSTARTING ORBYTE\n___________________________________\n";
	Simulation Sim;
	Sim.run(argc, args);
	return 0;
}