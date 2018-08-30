#pragma once

#include "SceneObj.h"
#include "Math.h"
#include "GraphicsInterface.h"

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

struct SViewProjection
{
	SMatrix View;
	SMatrix Projection;
	SMatrix InvView;
	SMatrix InvProjection;
};

struct SLight
{
	SVector3 Strength;
	SVector3 Direction;
	SVector3 Position;
};

#define MAX_LIGHTS 16