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
	double cb_mass = 0;
	double cb_scale = 0;
	OrbitBodyCollection obc;
	vector3 c_pos;
};

union ulldouble
{
	double d;
	unsigned long long ull;
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
		ulldouble ulld;
		ulld.d = db;
		std::bitset<64>b(ulld.ull);
		std::string result = b.to_string();

		//testing
		std::cout << "\n" << db << " (Has size: " << sizeof(db) << "bytes )" << " => Encode => " + result + " => Decode => " << DecodeDouble(result);

		return result;
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

	vector3 DecodeVec3(std::string to_decode)
	{
		std::string x = to_decode.substr(0, 64);
		std::string y = to_decode.substr(64, 64);
		std::string z = to_decode.substr(128, 64);
		vector3 out = { DecodeDouble(x), DecodeDouble(y), DecodeDouble(z) };
		return out;
	}

	double DecodeDouble(std::string to_decode)
	{
		//64 bits long
		if (to_decode.length() != 64)
		{
			std::cout << "\n\nERROR: SEE DECODE DOUBLE: " << to_decode.length() << " bits recieved!";
			return -1;
		}
		std::bitset<64> out;
		std::istringstream bit_stream(to_decode);
		bit_stream >> out;
		ulldouble ulld;
		ulld.ull = out.to_ullong();
		return ulld.d;
	}

	std::string DecodeString(std::string to_decode)
	{
		int no_chars = to_decode.length() / 8;
		std::string out;
		for (int i = 0; i < no_chars; i++)
		{
			int start = i * 8; //8 bits given to each char
			std::string this_char = to_decode.substr(start, 8);
			std::bitset<8> bits;
			std::istringstream bit_stream(this_char);
			bit_stream >> bits;
			
			unsigned long character_value = bits.to_ulong();
			unsigned char character = static_cast<unsigned char>(character_value);
			out += character;
		}
		return out;
	}

	int WriteDataToFile(SimulationData sd, std::vector<std::string> bodies_to_save, std::string path) //Can selectively save certain bodies
	{
		std::string to_write;
		std::ofstream out(path);

		to_write = EncodeDouble(sd.cb_mass) + EncodeDouble(sd.cb_scale) + EncodeVec3(sd.c_pos) + EncodeDouble(bodies_to_save.size());
		//64 + 64 + (3*64) + 8
		for (int i = 0; i < bodies_to_save.size(); i++)
		{
			OrbitBodyData data = sd.obc.GetBodyData(bodies_to_save[i]); //Get data via hashing algorithm
			to_write += EncodeDouble(data.bytes_for_name)
			+ EncodeString(data.name)
			+ EncodeVec3(data.center)
			+ EncodeDouble(data.mass)
			+ EncodeDouble(data.scale)
			+ EncodeVec3(data.velocity);
		}

		std::cout << "Writing data to file: " << to_write << "\n";

		out << to_write;
		out.close();
		ReadDataFromFile(path);
		return 0;
	}

	int ReadDataFromFile(std::string path)
	{
		std::ifstream in(path);
		std::string data; //should only be one line
		std::getline(in, data);
		std::cout << "Read data from file: " << data << "\n";
		//cbmass, cbscale, cpos, no_bodies => (bytes for name, name, center, mass, scale, velocity)
		SimulationData sd;
		int i = 0;

		sd.cb_mass = DecodeDouble(data.substr(i, 64));
		std::cout << "\nMass of centre body: "<< sd.cb_mass;
		i += 64;

		sd.cb_scale = DecodeDouble(data.substr(i, 64));
		std::cout << "\nScale of centre body: " << sd.cb_scale;
		i += 64;

		sd.c_pos = DecodeVec3(data.substr(128, 64 * 3));
		std::cout << "\nPosition of camera: " << sd.c_pos.Debug();
		i += 64 * 3;

		int no_orbits = (int)DecodeDouble(data.substr(i, 64));
		std::cout << "\nNumber of orbits: " << no_orbits;
		i += 64;

		OrbitBodyCollection obc;

		for (int j = 0; j < no_orbits; j++)
		{
			int no_bytes_for_name = DecodeDouble(data.substr(i, 64));
			i += 64;
			std::cout << "\nNumber of bytes for name: " << no_bytes_for_name;

			std::string name = DecodeString(data.substr(i, 8 * no_bytes_for_name));
			i += 8 * no_bytes_for_name;
			std::cout << "\nOrbit Name: " << name;

			vector3 center = DecodeVec3(data.substr(i, 64 * 3));
			i += 64 * 3;

			double mass = DecodeDouble(data.substr(i, 64));
			i += 64;

			double scale = DecodeDouble(data.substr(i, 64));
			i += 64;

			vector3 velocity = DecodeVec3(data.substr(i, 64 * 3));
			i += 64 * 3;
			OrbitBodyData obd(name, center, mass, scale, velocity);
			obc.AddBodyData(obd);
		}
		sd.obc = obc;
		
		return 0;
	}
};

#endif /*ORBYTE_DATA_H*/
