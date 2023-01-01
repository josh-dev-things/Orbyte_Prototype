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
#include "Orbyte_Data.h"
#include "Orbyte_Graphics.h"
#include "Camera.h"

class CentralBody
{
	/*
		Why does this class not inherit from Orbit Body? Because this is designed to be static. Any similarity between class attributes and methods
		is due to how graphics have been implemented and consistency with naming conventions, not a design oversight.
	*/
private:
	std::vector<vector3> vertices;
	std::vector<edge> edges;

	void Generate_Vertices(double scale)
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
			v.x += position.x;
			v.y *= scale;
			v.y += position.y;
			v.z *= scale;
			v.z += position.z;
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
			{1,5},

			{2,4},
			{2,5},
			{3,4},
			{3,5}
		};
		edges = _edges; //I want to put this in its own method, however that's unnecessary as this is static soooo...

	}

public:
	double mass = 1.989E30;
	double mu = 0;
	const double Gravitational_Constant = 6.6743E-11;
	vector3 position;

	CentralBody(double _mass = 1.989E30, double _scale=6.96E8)
	{
		mu = Gravitational_Constant * mass;
		position = { 0, 0, 0 };
		Generate_Vertices(_scale);
	}

	int Draw(Graphyte& g, Camera& c)
	{
		vector3 screen_dimensions = g.Get_Screen_Dimensions(); //Vector3 containing Screen Dimensions, we ignore z
		std::vector<vector3> verts = Get_Vertices(); //Why are we using an accessor inside the class? Because its tidy and we need to get all the vertices in a new structure so that we can write the screen space positions by reference.

		for (auto& p : verts)
		{
			p = c.WorldSpaceToScreenSpace(p, screen_dimensions.x, screen_dimensions.y);

			if (p.z > 0)
			{
				g.pixel(p.x, p.y);
			}
		}

		for (auto edg : edges)
		{
			if (verts[edg.a].z > 0 && verts[edg.b].z > 0)
			{
				g.line(verts[edg.a].x,
					verts[edg.a].y,
					verts[edg.b].x,
					verts[edg.b].y
				);
			}
		}

		verts.clear();

		return 0;
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

class Satellite; //A Forward Declaration so nothing collapses

class Body
{
private:
	std::vector<Satellite> satellites;

protected:
	std::vector<vector3> vertices;
	std::vector<edge> edges;
	vector3 last_trail_point;
	std::vector<vector3> trail_points;
	double scale;
	CentralBody central_body;

	vector3 start_pos;
	vector3 start_vel;
	double time_since_start = 0;

	//Orbit information
	vector3 position{ 0, 0, 0 };
	double radius;
	vector3 velocity{ 0,0,0 };
	vector3 acceleration{ 0, 0, 0 };
	double mu = 0;

	//Label
	Text* label = NULL;
	
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
		double mag = Magnitude(r);
		vector3 a = {
			(-mu * r.x) / (pow(mag, 3)),
			(-mu * r.y) / (pow(mag, 3)),
			(-mu * r.z) / (pow(mag, 3))
		};
		//std::cout << "rk_result: " << (pow(nr.z, 3)) << "\n";
		return { v, a };
	}

	//rk4_step function https://www.youtube.com/watch?v=TzX6bg3Kc0E&t=241s
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
		return { result_pos, result_vel, rk1[1]};
	}

	//We need to override initial velocities in case user wants a perfectly circular orbit.
	virtual void Project_Circular_Orbit(vector3& _velocity)
	{
		//We manipulate the velocity so that a perfectly circular orbit is achieved
		if (position.x != 0)
		{
			_velocity.y = sqrt(mu / position.x);
		}
		if (position.y != 0)
		{
			_velocity.x = sqrt(mu / position.y);
		}
		if (position.z != 0)
		{
			_velocity.x = sqrt(mu / position.z);
		}

		//Don't have to return a value because parameter is passed by reference.
	}

	//Hooray, polymorphism!
	virtual std::vector<vector3> Generate_Vertices(double scale)
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
			v.x += position.x;
			v.y *= scale;
			v.y += position.y;
			v.z *= scale;
			v.z += position.z;
		}

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
		edges = _edges; //I want to put this in its own method, however that's unnecessary as this is static soooo...

		return _vertices;
	}

	void MoveToPos(vector3 new_pos)
	{
		vector3 old_pos = position;
		position = new_pos;

		vector3 delta = position - old_pos;

		for (auto& p : vertices)
		{
			p = p + delta;
		}
	}

	void rotate(float rot_x = 1, float rot_y = 1, float rot_z = 1) //Something is broken. STILL BROKEN
	{
		for (auto& p : vertices)
		{
			vector3 point = p;
			//centroid adjustments
			point = point - position;

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
			point = point + position;

			p = point;
		}
	}

