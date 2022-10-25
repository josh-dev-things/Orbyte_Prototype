#include <vector>
#include <SDL.h>
#include "vec3.h"

class Ico
{
	std::vector<vector3> vertices;
	std::vector<edge> edges;
	public : float x, y, z;
	float radius;
	const float GR = 1.618;

	public: Ico(float _radius, float _x, float _y, float _z)
	{
		radius = _radius;
		x = _x;
		y = _y;
		z = _z;

		//Now Get The Points
		std::vector<vector3> _vertices{
			{0, 1, GR},
			{0, -1, GR},
			{0, 1, -GR},
			{0, -1, -GR},

			{1, GR, 0},
			{-1, GR, 0},
			{1, -GR, 0},
			{-1, -GR, 0},

			{GR, 0, 1},
			{-GR, 0, 1},
			{GR, 0, -1},
			{-GR, 0, -1}
		};
		vertices = _vertices;

		for (auto& v : vertices)
		{
			v.x *= 50;
			v.x += x;
			v.y *= 50;
			v.y += y;
			v.z *= 50;
			v.z += z;
		}

		std::vector<edge> _edges{
			{0, 1}
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