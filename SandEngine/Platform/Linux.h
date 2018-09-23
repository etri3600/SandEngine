#pragma once

#include "PlatformSystem.h"

#if __LINUX__

class SLinux : public SPlatformSystem
{
public:
	bool Init() override;
	bool Tick() override;
};
#endif