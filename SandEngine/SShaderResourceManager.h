#pragma once

#include "SModelStruct.h"

class SShaderResourceManager
{
public:
	virtual void CreateShaderResource(const SModel& model) = 0;
	virtual void BindShaderResource() = 0;
};

