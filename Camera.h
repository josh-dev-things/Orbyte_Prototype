#pragma once
#ifndef CAMERA_H
#define CAMERA_H
#include "vec3.h"
#include <iostream>
#include <string>
#include <SDL.h>
#include <stdio.h> //This library makes debugging nicer, but shouldn't really be involved in user usage.
#include <vector>
#include <numeric>
#include <sstream>

class Camera
{
	public:vector3 position = {0, 0, 0};
		  float clipping_z = 1;
	Camera(vector3 _position, float _near_clipping_plane)
	{
		position = _position;
		std::cout << "Instantiated Camera With Position: " << position.Debug() << "\n";
	}

	vector3 WorldSpaceToScreenSpace(vector3 world_pos, float screen_height, float screen_width)
	{
		vector3 pos = world_pos - position;
		//std::cout << "WORLD POS: " << world_pos.x << " | CAMERA POS: " << position.x << " | => " << pos.x;
		if (pos.z < clipping_z)
		{
			//DONT DRAW IT
		}
		vector3 Screen_Space_Pos = {
			(pos.x / pos.z) * screen_width,
			(pos.y / pos.z)* screen_height,
			pos.z
		};
		return Screen_Space_Pos;
	}
};

#endif /*CAMERA_H*/