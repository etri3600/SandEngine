#pragma once

#include "SMath.h"
#include "SGraphicsInterface.h"

struct SModelViewProjection
{
	SMatrix Model;
	SMatrix View;
	SMatrix Projection;
	SMatrix invetTransposeModel;
};

#define MAX_BONES 96

struct SBoneTransform
{
	SMatrix transform[MAX_BONES];
};