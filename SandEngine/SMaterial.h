#pragma once

enum class EMaterialType
{
	SKINNING,
	TEXTURE,
	SIMPLE,
	LIGHT,
	MT_MAX
};

class SMaterial
{
public:
	EMaterialType Type;
};