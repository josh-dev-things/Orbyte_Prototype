#pragma once

#ifndef ORBITBODY_H
#define ORBITBODY_H

#include <vector>
#include <SDL.h>
#include "vec3.h"

class body
{
	std::vector<vector3> vertices;
	std::vector<edge> edges;
	public: float x, y, z;

	public: body(float center_x, float center_y, float center_z, float scale)
	{
		x = center_x;
		y = center_y;
		z = center_z;

		std::vector<vector3> _vertices{
			{1, 0, 0},
			{-1, 0, 0},

			{0, 1, 0},
			{0, -1, 0},

			{0, 0, 1},
			{0, 0, -1}
		};

		for (auto& v : _vertices)
		{
			v.x *= scale;
			v.x += x;
			v.y *= scale;
			v.y += y;
			v.z *= scale;
			v.z += z;
		}
		vertices = _vertices;

		//Edges Now
		std::vector<edge> _edges{
			{0, 3},
			{0, 2},
			{0, 4},
			{0,5},

			{1, 2},
			{1,3},
			{1,4},
			{1,5}
		};
		edges = _edges;
	}

	std::vector<vector3> Get_Vertices()
	{
		//Return vertices
		return vertices;
	}

	std::vector<edge> Get_Edges()
	{
		return edges;
	}
};

#endif /*ORBITBODY_H*/