public: 
	std::string name;
	Body(std::string _name, vector3 _center, double _scale, vector3 _velocity, CentralBody c_body, Graphyte& g, bool override_velocity = true):
		central_body{c_body}
	{
		position = _center;
		radius = Magnitude(position);

		label = g.CreateText(_name, 16);
		label->pos_x = 100;
		label->pos_y = 100;

		mu = central_body.mu;
		name = _name;
		if (override_velocity)
		{
			Project_Circular_Orbit(_velocity);
		}
		velocity = _velocity;
		start_vel = velocity;

		scale = _scale;

		start_pos = position;
		std::cout << "Instantiated Orbiting Body with initial position: " << start_pos.Debug() << " and velocity: " << velocity.Debug() << "\n";
		
		vertices = Generate_Vertices(scale);
	}

	OrbitBodyData GetOrbitBodyData() //To be used when saving to a .orbyte file
	{
		return OrbitBodyData(name, position, scale, velocity, false);
	}

	std::string GetBodyData() //For debugging purposes...
	{
		std::string text = name + " velocity: " + velocity.Debug() + "|| Time: " + std::to_string(time_since_start);
		return text;
	}

	void Reset()
	{
		position = start_pos;

		velocity = start_vel;
	}

	int Add_Satellite(const Satellite& sat); //This is defined after Satellite is defined.

	int Update_Satellites(float delta, float time_scale);

	int Draw_Satellites(Graphyte& g, Camera& c);

	int Update_Body(float delta, float time_scale)
	{
		time_since_start += delta * time_scale;
		rotate(0.0005f * time_scale, 0.0005f * time_scale, 0.0005f * time_scale);
		vector3 this_pos = position; //god_pos is normally origin, but not for a satellite.
		float t = (delta / 1000);
		std::vector<vector3> sim_step = rk4_step(t * time_scale, this_pos, velocity, t * time_scale);
		this_pos = sim_step[0];
		//if (position.z > 0) { std::cout << position.Debug() << "\n"; std::cout << velocity.Debug() << "\n"; }
		MoveToPos(this_pos);


		if (Magnitude(this_pos - last_trail_point) > (0.5 * radius) / 24)
		{
			trail_points.emplace_back(this_pos);
			last_trail_point = this_pos;
			//printf("Added Point");
		}
		if (trail_points.size() > 24)
		{
			//printf("Erased Point");
			trail_points.erase(trail_points.begin());
		}
		
		velocity = sim_step[1];
		acceleration = sim_step[2];

		Update_Satellites(delta, time_scale);
		return 0;
	}

	int Draw_Arrows(Graphyte& g, Camera& c, vector3 start, vector3 screen_dimensions)
	{
		//Draw arrow for velocity
		Arrow arrow_velocity;
		double arrow_modifier = c.position.z < 0 ? c.position.z * -(1 / 1E6) : c.position.z * (1 / 1E6);
		vector3 arrow_end = c.WorldSpaceToScreenSpace(position + (velocity * arrow_modifier), screen_dimensions.x, screen_dimensions.y);
		vector3 dir = arrow_end - start;
		arrow_velocity.Draw(start, Normalize(dir), Magnitude(dir), 1, g); //Draw arrow, with 1 head.

		//Draw arrow for acceleration
		Arrow arrow_acceleration;
		arrow_end = c.WorldSpaceToScreenSpace(position + (acceleration * arrow_modifier * 5E5), screen_dimensions.x, screen_dimensions.y);
		dir = arrow_end - start;
		arrow_acceleration.Draw(start, Normalize(dir), Magnitude(dir), 2, g); //Draw arrow, with 2 heads.

		return 0;
	}

	int Draw(Graphyte& g, Camera& c)
	{
		vector3 screen_dimensions = g.Get_Screen_Dimensions(); //Vector3 containing Screen Dimensions, we ignore z
		std::vector<vector3> verts = Get_Vertices(); //Why are we using an accessor inside the class? Because its tidy and we need to get all the vertices in a new structure so that we can write the screen space positions by reference.

		for (auto& p : verts)
		{
			p = c.WorldSpaceToScreenSpace(p, screen_dimensions.x, screen_dimensions.y);

			if (p.z > 0)
			{
				g.pixel(p.x, p.y);
			}
		}

		for (auto t_p : trail_points)
		{
			t_p = c.WorldSpaceToScreenSpace(t_p, screen_dimensions.x, screen_dimensions.y);
			if (t_p.z > 0)
			{
				g.pixel(t_p.x, t_p.y);
			}
		}

		for (auto edg : edges)
		{
			if (verts[edg.a].z > 0 && verts[edg.b].z > 0)
			{
				g.line(verts[edg.a].x,
					verts[edg.a].y,
					verts[edg.b].x,
					verts[edg.b].y
				);
			}
		}

		verts.clear();

		//Make two lines for the orbit body label:
		vector3 start = position;
		vector3 end1 = position + vector3{scale, -scale, 0};
		start = c.WorldSpaceToScreenSpace(start, screen_dimensions.x, screen_dimensions.y);
		end1 = c.WorldSpaceToScreenSpace(end1, screen_dimensions.x, screen_dimensions.y);
		vector3 end2 = end1 + vector3{ (double)label->Get_Texture().getWidth(), 0, 0 };
		vector3 label_pos = end1 + ((end2 - end1) * 0.5);
		label_pos.y += (double)label->Get_Texture().getHeight() / 2;
		//Move & Draw Label: Done in the draw method because we need access to the camera and creating a new method makes no sense.
		g.line(start.x, start.y, end1.x, end1.y);
		g.line(end1.x, end1.y, end2.x, end2.y);
		label->Set_Position(label_pos);
		
		
		Draw_Arrows(g, c, start, screen_dimensions);

		Draw_Satellites(g, c);

		return 0;
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

	std::vector<vector3> Get_Trail_Points()
	{
		return trail_points;
	}

	vector3 Get_Tangential_Velocity()
	{
		return velocity;
	}

	vector3 Get_Position()
	{
		return position;
	}

	/// <summary>
	/// The amount of time in seconds for the body to complete 1 orbit
	/// </summary>
	/// <returns>Time period</returns>
	double Calculate_Period()
	{
		double T = 2 * 3.14159265359 * sqrt((pow(radius, 3) / mu)); //THIS DOES NOT GIVE A GOOD VALUE :(
		double length_of_orbit = 2 * 3.14159265359 * radius; //YEP
		double t = length_of_orbit / Magnitude(velocity); //THIS GIVES CORRECT VALUE
		std::cout << name <<" Orbit Characteristics: \n" << T << " seconds | Calculated orbit period\n" << length_of_orbit << " metres\n" << t << " other t value\n" << mu << "\n";
		return T;
	}
};

