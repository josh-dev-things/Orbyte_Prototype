#pragma once

#ifndef UTILS_H
#define UTILS_H

#include "OrbitBody.h"
#include <vector>

std::vector<Body> MergeOrbitBodies(std::vector<Body> left, std::vector<Body> right)
{
	std::vector<Body> Merged;

	int iLeft = 0; // left current position
	int iRight = 0; // right current position
	int iMerged = 0; // merged current position

	while (iLeft < left.size() && iRight < right.size())
	{
		if (Magnitude(left[iLeft].Get_Position()) < Magnitude(right[iRight].Get_Position()))
		{
		//https://github.com/isaaccomputerscience/isaac-code-samples/blob/main/sorting-algorithms/merge-sort/c-sharp/merge_sort.cs
		}
	}



}

void Sort_Orbit_Bodies()
{
	
}

#endif /*UTILS_H*/