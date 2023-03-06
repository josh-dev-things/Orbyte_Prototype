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

	double operator*(vector3 v)
	{
		double result = (x*v.x) + (y*v.y) + (z*v.z);
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

// Length of a vector3. Thank you Pythagoras.
double Magnitude(vector3 vec)
{
	return sqrt((vec.x * vec.x) + (vec.y * vec.y) + (vec.z * vec.z));
}

// Distance between two points in 3D space
double Distance(vector3 vecFrom, vector3 vecTo)
{
	vector3 a = vecTo - vecFrom;
	double distance = Magnitude(a);
	
	return distance;
}

// "Dot Product" of two vectors. 
double Scalar_Product(vector3 a, vector3 b)
{
	return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
}

// Vector3 in same direction but with magnitude 1
vector3 Normalize(vector3 vec)
{
	double mag = Magnitude(vec);
	vector3 norm = {
		vec.x / mag,
		vec.y / mag,
		vec.z / mag
	};
	return norm;
}

#endif /*VEC3_H*/