class Satellite : public Body
{
private:
	Body parentBody;
	std::vector<vector3> Generate_Vertices(double scale) override {
		//thing
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
			v.x += position.x;
			v.y *= scale;
			v.y += position.y;
			v.z *= scale;
			v.z += position.z;
		}

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
		edges = _edges; //I want to put this in its own method, however that's unnecessary as this is static soooo...

		return _vertices;
	};

	void Project_Circular_Orbit(vector3& _velocity) override {
		vector3 p_velocity = parentBody.Get_Tangential_Velocity();
		//We manipulate the velocity so that a perfectly circular orbit is achieved
		if (position.x != 0)
		{
			_velocity.y = sqrt(mu / position.x);
		}
		if (position.y != 0)
		{
			_velocity.x = sqrt(mu / position.y);
		}
		if (position.z != 0)
		{
			_velocity.x = sqrt(mu / position.z);
		}

		_velocity = _velocity + p_velocity; // Adding the parent velocity because we need this to orbit something moving through space, not orbiting where it thought it was.

		//Don't have to return a value because parameter is passed by reference.
	}
public:
	/// <summary>
	/// Constructor for the Satellite object. It initializes the parent class: Body, and the parentBody attribute.
	/// </summary>
	/// <param name="_name"></param>
	/// <param name="_parentBody"></param>
	/// <param name="center"></param>
	/// <param name="_scale"></param>
	/// <param name="_velocity"></param>
	/// <param name="override_velocity"></param>
	Satellite(std::string _name, Body _parentBody, vector3 center, float _scale, vector3 _velocity, Graphyte& g, bool override_velocity = true): 
		Body(_name, center + _parentBody.Get_Position(), _scale, _velocity, CentralBody(), g, override_velocity), parentBody(_parentBody)
	{
		//Now do some satellite thingies I guess

	}
};

int Body::Add_Satellite(const Satellite& sat)
{
	satellites.emplace_back(sat);
	std::cout << name << " has a new satellite: " << sat.name << " | Total number of satellites: " << satellites.size() << "\n";
	return 0;
}

int Body::Update_Satellites(float delta, float time_scale)
{
	//Now update Satellites
	for (auto& sat : satellites)
	{
		sat.Update_Body(delta, time_scale);
	}
	return 0;
}

int Body::Draw_Satellites(Graphyte& g, Camera& c)
{
	for (auto& sat : satellites)
	{
		sat.Draw(g, c);
	}
	return 0;
}
#endif /*ORBITBODY_H*/
