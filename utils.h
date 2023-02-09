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

//std::vector<Body> MergeOrbitBodies(std::vector<Body>& left, std::vector<Body>& right)
//{
//	std::vector<std::string> Merged(left.size() + right.size());
//
//	int iLeft = 0; // left index
//	int iRight = 0; // right index
//	int iMerged = 0; // merged index
//
//	while (iLeft < left.size() && iRight < right.size())
//	{
//		if (Magnitude(left[iLeft].Get_Position()) < Magnitude(right[iRight].Get_Position()))
//		{
//		//https://github.com/isaaccomputerscience/isaac-code-samples/blob/main/sorting-algorithms/merge-sort/c-sharp/merge_sort.cs
//			Merged[iMerged] = left[iLeft].name;
//			iLeft++;
//			iMerged++;
//		}
//		else {
//			Merged[iMerged] = right[iRight].name;
//			iRight++;
//			iMerged++;
//		}
//	}
//
//	//Clean up any data not included
//
//	while (iLeft < left.size())
//
//}
//
//void Sort_Orbit_Bodies()
//{
//	
//}

#endif /*UTILS_H*/