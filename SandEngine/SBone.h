#pragma once

#include <string>
#include "SMath.h"

struct SBoneNode
{
	std::string name;
	SMatrix localTransformation, globalTransformation;
	SBoneNode* parent;
	std::vector<SBoneNode*> children;
};

struct SBone
{
	std::string name;
	SMatrix boneOffset;
	SMatrix localTransform, globalTransform;
};