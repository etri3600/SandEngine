#pragma once

#include "Scene/SceneObj.h"
#include "SMath.h"

class SLighting : public SSceneObj
{
public:
	SVector3 Strength;
	SVector3 Direction;
	SVector3 Position;
};