#pragma once

#include "Material.h"

class SSceneObj
{
public:
	SMaterial Material;

	virtual void Update(double delta) {};

protected:
};