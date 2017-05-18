#pragma once

enum class MaterialType
{
	SKINNING,
	TEXTURE,
	SIMPLE
};

class SMaterial
{
public:
	MaterialType Type;
};