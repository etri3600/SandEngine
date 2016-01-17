#pragma once

#include <map>

struct SBone
{
	std::map<unsigned int, float> BoneWeightMap;
	SMatrix boneMatrix;
};