#pragma once

#include "SShaderResourceManager.h"

class SDX12ShaderResourceManager : public SShaderResourceManager
{
public:
	void CreateShaderResource(const SModel& model) override;
	void BindShaderResource() override;
};

