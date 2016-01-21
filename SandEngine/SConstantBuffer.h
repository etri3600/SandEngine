#pragma once

#include "SMath.h"
#include "SGraphicsInterface.h"

struct SModelViewProjection
{
	SMatrix Model;
	SMatrix View;
	SMatrix Projection;
	SMatrix NormalMatrix;
};

#define MAX_BONES 128

struct SBoneTransform
{
	SMatrix transform[MAX_BONES];
};