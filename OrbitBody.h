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
		Why does this class not inherit from Body? Because this is designed to be static. Any similarity between class attributes and methods
		is due to how graphics have been implemented and consistency with naming conventions, not a design oversight.
	*/
private:
	Mesh mesh;

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

		mesh.vertices = _vertices;

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
		mesh.edges = _edges; //I want to put this in its own method, however that's unnecessary as this is static soooo...

	}

public:
	double mass = 1.989E30;
	double mu = 0;
	double scale;
	const double Gravitational_Constant = 6.6743E-11;
	vector3 position;

	CentralBody(double _mass = 1.989E30, double _scale=6.96E8)
	{
		mu = Gravitational_Constant * mass;
		position = { 0, 0, 0 };
		scale = _scale;
		Generate_Vertices(_scale);
	}

	int Draw(Graphyte& g, Camera& c)
	{
		vector3 screen_dimensions = g.Get_Screen_Dimensions(); //Vector3 containing Screen Dimensions, we ignore z
		std::vector<vector3> verts = mesh.vertices;

		for (vector3& p : verts)
		{
			p = c.WorldSpaceToScreenSpace(p, screen_dimensions.x, screen_dimensions.y);

			if (p.z > 0)
			{
				g.pixel(p.x, p.y);
			}
		}

		for (edge edg : mesh.edges)
		{
			if (verts[edg.a].z > 0 && verts[edg.b].z > 0)
			{
				//std::cout << "Debugging C_Body rendering: " << verts[edg.a].Debug() << "\n";
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

	Mesh Get_Mesh()
	{
		return mesh;
	}

	void RegenerateVertices()
	{
		std::cout << "\n Recalculating vertex positions w/ scale: " << scale;
		std::cout << "\n Me: (regen) " << this;
		this->mesh.vertices.clear();
		this->Generate_Vertices(scale);
	}

	void RecalculateMu()
	{
		mu = Gravitational_Constant * mass;
	}
};

class Satellite; //A Forward Declaration so nothing collapses

class Body
{
private:
	std::vector<Satellite*> satellites;
	Graphyte& graphyte;

	int Add_Satellite(Satellite* sat);

	void Delete_Satellites();

	void Create_Satellite();

	int Update_Satellites(float delta, float time_scale, std::vector<Body*>* bodies_in_system);

	int Draw_Satellites(Graphyte& g, Camera& c);

	void Close_Satellite_Inspectors();

	int Clean_Up_Satellites();

protected:
	Mesh mesh;
	vector3 last_trail_point;
	std::vector<vector3> trail_points;

	vector3 start_pos;
	vector3 start_vel;
	double time_since_start = 0;

	//Orbit information
	vector3 position{ 0, 0, 0 };
	double radius;
	vector3 velocity{ 0,0,0 };
	double angular_velocity = 0;
	vector3 acceleration{ 0, 0, 0 };
	double mu = 0; 
	double mass = 0;
	double scale;
	const double Gravitational_Constant = 6.6743E-11;

	//Labels
	Text* name_label = NULL;
	Text* inspector_name = NULL;
	Text* inspector_mass = NULL; 
	Text* inspector_radius = NULL;
	Text* inspector_velocity = NULL;
	Text* inspector_angular_velocity = NULL;
	Text* inspector_acceleration = NULL;
	Text* inspector_period = NULL;

	//Inspector Function Buttons
	FunctionButton* inspector_reset = NULL;
	FunctionButton* inspector_delete = NULL;
	FunctionButton* inspector_satellite = NULL;

	//Field Values
	DoubleFieldValue ScaleFV;
	DoubleFieldValue MassFV;
	DoubleFieldValue PosXFV, PosYFV, PosZFV;
	DoubleFieldValue VelXFV, VelYFV, VelZFV;

	StringFieldValue NameFV;

	//GUI
	GUI_Block* gui = NULL;

	//BUTTON
	FunctionButton* f_button = NULL;
	
	virtual void update_inspector()
	{
		if (gui->is_visible)
		{
			if (inspector_name != NULL){ inspector_name->Set_Text(name);}//The set text method checks if we are making a redundant set => more performant
			if (inspector_mass != NULL) { inspector_mass->Set_Text("| Mass: " + std::to_string(mass) + "kg"); }
			if (inspector_radius != NULL) { inspector_radius->Set_Text("| Radius: " + std::to_string(Magnitude(position) / 1000) + "km"); }
			if (inspector_velocity != NULL) { inspector_velocity->Set_Text("| Velocity: " + velocity.Debug()); }
			if (inspector_angular_velocity != NULL) { inspector_angular_velocity->Set_Text("| Angular Velocity: " + std::to_string(angular_velocity * 60 * 60 * 24) + "rad/day"); }
			if (inspector_acceleration != NULL) { inspector_acceleration->Set_Text("| Acceleration: " + acceleration.Debug()); }
			if (inspector_period != NULL) { inspector_period->Set_Text("| Orbit Period: " + std::to_string(Calculate_Period() / (60 * 60 * 24)) + " days"); }
		}
	}

	std::vector<vector3> two_body_ode(float t, vector3 _r, vector3 _v, std::vector<Body*>* masses)
	{
		vector3 a;
		vector3 pos = _r; //displacement
		vector3 v = _v; //velocity


		//SUN
		vector3 r = pos; //displacement

		vector3 nr = Normalize(r);
		//std::cout << "NR.z" << nr.z << "\n";
		if (nr.z == 0) { nr.z = 1; r.z = 0; }
		if (nr.y == 0) { nr.y = 1; r.y = 0; }
		if (nr.x == 0) { nr.x = 1; r.x = 0; }
		double mag = Magnitude(r);
		a = vector3({
			(-mu * r.x) / (pow(mag, 3)),
			(-mu * r.y) / (pow(mag, 3)),
			(-mu * r.z) / (pow(mag, 3))
			});

		std::cout << "\n" + name +  " " << mag;

		//Others

		for (Body* b : *masses)
		{
			if (b != this)
			{
				vector3 r = pos - b->Get_Position(); //displacement

				double _mu = Gravitational_Constant * b->Get_Mass();

				//std::cout << "R.z" << r.z << "\n";
				vector3 nr = Normalize(r);
				//std::cout << "NR.z" << nr.z << "\n";
				if (nr.z == 0) { nr.z = 1; r.z = 0; }
				if (nr.y == 0) { nr.y = 1; r.y = 0; }
				if (nr.x == 0) { nr.x = 1; r.x = 0; }
				double mag = Magnitude(r);
				a = a + vector3({
					(-_mu * r.x) / (pow(mag, 3)),
					(-_mu * r.y) / (pow(mag, 3)),
					(-_mu * r.z) / (pow(mag, 3))
					});
			}
		}
		//std::cout << "rk_result: " << (pow(nr.z, 3)) << "\n";
		return { v, a };
	}

	std::vector<vector3> rk4_step(float _time, vector3 _position, vector3 _velocity, std::vector<Body*>* masses, float _dt = 1)
	{
		//std::cout << "\n DEBUGGING RK4 STEP FOR: " + name + "\n" + "position: " + _position.Debug() + "\nvelocity: " + _velocity.Debug();
		//structure of the vectors: [pos, velocity]
		std::vector<vector3> rk1 = two_body_ode(_time, _position, _velocity, masses);
		std::vector<vector3> rk2 = two_body_ode(_time + (0.5 * _dt), _position + (rk1[0] * 0.5f * _dt), _velocity + (rk1[1] * 0.5f * _dt), masses);
		std::vector<vector3> rk3 = two_body_ode(_time + (0.5 * _dt), _position + (rk2[0] * 0.5f * _dt), _velocity + (rk2[1] * 0.5f * _dt), masses);
		std::vector<vector3> rk4 = two_body_ode(_time + _dt, _position + (rk3[0] * _dt), _velocity + (rk3[1] * _dt), masses);
		
		vector3 result_pos = _position + (rk1[0] + (rk2[0] * 2.0f) + (rk3[0] * 2.0f) + rk4[0]) * (_dt / 6.0f);
		vector3 result_vel = _velocity + (rk1[1] + rk2[1] * 2 + rk3[1] * 2 + rk4[1]) * (_dt / 6);
		//std::cout << "\n Result FOR: " + name + "\n" + "position: " + result_pos.Debug() + "\nvelocity: " + result_vel.Debug();
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
		mesh.edges = _edges;

		printf("\n Generated vertices");

		return _vertices;
	}

	void MoveToPos(vector3 new_pos)
	{
		vector3 old_pos = position;
		position = new_pos;
		radius = Magnitude(position);

		vector3 delta = position - old_pos;

		for (auto& p : mesh.vertices)
		{
			p = p + delta;
		}

		return;
	}

	vector3 rotate(vector3 rot, vector3 point, vector3 c) //Something is broken. STILL BROKEN
	{

		//centroid adjustments
		point.x -= c.x;
		point.y -= c.y;
		point.z -= c.z;

		//float start_magnitude = Magnitude(point);

		//Rotate point
		float rad = 0;
		float x, y, z;
		rad = rot.x;

		x = point.x;
		y = point.y;
		z = point.z;

		point.y = (std::cos(rad) * y) - (std::sin(rad) * z);
		point.z = (std::sin(rad) * y) + (std::cos(rad) * z);

		x = point.x;
		y = point.y;
		z = point.z;

		rad = rot.y;
		point.x = (std::cos(rad) * x) + (std::sin(rad) * z);
		point.z = (-std::sin(rad) * x) + (std::cos(rad) * z);

		x = point.x;
		y = point.y;
		z = point.z;

		rad = rot.z;
		point.x = (std::cos(rad) * x) - (std::sin(rad) * y);
		point.y = (std::sin(rad) * x) + (std::cos(rad) * y);

		//centroid adjustments
		point.x += c.x;
		point.y += c.y;
		point.z += c.z;

		return point;

	}

	void rotate_about_centre(vector3 rot)
	{
		for (auto& p : mesh.vertices)
		{
			p = rotate(rot, p, position);
		}
	}

	void CreateInspector(Graphyte& g)
	{
		// TODO: Generate Orbit-Body specific GUI Blocks that can be toggled visibility. This'll be a challenge, good luck!
		gui = new GUI_Block(); // I suspect this is about to become null once the constructor finishes, but who know!
		vector3 screen_dimensions = g.Get_Screen_Dimensions();
		gui->position = { (screen_dimensions.x / 2) - 300, -(screen_dimensions.y / 2) + 330, 0 };
		inspector_name = g.CreateText(name + ": ", 12);
		gui->Add_Stacked_Element(inspector_name);
		std::cout << inspector_name;
		inspector_mass = g.CreateText(std::to_string(mass), 12);
		gui->Add_Stacked_Element(inspector_mass);
		inspector_radius = g.CreateText(std::to_string(Magnitude(position)), 12);
		gui->Add_Stacked_Element(inspector_radius);
		inspector_velocity = g.CreateText("velocity should be here", 12);
		gui->Add_Stacked_Element(inspector_velocity);
		inspector_angular_velocity = g.CreateText("angular velocity should be here", 12);
		gui->Add_Stacked_Element(inspector_angular_velocity);
		inspector_acceleration = g.CreateText("acceleration should be here", 12);
		gui->Add_Stacked_Element(inspector_acceleration);
		inspector_period = g.CreateText("period should be here", 12);
		gui->Add_Stacked_Element(inspector_period);

		//Input fields
		gui->Add_Stacked_Element(g.CreateText("EDIT PARAMETERS_____", 12));

		gui->Add_Stacked_Element(g.CreateText("| Name: ", 10));
		TextField* tf = new TextField({ 10,10,0 }, NameFV, g, name);
		g.text_fields.push_back(tf);
		gui->Add_Inline_Element(tf);

		gui->Add_Stacked_Element(g.CreateText("| Scale: ", 10));
		tf = new TextField({ 10,10,0 }, ScaleFV, g, std::to_string(scale));
		g.text_fields.push_back(tf);
		gui->Add_Inline_Element(tf);

		gui->Add_Stacked_Element(g.CreateText("| Mass: ", 10));
		tf = new TextField({ 10,10,0 }, MassFV, g, std::to_string(mass));
		g.text_fields.push_back(tf);
		gui->Add_Inline_Element(tf);

		//POSITION:

		gui->Add_Stacked_Element(g.CreateText("| Position x: ", 10));
		tf = new TextField({ 10,10,0 }, PosXFV, g, std::to_string(position.x));
		g.text_fields.push_back(tf);
		gui->Add_Inline_Element(tf);

		gui->Add_Stacked_Element(g.CreateText("| Position y: ", 10));
		tf = new TextField({ 10,10,0 }, PosYFV, g, std::to_string(position.y));
		g.text_fields.push_back(tf);
		gui->Add_Inline_Element(tf);

		gui->Add_Stacked_Element(g.CreateText("| Position z: ", 10));
		tf = new TextField({ 10,10,0 }, PosZFV, g, std::to_string(position.z));
		g.text_fields.push_back(tf);
		gui->Add_Inline_Element(tf);

		//VELOCITY:

		gui->Add_Stacked_Element(g.CreateText("| Velocity x: ", 10));
		tf = new TextField({ 10,10,0 }, VelXFV, g, std::to_string(velocity.x));
		g.text_fields.push_back(tf);
		gui->Add_Inline_Element(tf);

		gui->Add_Stacked_Element(g.CreateText("| Velocity y: ", 10));
		tf = new TextField({ 10,10,0 }, VelYFV, g, std::to_string(velocity.y));
		g.text_fields.push_back(tf);
		gui->Add_Inline_Element(tf);

		gui->Add_Stacked_Element(g.CreateText("| Velocity z: ", 10));
		tf = new TextField({ 10,10,0 }, VelZFV, g, std::to_string(velocity.z));
		g.text_fields.push_back(tf);
		gui->Add_Inline_Element(tf);

		//FUNCTION BUTTONS:

		//Create the button
		f_button = new FunctionButton([this]() { this->ShowBodyInspector(); }, name_label->Get_Position(), name_label->Get_Dimensions(), g, "", [this]() { this->snap_camera_to_body(); }); //TODO: Fix this please
		g.function_buttons.push_back(f_button); //No idea how this has access to function_buttons but so it does...
		std::cout << "\n\n\n\nFUNCTION BUTTON " << f_button;

		inspector_delete = new FunctionButton([this]() { this->Delete(); }, { (screen_dimensions.x / 2) - 275, -(screen_dimensions.y / 2) + 30, 0 }, {25, 25, 0}, g, "icons/delete.png");
		g.function_buttons.emplace_back(inspector_delete);

		inspector_reset = new FunctionButton([this]() { this->Reset(); }, { (screen_dimensions.x / 2) - 245, -(screen_dimensions.y / 2) + 30, 0 }, { 25, 25, 0 }, g, "icons/reset.png");
		g.function_buttons.emplace_back(inspector_reset);

		inspector_satellite = new FunctionButton([this]() { this->Create_Satellite(); }, {(screen_dimensions.x / 2) - 215, -(screen_dimensions.y / 2) + 30, 0}, {25, 25, 0}, g, "icons/add.png");
		g.function_buttons.emplace_back(inspector_satellite);

		HideBodyInspector();
	}

	void snap_camera_to_body()
	{
		snap_camera = true;
	}

public: 
	std::string name;
	bool to_delete = false; //Used in mainloop to schedule objects for deletion next update. => deconstructor (see free())
	bool snap_camera = false;

	Body(std::string _name, vector3 _center, double _mass, double _scale, vector3 _velocity, double _mu, Graphyte& g, bool override_velocity = false):
		graphyte(g), 
		ScaleFV(&scale, [this]() { this->RegenerateVertices(); }), MassFV(&mass), NameFV(&name, [this]() { this->Rename(); }), // Scale Dield Value. When written to, recalculate geometry
		PosXFV(&this->position.x, [this]() { this->RecenterBody(); }), PosYFV(&this->position.y, [this]() { this->RecenterBody(); }), PosZFV(&this->position.z, [this]() { this->RecenterBody(); }),
		VelXFV(&this->velocity.x, [this]() { this->SetStartVelocity(); }), VelYFV(&this->velocity.y, [this]() { this->SetStartVelocity(); }), VelZFV(&this->velocity.z, [this]() { this->SetStartVelocity(); })
	{
		position = _center;
		start_pos = position;
		radius = Magnitude(position);

		name_label = g.CreateText(_name, 16); // Create floating text label that follows orbit body
		name_label->pos_x = 100; //Test values (overwritten later)
		name_label->pos_y = 100;

		mu = _mu; //Setting attributes
		name = _name;
		if (override_velocity)
		{
			Project_Circular_Orbit(_velocity); // Force a circular orbit. Not reccommended as will override given parameters. [NO LONGER SUPPORTED]
		}
		velocity = _velocity;
		start_vel = velocity;

		scale = _scale;
		mass = _mass;

		std::cout << "Instantiated Orbiting Body with initial position: " << start_pos.Debug() << " and velocity: " << velocity.Debug() << "\n";
		
		mesh.vertices = Generate_Vertices(scale); // Generate body geometry

		CreateInspector(g); // Create GUI for body
	}

	~Body()
	{
		free();
	}

	void free() //I *WANT* to improve this, but the NEA deadline is looming so this is how it stays for now.
	{
		
		satellites.clear();
		mesh.vertices.clear();
		trail_points.clear();
		mesh.edges.clear();

		gui = nullptr;
		f_button->SetEnabled(false);
		f_button = nullptr;
	}

	void RegenerateVertices()
	{
		std::cout << "\n Recalculating vertex positions w/ scale: " << scale;
		std::cout << "\n Me: (regen) " <<this;
		this->mesh.vertices.clear();
		this->mesh.vertices = this->Generate_Vertices(scale);
		std::cout << "\n" << Magnitude(mesh.vertices[0] - position);
	}

	void RecenterBody()
	{
		MoveToPos(position);
		start_pos = position;
		time_since_start = 0;
	}

	void SetStartVelocity()
	{
		start_vel = velocity;
		time_since_start = 0;
	}

	void Rename()
	{
		std::cout << "\nRenamed an orbiting body.";
		name_label->Set_Text(name);
		return;
	}

	void ShowBodyInspector()
	{
		inspector_delete->SetEnabled(true);
		inspector_reset->SetEnabled(true);
		inspector_satellite->SetEnabled(true);
		gui->Show();
	}

	void HideBodyInspector()
	{
		inspector_delete->SetEnabled(false);
		inspector_reset->SetEnabled(false);
		inspector_satellite->SetEnabled(false);
		gui->Hide();
		Close_Satellite_Inspectors();
	}

	OrbitBodyData GetOrbitBodyData() //To be used when saving to a .orbyte file
	{
		return OrbitBodyData(name, position, mass, scale, velocity);
	}

	std::string DebugBody() //For debugging purposes...
	{
		std::string text = name + " velocity: " + velocity.Debug() + "|| Time: " + std::to_string(time_since_start);
		return text;
	}

	void Reset()
	{
		time_since_start = 0;
		position = start_pos;
		radius = Magnitude(position);
		velocity = start_vel;
		RegenerateVertices();
	}

	void Delete()
	{
		std::cout << "\n|||DELETED ORBIT BODY: " << name << "|||";
		HideBodyInspector();
		name_label->Set_Visibility(false);
		f_button->SetEnabled(false);
		to_delete = true;
		Delete_Satellites();
	}

	virtual int Update_Body(float delta, float time_scale, std::vector<Body*>* bodies_in_system)
	{
		if (time_scale == 0) // If paused, don't update.
		{
			return 0;
		} 

		Update_Satellites(delta, time_scale, bodies_in_system); // Call Update Method of all child satellites

		rotate_about_centre({0.01, 0.01, 0.01}); // Gradual rotation about body origin to mimic a planet's rotation about its axis

		vector3 this_pos = position;
		float t = (delta / 1000); //time in seconds
		std::vector<vector3> sim_step = rk4_step(time_since_start, this_pos, velocity, bodies_in_system, t * time_scale); // Get RK4 result into a sim_step buffer.
		this_pos = sim_step[0];
		//if (position.z > 0) { std::cout << position.Debug() << "\n"; std::cout << velocity.Debug() << "\n"; }
		MoveToPos(this_pos); // Shift vertices to new position
		angular_velocity = Magnitude(velocity) / Magnitude(position); // angular velocity = tangential velocity / radius
		time_since_start += t * time_scale;

		// Add a "breadcrumb" or trail point if a certain distance away from last
		if (Magnitude(this_pos - last_trail_point) > (0.5 * radius) / 24)
		{
			trail_points.emplace_back(this_pos);
			last_trail_point = this_pos;
		}

		// Remove trail point to only show most recent 
		if (trail_points.size() > 24)
		{
			trail_points.erase(trail_points.begin());
		}
		
		velocity = sim_step[1]; // Get result from RK4 buffer
		acceleration = sim_step[2];

		update_inspector(); // Update GUI

		return 0; // Successful update.
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
		std::vector<vector3> verts = this->mesh.vertices;
		for (auto& p : verts)
		{
			p = c.WorldSpaceToScreenSpace(p, screen_dimensions.x, screen_dimensions.y);

			if (p.z > 0)
			{
				g.pixel(p.x, p.y);
			}
		}

		for (vector3 t_p : trail_points)
		{
			t_p = c.WorldSpaceToScreenSpace(t_p, screen_dimensions.x, screen_dimensions.y);
			if (t_p.z > 0)
			{
				g.pixel(t_p.x, t_p.y);
				//std::cout << t_p.y << "\n"; //testing a hunch
			}
		}

		for (edge edg : mesh.edges)
		{
			if (verts[edg.a].z > 0 && verts[edg.b].z > 0)
			{
				//std::cout << verts[edg.a].Debug() << "\n";
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
		vector3 end2 = end1 + vector3{ (double)name_label->Get_Texture().getWidth(), 0, 0 };
		vector3 label_pos = end1 + ((end2 - end1) * 0.5);
		label_pos.y += (double)name_label->Get_Texture().getHeight() / 2;
		//Move & Draw Label: Done in the draw method because we need access to the camera and creating a new method makes no sense.
		g.line(start.x, start.y, end1.x, end1.y);
		g.line(end1.x, end1.y, end2.x, end2.y);
		name_label->Set_Position(label_pos);
		if (f_button != NULL)
		{
			f_button->SetPosition(label_pos);
		}
		
		Draw_Arrows(g, c, start, screen_dimensions);

		Draw_Satellites(g, c);

		return 0;
	}

	Mesh Get_Mesh()
	{
		//Return vertices
		return mesh;
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

	double Get_Mass()
	{
		return mass;
	}

	vector3 Get_Acceleration()
	{
		return acceleration;
	}

	void Set_Mu(double _mu);

	double Get_Mu()
	{
		return mu;
	}

	/// <summary>
	/// The amount of time in seconds for the body to complete 1 orbit
	/// </summary>
	/// <returns>Time period</returns>
	virtual double Calculate_Period()
	{
		double T = 2 * 3.14159265359 * sqrt((pow(radius, 3) / mu)); //2 * pi * R
		double length_of_orbit = 2 * 3.14159265359 * radius; //// 2 * pi * R
		double t = length_of_orbit / Magnitude(velocity); //THIS GIVES CORRECT VALUE
		//std::cout << name <<" Orbit Characteristics: \n" << T << " seconds | Calculated orbit period\n" << length_of_orbit << " metres\n" << t << " other t value\n" << mu << "\n";
		return T;
	}
};

class Satellite : public Body
{
private:
	Body* parentBody; // Pointer to parent, e.g. Moon -> Earth
	//Satellites have different geometry! Cube.
	std::vector<vector3> Generate_Vertices(double scale) override {
		//Polymorphism!
		std::vector<vector3> _vertices{
			{0, 1, 1},
			{1, 0, 1},
			{0, -1, 1},
			{-1, 0, 1},

			{0, 1, -1},
			{1, 0, -1},
			{0, -1, -1},
			{-1, 0, -1},
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
			{0,1},
			{0, 3},
			{2, 1},
			{2, 3},

			{4, 5},
			{4, 7},
			{6, 5},
			{6, 7},

			{0, 4},
			{1, 5},
			{2, 6},
			{3, 7}
		};
		mesh.edges = _edges;

		return _vertices;
	};
	//Override Circular Orbit Projection [NO LONGER SUPPORTED]
	void Project_Circular_Orbit(vector3& _velocity) override {
		vector3 p_velocity = parentBody->Get_Tangential_Velocity();
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
		std::cout << "Someone has projected a circular orbit!";
		_velocity = _velocity + p_velocity; // Adding the parent velocity because we need this to orbit something moving through space, not orbiting where it thought it was.

		//Don't have to return a value because parameter is passed by reference.
	}

protected:
	//Override Inspector Values
	void update_inspector() override
	{
		if (gui->is_visible)
		{
			if (inspector_name != NULL) { inspector_name->Set_Text(parentBody->name + "'s: " + name); }//The set text method checks if we are making a redundant set => more performant
			if (inspector_mass != NULL) { inspector_mass->Set_Text("| Mass: " + std::to_string(mass) + "kg"); }
			// Override Radius
			if (inspector_radius != NULL) { inspector_radius->Set_Text("| Radius: " + std::to_string(Magnitude(position - parentBody->Get_Position()) / 1000) + "km"); }
			// Relative Velocity
			if (inspector_velocity != NULL) { inspector_velocity->Set_Text("| Velocity: " + (velocity - parentBody->Get_Tangential_Velocity()).Debug()); }
			if (inspector_angular_velocity != NULL) { inspector_angular_velocity->Set_Text("| Angular Velocity: " + std::to_string(angular_velocity * 60 * 60 * 24) + "rad/day"); }
			if (inspector_acceleration != NULL) { inspector_acceleration->Set_Text("| Acceleration: " + (acceleration - parentBody->Get_Acceleration()).Debug()); }
			if (inspector_period != NULL) { inspector_period->Set_Text("| Orbit Period: " + std::to_string(Calculate_Period() / (60 * 60 * 24)) + " days"); }
		}
	}
	//Override Period Calculation
	double Calculate_Period() override
	{
		double T = 2 * 3.14159265359 * sqrt((pow(radius, 3) / mu));
		double length_of_orbit = 2 * 3.14159265359 * (Magnitude(parentBody->Get_Position() - position)); //YEP
		double t = length_of_orbit / Magnitude(velocity - parentBody->Get_Tangential_Velocity());
		//std::cout << name <<" Orbit Characteristics: \n" << T << " seconds | Calculated orbit period\n" << length_of_orbit << " metres\n" << t << " other t value\n" << mu << "\n";
		return t;
	}

public:
	//Constructor
	Satellite(std::string _name, Body* _parentBody, vector3 center, double _mass, double _scale, vector3 _velocity, Graphyte& g, bool override_velocity = false): 
		Body(_name, center + _parentBody->Get_Position(), _mass, _scale, _velocity + _parentBody->Get_Tangential_Velocity(), _parentBody->Get_Mu(), g, false), parentBody(_parentBody)
	{
		std::cout << "\n____________\nSATELLITE INSTANTIATION\n____________\n" << "parent body name: " << parentBody->name << "\nparent body location: " << parentBody->Get_Position().Debug() << "\nmy location: " << Get_Position().Debug() + "\n";
		std::cout << "\nSAT POS (RELATIVE) CONSTRUCTOR:" + (position).Debug() + "\n";
		std::cout << "SAT VEL (RELATIVE) CONSTRUCTOR:" + (velocity).Debug() + " MEANT TO BE: " + _velocity.Debug() + "\n";
	}
	//Override Update
	int Update_Body(float delta, float time_scale, std::vector<Body*>* bodies_in_system) override
	{
		if (time_scale == 0) // If paused, don't update.
		{
			return 0;
		}

		//rotate(0.0005f, 0.0005f, 0.0005f);
		vector3 this_pos = position;
		//std::cout << "\n" + name << " SAT POS (RELATIVE):" + this_pos.Debug() + "\n";
		float t = (delta / 1000); //time in seconds
		std::vector<vector3> sim_step = rk4_step(time_since_start, this_pos, velocity, bodies_in_system, t * time_scale);
		this_pos = sim_step[0];
		radius = Magnitude(this_pos - parentBody->Get_Position());
		
		MoveToPos(this_pos);
		angular_velocity = Magnitude(velocity - parentBody->Get_Tangential_Velocity()) / Magnitude(position - parentBody->Get_Position());
		time_since_start += t * time_scale;

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
		//std::cout << "SAT VEL (RELATIVE):" + (velocity - parentBody->Get_Tangential_Velocity()).Debug() + "\n";
		acceleration = sim_step[2];
		//std::cout << "\nSatellite Accel: " << Normalize(acceleration).Debug();

		update_inspector();

		return 0;
	}
};

int Body::Update_Satellites(float delta, float time_scale, std::vector<Body*>* bodies_in_system)
{
	//Now update Satellites
	Clean_Up_Satellites();
	for (Satellite* sat : satellites)
	{
		sat->Update_Body(delta, time_scale, bodies_in_system);
	}
	return 0;
}

int Body::Add_Satellite(Satellite* sat)
{
	satellites.emplace_back(sat);
	std::cout << name << " has a new satellite: " << sat->name << " | Total number of satellites: " << satellites.size() << "\n";
	return satellites.size() - 1;
}

void Body::Create_Satellite()
{
	// TODO: Figure this out I guess!
	//Add_Satellite(Satellite("Moon", this, { 3.8E8, 0, 0 }, 7.3E22, 1.7E5, { 0, -1200, 0 }, graphyte, false)); //Continue with this.
	Satellite* new_sat = new Satellite("Moon", this, { 3.8E8, 0, 0 }, 7.3E24, 1.7E5, { 0, -1200, 0 }, graphyte, false);
	Add_Satellite(new_sat); //Continue with this.
}

void Body::Delete_Satellites()
{
	for (Satellite* sat : satellites)
	{
		sat->Delete();
	}
	Clean_Up_Satellites();
}

int Body::Clean_Up_Satellites()
{
	int length = satellites.size();
	for (int i = 0; i < length; i += 0) //This is an odd loop
	{
		if (satellites[i]->to_delete)
		{
			satellites.erase(satellites.begin() + i);
			length -= 1;
		}
		else {
			i++;
		}
	}
	return 0;
}

void Body::Close_Satellite_Inspectors()
{
	for (Satellite* sat : satellites)
	{
		sat->HideBodyInspector();
	}
}

void Body::Set_Mu(double _mu)
{
	mu = _mu;
	for (Satellite* s : satellites)
	{
		s->Set_Mu(mu);
	}
}

int Body::Draw_Satellites(Graphyte& g, Camera& c)
{
	for (Satellite* sat : satellites)
	{
		sat->Draw(g, c);
		
	}
	return 0;
}
#endif /*ORBITBODY_H*/
 