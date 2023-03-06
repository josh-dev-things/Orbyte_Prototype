#pragma once
#ifndef ORBYTE_DATA_H
#define ORBYTE_DATA_H

#include<iostream>
#include<fstream>
#include "vec3.h"
#include <bitset>
#include "utils.h"
#include "OrbitBody.h"
#include <ShObjIdl_core.h>
#include <Windows.h>

struct OrbitBodyData
{
	//Information for storage
	std::string name; // Name of Body
	vector3 center; // Position of Body
	double mass; // Mass of Body
	double scale; // Scale of Body
	vector3 velocity; // Velocity of Body
	
	uint8_t bytes_for_name; //So the first 8 bits of the file will tell us how many bytes the name contains. Name being the only var with "unlimited length"
	OrbitBodyData(std::string _name = "", vector3 _center = { 0, 0, 0 }, double _mass = 1, double _scale = 1, vector3 _velocity = {0, 0, 0})
	{
		name = _name;
		center = _center;
		scale = _scale;
		velocity = _velocity;
		mass = _mass;
		bytes_for_name = (uint8_t)name.length();
	}
};

//A Hash table of orbit objects
struct OrbitBodyCollection
{
private:
	//101 has been chosen as it is prime and not too close to a power of two 
	// => Maximum of 101 bodies in storage!
	std::vector<OrbitBodyData> data = std::vector<OrbitBodyData>(101);
	int count = 0;

	int Hash(std::string name)
	{
		// Reverse the string. A palindrome could XOR itself
		std::string reverse = name;
		reverse_string(reverse, reverse.length() - 1, 0);

		// XOR to produce Hash
		std::string hash = bitwise_string_xor(name, reverse);
		int total = 0;

		for (int i = 0; i < hash.length(); i++)
		{
			total += int(hash.at(i));
		}

		// Return Index To Write
		return (total % data.size());
	}

	void TryWrite(OrbitBodyData d, int index)
	{
		// Saving one empty space so that Reading does not recurse infinitely
		// Max bodies can store: 100
		if (count < data.size() - 1)
		{
			if (data[index].name == "")
			{
				// If address is empty. Bodies Cannot have no name!
				data[index] = d;
				count++;
				return;
			}
			else
			{
				//Some recursion for collision avoidance with open addressing 
				TryWrite(d, (index + 1) % data.size());
			}
		}
		else {
			std::cout << "\nErr. Cannot write to OrbitBodyCollection => Hash Table is full!\n";
		}
	}

	OrbitBodyData TryRead(std::string name, int index)
	{
		if (data[index].name == "")
		{
			//Unsuccessful Read
			return data[index]; //Equivalent to NULL
		}
		if (data[index].name == name)
		{
			//Successful Read
			return data[index];
		}
		else {
			//Increment & read again
			return TryRead(name, (index + 1) % data.size()); //Careful with recursion depth!
		}
	}

public:
	void AddBodyData(OrbitBodyData new_data)
	{
		// Try write data to address generated by Hash() method
		data[Hash(new_data.name)] = new_data;

		// debug
		std::cout << "\n Adding body data with hash: " << Hash(new_data.name)
			<< "\n" << "Testing fetch [If blank, problem!]: "
			<< GetBodyData(new_data.name).name << "\n";
	}

	OrbitBodyData GetBodyData(std::string name)
	{
		// Return OrbitBodyData stored in the Hash table
		// at address given by Hash(name)
		return TryRead(name, Hash(name));
	}

	std::vector<OrbitBodyData> GetAllOrbits()
	{
		std::vector<OrbitBodyData> result;
		for (OrbitBodyData d : data)
		{
			if (d.name != "")
			{
				result.push_back(d);
			}
		}
		return result;
	}
};

struct SimulationData
{
	double cb_mass = 0; // Mass of center body
	double cb_scale = 0; // Scale of center body
	OrbitBodyCollection obc; // All orbits
	vector3 c_pos; // Camera position
};

