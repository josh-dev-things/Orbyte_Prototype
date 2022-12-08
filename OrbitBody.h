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
	vector3 last_trail_point;
	public: std::vector<vector3> trail_points;
		  std::string name;
	public: float x, y, z;
		  float scale;

private : vector3 start_pos;
private: vector3 start_vel;
	   float time_since_start = 0;

	//Orbit information
	vector3 velocity{ 0,0,0 };
	float mu = 0;
	const float god_mass = 5.9 * pow(10,24);
	vector3 god_pos;

	/// <summary>
	/// center_X, center_Y, center_Z, scale, _velocity, _god_pos, override_velocity
	/// </summary>
public: body(std::string _name, float center_x, float center_y, float center_z, float _scale, vector3 _velocity, vector3 _god_pos, bool override_velocity = true)
	{
		mu = 6.6743 * pow(10, -11) * god_mass;
		name = _name;
		if (override_velocity)
		{
			//We manipulate the velocity so that a perfectly circular orbit is achieved
			if (center_x != 0)
			{
				_velocity.y = sqrt(mu / center_x);
			}
			if (center_y != 0)
			{
				_velocity.x = sqrt(mu / center_y);
			}
			if (center_z != 0)
			{
				_velocity.x = sqrt(mu / center_z);
			}
		}
		velocity = _velocity;
		start_vel = velocity;
		god_pos = _god_pos;

		scale = _scale;

		x = center_x;
		y = center_y;
		z = center_z;

		start_pos = { x, y, z };
		std::cout << "Instantiated Orbiting Body with initial position: " << start_pos.Debug() << " and velocity: " << velocity.Debug() << "\n";
		
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
		vector3 r = _r; //r.z is wrong
		vector3 v = _v;
		//std::cout << "R.z" << r.z << "\n";
		vector3 nr = Normalize(r);
		//std::cout << "NR.z" << nr.z << "\n";
		if (nr.z == 0) { nr.z = 1; r.z = 0; }
		if (nr.y == 0) { nr.y = 1; r.y = 0; }
		if (nr.x == 0) { nr.x = 1; r.x = 0; }
		float mag = Magnitude(r);
		vector3 a = {
			(-mu * r.x) / (pow(mag, 3)),
			(-mu * r.y) / (pow(mag, 3)),
			(-mu * r.z) / (pow(mag, 3))
		};
		//std::cout << "rk_result: " << (pow(nr.z, 3)) << "\n";
		return { v, a };
	}

public: void reset()
{
	x = start_pos.x;
	y = start_pos.y;
	z = start_pos.z;

	velocity = start_vel;
}

	//TODO: rk4_step function https://www.youtube.com/watch?v=TzX6bg3Kc0E&t=241s
	std::vector<vector3> rk4_step(float _time, vector3 _position, vector3 _velocity, float _dt = 1) //SHITS THE BED WHEN Y = 0
	{
		//structure of the vectors: [pos, velocity]
		std::vector<vector3> rk1 = two_body_ode(_time, _position, _velocity);
		//std::cout << (rk1[0]).x << "\n";
		std::vector<vector3> rk2 = two_body_ode(_time + (0.5 * _dt), _position + (rk1[0] * 0.5f * _dt), _velocity + (rk1[1] * 0.5f * _dt));
		//std::cout << (_position + (rk1[0] * 0.5f * _dt)).x << "\n";
		//std::cout << "rk3 parameter for r: " << ((rk1[1])).z << "\n";
		std::vector<vector3> rk3 = two_body_ode(_time + (0.5 * _dt), _position + (rk2[0] * 0.5f * _dt), _velocity + (rk2[1] * 0.5f * _dt)); //Breaks here
		std::vector<vector3> rk4 = two_body_ode(_time + _dt, _position + (rk3[0] * _dt), _velocity + (rk3[1] * _dt)); //BUG
		//printf("RK4 complete\n");
		vector3 result_pos = _position + (rk1[0] + (rk2[0] * 2.0f) + (rk3[0] * 2.0f) + rk4[0]) * (_dt / 6.0f);
		//std::cout << (_velocity + (rk2[1] * 0.5f * _dt)).x << "\n"; //RK4 STEP IS BROKEN! BUG IS HERE! | RK3 VELOCITY -> output is bad see 2bodyODE function
		vector3 result_vel = _velocity + (rk1[1] + rk2[1] * 2 + rk3[1] * 2 + rk4[1]) * (_dt / 6);
		return {result_pos, result_vel};
		
	}

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


	public: int Update_Body(float delta, float time_scale)
	{
		time_since_start += delta * time_scale;
		rotate(0.0005f * time_scale, 0.0005f * time_scale, 0.0005f * time_scale);
		vector3 position = {x, y, z};
		float t = (delta / 1000);
		std::vector<vector3> sim_step = rk4_step(t * time_scale, position, velocity, t * time_scale / 1000);
		position = sim_step[0];
		//if (position.z > 0) { std::cout << position.Debug() << "\n"; std::cout << velocity.Debug() << "\n"; }
		MoveToPos(position);

		if (Magnitude(position - last_trail_point) > Magnitude(velocity)/ 24)
		{
			trail_points.emplace_back(position);
			last_trail_point = position;
			//printf("Added Point");
		}
		if (trail_points.size() > 24)
		{
			//printf("Erased Point");
			trail_points.erase(trail_points.begin());
		}
		
		velocity = sim_step[1];
		return 0;
	}

	void MoveToPos(vector3 new_pos)
	{
		vector3 old_pos = { x, y ,z };
		x = new_pos.x;
		y = new_pos.y;
		z = new_pos.z;

		float dx = x - old_pos.x;
		float dy = y - old_pos.y;
		float dz = z - old_pos.z;

		for (auto& p : vertices)
		{
			p.x += dx;
			p.y += dy;
			p.z += dz;
		}
	}

	std::string GetBodyData()
	{
		std::string text = name + " velocity: " + velocity.Debug() + "|| Time: " + std::to_string(time_since_start);
		return text;
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

	void rotate(float rot_x = 1, float rot_y = 1, float rot_z = 1) //Something is broken. STILL BROKEN
	{
		for (auto& p : vertices)
		{
			vector3 point = p;
			//centroid adjustments
			point.x -= x;
			point.y -= y;
			point.z -= z;

			//float start_magnitude = Magnitude(point);

			//Rotate point
			float rad = 0;

			rad = rot_x;
			point.y = std::cos(rad) * point.y - std::sin(rad) * point.z;
			point.z = std::sin(rad) * point.y + std::cos(rad) * point.z;

			rad = rot_y;
			point.x = std::cos(rad) * point.x + std::sin(rad) * point.z;
			point.z = -std::sin(rad) * point.x + std::cos(rad) * point.z;

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
