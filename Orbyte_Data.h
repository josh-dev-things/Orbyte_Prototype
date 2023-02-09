#pragma once
#ifndef ORBYTE_DATA_H
#define ORBYTE_DATA_H

#include<iostream>
#include<fstream>
#include "vec3.h"
#include <bitset>
#include "utils.h"

struct OrbitBodyData
{
	//This should not be in this header file you dipshit
	//body(std::string _name, float center_x, float center_y, float center_z, float _scale, vector3 _velocity, vector3 _god_pos, bool override_velocity = true)
	std::string name;
	vector3 center;
	float scale;
	vector3 velocity;
	bool override_velocity; //tbf if this is true it wont matter.

	//Information for storage
	uint8_t bytes_for_name; //So the first 8 bits of the file will tell us how many bytes the name contains. Name being the only var with "unlimited length"
	OrbitBodyData(std::string _name = "", vector3 _center = { 0, 0, 0 }, float _scale = 1, vector3 _velocity = {0, 0, 0}, bool _override_velocity = false)
	{
		name = _name;
		center = _center;
		scale = _scale;
		velocity = _velocity;
		override_velocity = _override_velocity;

		//Number of chars in name = number of bytes => 8 x number of chars = number of bits for name
		//FLOAT is 32 BITS
		//SO VECTOR3 is 3 x 32 BITS

		// 8 x name.length + 3x32 + 32 + 3x32 + 1 = number of bits in file
		bytes_for_name = (uint8_t)name.length();
	}

};

struct OrbitBodyCollection //A Hash table of orbit objects
{
	std::vector<OrbitBodyData> data = std::vector<OrbitBodyData>(101); //101 has been chosen as it is prime and not too close to a power of two

public:
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

	void AddBodyData(OrbitBodyData new_data)
	{
		data[Hash(new_data.name)] = new_data;
		std::cout << "\n TESTING HASH: " <<  Hash(new_data.name) << "\n";
	}


};

struct SimulationData
{
	std::vector<OrbitBodyData> OrbitBodies;
	SimulationData(std::vector<OrbitBodyData> _bodies)
	{
		OrbitBodies = _bodies;
	}
};

//So this header file is going to contain the data controller. The big bad wolf in charge of all the data being read and written out of the application.
class DataController
{
public: 

	std::string EncodeVec3(vector3 vec)
	{
		return EncodeFloat(vec.x) + EncodeFloat(vec.y) + EncodeFloat(vec.z);
	}

	std::string EncodeFloat(float fl)
	{
		return std::bitset<32>(fl).to_string();
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

	int WriteDataToFile(OrbitBodyData data)
	{
		std::string to_write;
		std::ofstream out("solar_system.orbyte");

		//Start writing
		to_write += std::bitset<8>(data.bytes_for_name).to_string()
			+ EncodeString(data.name)
			+ EncodeVec3(data.center)
			+ EncodeFloat(data.scale)
			+ EncodeVec3(data.velocity)
			+ EncodeBool(data.override_velocity);

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
		//FIRST 8 BITS ALLOCATED TO LENGTH OF NAME
		return 0;
	}

	int CreateNewFile()
	{
		//Make a new file
		return 0;
	}
};

#endif /*ORBYTE_DATA_H*/
