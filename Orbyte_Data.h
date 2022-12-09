#pragma once
#ifndef ORBYTE_DATA_H
#define ORBYTE_DATA_H

#include<iostream>
#include<fstream>
#include "vec3.h"

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
	OrbitBodyData(std::string _name, vector3 _center, float _scale, vector3 _velocity, bool _override_velocity)
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

//So this header file is going to contain the data controller. The big bad wolf in charge of all the data being read into and written out of the application.
class DataController
{
public: 

	int WriteDataToFile(OrbitBodyData data)
	{
		std::ofstream wf;
		wf.open("solar_system.orbyte", std::ios::out | std::ios::binary);
		if (!wf)
		{
			std::cout << "Could not open .orbyte file to write data.";
			return 1;
		}

		//Start writing
		std::cout << "Writing data to file: " << &data.bytes_for_name << "\n";
		uint8_t bfn = data.bytes_for_name;
		wf.write((char *)&bfn, sizeof(bfn));
		wf.close();

		return 0;
	}

	int CreateNewFile()
	{
		//Make a new file
		return 0;
	}
};

#endif /*ORBYTE_DATA_H*/
