#ifndef VEC3_H
#define VEC3_H
struct vector3
{
	float x, y, z;
	vector3 normalize() {
		return { x, y, z };
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
		vec.z / mag
	};
	return norm;
}

struct edge
{
	int a, b;
};

#endif /*VEC3_H*/