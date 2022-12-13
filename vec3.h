#ifndef VEC3_H
#define VEC3_H
struct vector3
{
	float x, y, z;
	
	public: vector3 operator*(float right)
	{
		vector3 result = { x * right, y * right, z * right };
		return result;
	}

	public: vector3 operator+(float right)
	{
		vector3 result = { x + right, y + right, z + right };
		return result;
	}

	public: vector3 operator-(float right)
	{
		vector3 result = { x - right, y - right, z - right };
		return result;
	}

	public: vector3 operator+(vector3 v)
	{
		vector3 result = { x + v.x, y + v.y, z + v.z };
		return result;
	}

	public: vector3 operator-(vector3 v)
	{
		vector3 result = { x - v.x, y - v.y, z - v.z };
		return result;
	}

	public: vector3 operator*(vector3 v)
	{
		vector3 result = { x * v.x, y * v.y, z * v.z };
		return result;
	}

	std::string Debug()
	{
		return std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z);
	}
	
};

float Magnitude(vector3 vec)
{
	return sqrtf((vec.x * vec.x) + (vec.y * vec.y) + (vec.z * vec.z));
}

vector3 Normalize(vector3 vec)
{
	float mag = Magnitude(vec);
	vector3 norm = {
		vec.x / mag,
		vec.y / mag,
		vec.z / mag // Problem here?
	};
	return norm;
}

struct edge
{
	int a, b;
};

#endif /*VEC3_H*/