#pragma once

enum class EMaterialType
{
	SKINNING,
	TEXTURE,
	SIMPLE,
	POSTPROCESS,
	LIGHT,
	MT_MAX
};

class SMaterial
{
public:
	EMaterialType Type;
};