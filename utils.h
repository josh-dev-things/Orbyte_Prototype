#pragma once

#ifndef UTILS_H
#define UTILS_H

#include "OrbitBody.h"
#include <vector>

//Take two strings and XOR them. String a defines length of output string.
std::string bitwise_string_xor(std::string a, std::string b)
{
	std::stringstream stream;
	for (int i = 0; i < a.length(); i++)
	{
		stream << (a.at(i) ^ b.at(i));
	}

	return stream.str();
}

//Reverses a string using a recursive "pincer" index method, i decrements and j increments.
void reverse_string(std::string& a, int i, int j)
{
	if (i <= j) { return; }
	std::swap(a[j], a[i]);
	reverse_string(a, i - 1, j + 1);
}

#endif /*UTILS_H*/