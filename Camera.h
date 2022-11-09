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
	Camera(vector3 _position, float _near_clipping_plane)
	{
		position = _position;
		std::cout << "Instantiated Camera With Position: " << position.Debug() << "\n";
	}

	vector3 WorldSpaceToScreenSpace(vector3 world_pos, float screen_height, float screen_width)
	{
		 
	}
};

#endif /*CAMERA_H*/