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
		double no_orbits = bodies_to_save.size();

		std::cout << "Writing data to file: " << path << "\n";

		std::ofstream out(path, std::ios::binary | std::ios::out);;
		if (!out)
		{
			std::cout << "\nERR. Writing to file failed.";
			return -1;
		}
		std::cout << (char*)&sd;
		uint8_t my_size = sizeof(sd);

		out.write((char*)&sd.cb_mass, sizeof(double));
		out.write((char*)&sd.cb_scale, sizeof(double));
		out.write((char*)&sd.c_pos, sizeof(vector3));
		out.write((char*)&no_orbits, sizeof(double));

		for (int i = 0; i < bodies_to_save.size(); i++)
		{
			OrbitBodyData data = sd.obc.GetBodyData(bodies_to_save[i]); //Get data via hashing algorithm

			double bfn = data.bytes_for_name;
			out.write((char*)&bfn, sizeof(double));

			std::string n = data.name;
			for (char n_char : n)
			{
				out.write((char*)&n_char, sizeof(char));
			}

			vector3 c = data.center;
			out.write((char*)&c, sizeof(vector3));

			double m = data.mass;
			out.write((char*)&m, sizeof(double));

			double s = data.scale;
			out.write((char*)&s, sizeof(double));

			vector3 v = data.velocity;
			out.write((char*)&v, sizeof(vector3));
		}
		out.close();

		std::cout << "\nBytes: " << my_size + 1;
		return 0;
	}

	SimulationData ReadDataFromFile(std::string path)
	{
		SimulationData sd;
		double no_orbits;

		std::ifstream in(path, std::ios::binary | std::ios::in);
		if (!in)
		{
			std::cout << "\nERR. Reading from file failed.";
			return sd; //But center mass will be 0;
		}

		in.read((char*)&sd.cb_mass, sizeof(double));
		std::cout << "\nReading mass: " << sd.cb_mass;
		in.read((char*)&sd.cb_scale, sizeof(double));
		std::cout << "\nReading scale: " << sd.cb_scale;
		in.read((char*)&sd.c_pos, sizeof(vector3));
		std::cout << "\nReading c_pos: " << sd.c_pos.Debug();
		in.read((char*)&no_orbits, sizeof(double));

		std::cout << "\nReading No. Orbits: " << no_orbits;

		for (int i = 0; i < no_orbits; i++)
		{
			OrbitBodyData data;

			double bfn;
			in.read((char*)&bfn, sizeof(double));
			data.bytes_for_name = bfn;
			std::cout << "\nReading No. bytes: " << (double)data.bytes_for_name;

			std::string name = "";
			for (int j = 0; j < data.bytes_for_name; j++)
			{
				char c;
				in.read((char*)&c, sizeof(char));
				name += c;
			}
			data.name = name;
			std::cout << "\nReading name: " << name;

			in.read((char*)&data.center, sizeof(vector3));

			in.read((char*)&data.mass, sizeof(double));

			in.read((char*)&data.scale, sizeof(double));

			in.read((char*)&data.velocity, sizeof(vector3));

			sd.obc.AddBodyData(data);
		}

		in.close();


		if (!in.good())
		{
			std::cout << "\n\nOh No.\n\n";
		}
		
		return sd;
	}
};

#endif /*ORBYTE_DATA_H*/
