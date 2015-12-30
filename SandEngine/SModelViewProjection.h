#pragma once

#include "SMath.h"

struct SModelViewProjection
{
	SMatrix Model;
	SMatrix View;
	SMatrix Projection;
	float padding[16];
};