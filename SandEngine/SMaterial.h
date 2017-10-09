#pragma once

enum class EMaterialType
{
	SKINNING,
	TEXTURE,
	SIMPLE,
	GBUFFER,
};

class SMaterial
{
public:
	EMaterialType Type;
};