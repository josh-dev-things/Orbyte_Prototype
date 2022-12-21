#ifndef VEC3_H
#define VEC3_H
struct vector3
{
	double x, y, z;
	
	vector3 operator*(double right)
	{
		vector3 result = { x * right, y * right, z * right };
		return result;
	}

	vector3 operator+(double right)
	{
		vector3 result = { x + right, y + right, z + right };
		return result;
	}

	vector3 operator-(double right)
	{
		vector3 result = { x - right, y - right, z - right };
		return result;
	}

	vector3 operator+(vector3 v)
	{
		vector3 result = { x + v.x, y + v.y, z + v.z };
		return result;
	}

	vector3 operator-(vector3 v)
	{
		vector3 result = { x - v.x, y - v.y, z - v.z };
		return result;
	}

	vector3 operator*(vector3 v)
	{
		vector3 result = { x * v.x, y * v.y, z * v.z };
		return result;
	}

	vector3* operator=(const vector3& v)
	{
		x = v.x;
		y = v.y;
		z = v.z;
		return this;
	}

	std::string Debug()
	{
		return std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z);
	}
	
};

double Magnitude(vector3 vec)
{
	return sqrtf((vec.x * vec.x) + (vec.y * vec.y) + (vec.z * vec.z));
}

double Distance(vector3 vecFrom, vector3 vecTo)
{
	vector3 a = vecTo - vecFrom; //Thank you past me for making those operator overloads 
	double distance = Magnitude(a);
	
	return distance;
}

vector3 Normalize(vector3 vec)
{
	float mag = Magnitude(vec);
	vector3 norm = {
		vec.x / mag,
		vec.y / mag,
		vec.z / mag
	};
	return norm;
}

struct edge
{
	int a, b;
};

#endif /*VEC3_H*/