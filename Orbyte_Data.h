#pragma once
#ifndef ORBYTE_DATA_H
#define ORBYTE_DATA_H

#include<iostream>
#include<fstream>
#include "vec3.h"
#include <bitset>
#include "utils.h"
#include "OrbitBody.h"

struct OrbitBodyData
{
	std::string name;
	vector3 center;
	double mass;
	double scale;
	vector3 velocity;

	//Information for storage
	uint8_t bytes_for_name; //So the first 8 bits of the file will tell us how many bytes the name contains. Name being the only var with "unlimited length"
	OrbitBodyData(std::string _name = "", vector3 _center = { 0, 0, 0 }, double _mass = 1, double _scale = 1, vector3 _velocity = {0, 0, 0})
	{
		name = _name;
		center = _center;
		scale = _scale;
		velocity = _velocity;
		mass = _mass;

		//Number of chars in name = number of bytes => 8 x number of chars = number of bits for name
		//FLOAT is 32 BITS
		//SO VECTOR3 is 3 x 32 BITS

		// 8 x name.length + 3x32 + 32 + 3x32 + 1 = number of bits in file
		bytes_for_name = (uint8_t)name.length();
	}

};

struct OrbitBodyCollection //A Hash table of orbit objects
{
private:

	std::vector<OrbitBodyData> data = std::vector<OrbitBodyData>(101); //101 has been chosen as it is prime and not too close to a power of two => Maximum of 101 bodies in storage!

	int Hash(std::string name)
	{
		std::string reverse = name;
		reverse_string(reverse, reverse.length() - 1, 0);

		std::string hash = bitwise_string_xor(name, reverse);
		int total = 0;

		for (int i = 0; i < hash.length(); i++)
		{
			total += int(hash.at(i));
		}
		return (total % data.size());
	}

	void TryWrite(OrbitBodyData d, int index)
	{
		if (data[index].name == "")
		{
			data[index] = d;
			return;
		}
		else
		{
			TryWrite(d, (index + 1) % data.size()); //Some recursion for collision avoidance with open addressing 
		}
	}

	OrbitBodyData TryRead(std::string name, int index)
	{
		if (data[index].name == "")
		{
			return data[index]; //Equivalent to NULL
		}
		if (data[index].name == name)
		{
			return data[index];
		}
		else {
			return TryRead(name, (index + 1) % data.size()); //Careful with memory usage here!
		}
	}

public:
	void AddBodyData(OrbitBodyData new_data)
	{
		data[Hash(new_data.name)] = new_data;
		std::cout << "\n Adding body data with hash: " << Hash(new_data.name) << "\n" << "Testing fetch [If blank, problem!]: " << GetBodyData(new_data.name).name << "\n";
	}

	OrbitBodyData GetBodyData(std::string name)
	{
		return TryRead(name, Hash(name));
	}
};

struct SimulationData
{
	OrbitBodyCollection obc;
	double cb_scale = 0;
	double cb_mass = 0;
	vector3 c_pos;
	SimulationData(double _cb_mass, double _cb_scale, OrbitBodyCollection _obc, vector3 _c_pos)
	{
		obc = _obc;
		cb_scale = _cb_scale;
		cb_mass = _cb_mass;
		c_pos = _c_pos;
	}
};

//So this header file is going to contain the data controller. The big bad wolf in charge of all the data being read and written out of the application.
class DataController
{
public: 

	std::string EncodeVec3(vector3 vec)
	{
		return EncodeDouble(vec.x) + EncodeDouble(vec.y) + EncodeDouble(vec.z);
	}

	std::string EncodeDouble(double db)
	{
		return std::bitset<64>(db).to_string();
	}

	std::string EncodeString(std::string str)
	{
		std::string result = "";
		for (char const& c : str)
		{
			result += std::bitset<8>(c).to_string();
		}
		return result;
	}

	std::string EncodeBool(bool b)
	{
		if (b)
		{
			return "1";
		}
		else {
			return "0";
		}
	}

	int WriteDataToFile(SimulationData sd, std::vector<std::string> bodies_to_save, std::string path) //Can selectively save certain bodies
	{
		std::string to_write;
		std::ofstream out(path);

		to_write = EncodeDouble(sd.cb_mass) + EncodeDouble(sd.cb_scale) + EncodeVec3(sd.c_pos) + std::bitset<8>((Uint8)bodies_to_save.size()).to_string();
		//64 + 64 + (3*64) + 8
		for (int i = 0; i < bodies_to_save.size(); i++)
		{
			OrbitBodyData data = sd.obc.GetBodyData(bodies_to_save[i]); //Get data via hashing algorithm
			to_write += std::bitset<8>(data.bytes_for_name).to_string()
			+ EncodeString(data.name)
			+ EncodeVec3(data.center)
			+ EncodeDouble(data.mass)
			+ EncodeDouble(data.scale)
			+ EncodeVec3(data.velocity);
		}

		std::cout << "Writing data to file: " << to_write << "\n";

		out << to_write;
		out.close();
		ReadDataFromFile();
		return 0;
	}

	int ReadDataFromFile()
	{
		std::ifstream in("solar_system.orbyte");
		std::string data; //should only be one line :)
		std::getline(in, data);
		std::cout << "Read data from file: " << data << "\n";
		
		return 0;
	}

	int CreateNewFile()
	{
		//Make a new file
		return 0;
	}
};

#endif /*ORBYTE_DATA_H*/