class DataController
{
public: 
	// Write simulation data to a .orbyte file at specified path.
	int WriteDataToFile(SimulationData sd, std::vector<std::string> bodies_to_save, std::string path)
	{
		double no_orbits = bodies_to_save.size();
		path = "simulations/" + path;

		std::cout << "Writing data to file: " << path << "\n";

		std::ofstream out(path, std::ios::binary | std::ios::out);;
		if (!out)
		{
			std::cout << "\nERR. Writing to file failed.";
			return 1; // Problem
		}
		std::cout << (char*)&sd;
		uint8_t my_size = sizeof(sd);

		out.write((char*)&sd.cb_mass, sizeof(double)); // Center Body Mass
		out.write((char*)&sd.cb_scale, sizeof(double)); // Center Body Scale
		out.write((char*)&sd.c_pos, sizeof(vector3)); // Camera Position
		out.write((char*)&no_orbits, sizeof(double)); // Number Of Orbits

		for (int i = 0; i < bodies_to_save.size(); i++)
		{
			//Access Data via hashing algorithm & hash table
			OrbitBodyData data = sd.obc.GetBodyData(bodies_to_save[i]);

			double bfn = data.bytes_for_name; // Number of characters in name
			out.write((char*)&bfn, sizeof(double));

			std::string n = data.name;
			for (char n_char : n) //Write Name
			{
				out.write((char*)&n_char, sizeof(char));
			}

			vector3 c = data.center; // Body position
			out.write((char*)&c, sizeof(vector3));

			double m = data.mass; // Body Mass
			out.write((char*)&m, sizeof(double));

			double s = data.scale; // Body Scale
			out.write((char*)&s, sizeof(double));

			vector3 v = data.velocity; // Body Velocity
			out.write((char*)&v, sizeof(vector3));
		}
		out.close();

		std::cout << "\nBytes: " << my_size + 1;
		return 0;
	}

	// Read simulation data from a .orbyte file at given path.
	SimulationData ReadDataFromFile(std::string path)
	{
		SimulationData sd; // New Simulation Data
		double no_orbits; // Number of orbits to read
		path = "simulations/" + path;

		std::ifstream in(path, std::ios::binary | std::ios::in);
		if (!in)
		{
			std::cout << "\nERR. Reading from file failed.";
			return sd; //But center mass will be 0, so known error.
		}
		// Mass
		in.read((char*)&sd.cb_mass, sizeof(double)); std::cout << "\nReading mass: " << sd.cb_mass;
		// Scale
		in.read((char*)&sd.cb_scale, sizeof(double)); std::cout << "\nReading scale: " << sd.cb_scale;
		// Camera Position
		in.read((char*)&sd.c_pos, sizeof(vector3)); std::cout << "\nReading c_pos: " << sd.c_pos.Debug();
		// Number of Orbits
		in.read((char*)&no_orbits, sizeof(double)); std::cout << "\nReading No. Orbits: " << no_orbits;

		// Iterate through orbits
		for (int i = 0; i < no_orbits; i++)
		{
			OrbitBodyData data; // New Data

			double bfn; // Bytes for Name
			in.read((char*)&bfn, sizeof(double));
			data.bytes_for_name = bfn;
			std::cout << "\nReading No. bytes: " << (double)data.bytes_for_name;

			std::string name = ""; // Reading name
			for (int j = 0; j < data.bytes_for_name; j++)
			{
				char c; // Character in name
				in.read((char*)&c, sizeof(char));
				name += c;
			}
			data.name = name; std::cout << "\nReading name: " << name;
			// Position
			in.read((char*)&data.center, sizeof(vector3));
			// Mass
			in.read((char*)&data.mass, sizeof(double));
			// Scale
			in.read((char*)&data.scale, sizeof(double));
			// Velocity
			in.read((char*)&data.velocity, sizeof(vector3));
			// Add to OrbitBodyCollection
			sd.obc.AddBodyData(data);
		}

		in.close();


		if (!in.good())
		{
			std::cout << "\n\nErr. Reading from .orbyte file failed!\n\n";
		}
		
		return sd;
	}
};

#endif /*ORBYTE_DATA_H*/
