#pragma once

#ifndef ORBITBODY_H
#define ORBITBODY_H

#include <iostream>
#include <string>
#include <SDL.h>
#include <stdio.h> //This library makes debugging nicer, but shouldn't really be involved in user usage.
#include <vector>
#include <numeric>
#include <sstream>
#include "vec3.h"

class body
{
	std::vector<vector3> vertices;
	std::vector<edge> edges;
	public: float x, y, z;

	//Orbit information
	vector3 velocity{ 0,0,0 };
	float mu = 0;
	float mass = 1;
	float god_mass = 10;
	vector3 god_pos;



	public: body(float center_x, float center_y, float center_z, float scale, vector3 _velocity, vector3 _god_pos)
	{
		mu = 6.6743 * pow(10, -11) * god_mass;
		velocity = _velocity;
		god_pos = _god_pos;

		x = center_x;
		y = center_y;
		z = center_z;
		
		vertices = Generate_Vertices(scale);

		//Edges Now
		std::vector<edge> _edges{
			{0, 3},
			{0, 2},
			{0, 4},
			{0,5},

			{1, 2},
			{1,3},
			{1,4},
			{1,5},

			{2,4},
			{2,5},
			{3,4},
			{3,5}
		};
		edges = _edges;
	}

	std::vector<vector3> two_body_ode(float t, vector3 _r, vector3 _v)
	{
		vector3 r = _r;
		vector3 nr = Normalize(r);
		vector3 a = {
			(-mu * r.x) / (pow(nr.x, 3)),
			(-mu * r.x) / (pow(nr.x, 3)),
			(-mu * r.x) / (pow(nr.x, 3))
		};
		return { _v, a };
	}

	//TODO: rk4_step function

	std::vector<vector3> Generate_Vertices(float scale)
	{
		std::vector<vector3> _vertices{
			{1, 0, 0},
			{-1, 0, 0},

			{0, 1, 0}, //2
			{0, -1, 0},

			{0, 0, 1}, //4
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
		return _vertices;
	}

	public: int Update_Body(Uint32 delta)
	{
		rotate(0.001f, 0.002f, 0.003f);
		
		Move(velocity.x, velocity.y, velocity.z, delta);
		return 0;
	}

	void Move(float mov_x, float mov_y, float mov_z, float delta)
	{
		float dx = mov_x * delta;
		float dy = mov_y * delta;
		float dz = mov_z * delta;

		x += dx;
		y += dy;
		z += dz;

		for (auto& p : vertices)
		{
			p.x += dx;
			p.y += dy;
			p.z += dz;
		}
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

	void rotate(float rot_x = 1, float rot_y = 1, float rot_z = 1)
	{
		for (auto& p : vertices)
		{
			vector3 point = p;
			//centroid adjustments
			point.x -= x;
			point.y -= y;
			point.z -= z;

			//Rotate point
			float rad = 0;

			rad = rot_x;
			point.y = std::cos(rad) * point.y - std::sin(rad) * point.z;
			point.z = std::sin(rad) * point.y + std::cos(rad) * point.z;

			rad = rot_y;
			point.x = std::cos(rad) * point.x - std::sin(rad) * point.z;
			point.z = std::sin(rad) * point.x + std::cos(rad) * point.z;

			rad = rot_z;
			point.x = std::cos(rad) * point.x - std::sin(rad) * point.y;
			point.y = std::sin(rad) * point.x + std::cos(rad) * point.y;

			//centroid adjustments
			point.x += x;
			point.y += y;
			point.z += z;

			p = point;
		}
	}
};

#endif /*ORBITBODY_H*/
