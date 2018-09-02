#pragma once

enum class EMaterialType
{
	SKINNING,
	TEXTURE,
	DEFERRED,
	POSTPROCESS,
	MT_MAX
};

class SMaterial
{
public:
	EMaterialType Type;
};