#pragma once

enum class EMaterialType
{
	SKINNING,
	TEXTURE,
	SIMPLE,
	LIGHT,
	POSTPROCESS,
	MT_MAX
};

class SMaterial
{
public:
	EMaterialType Type;
};