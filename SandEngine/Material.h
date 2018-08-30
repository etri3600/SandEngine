#pragma once

enum class EMaterialType
{
	SKINNING,
	TEXTURE,
	SIMPLE,
	DEFERRED,
	POSTPROCESS,
	MT_MAX
};

class SMaterial
{
public:
	EMaterialType Type;
};