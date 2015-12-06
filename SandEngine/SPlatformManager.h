#pragma once

#include "SWindows.h"
#include "SLinux.h"

class SPlatformManager
{
public:
	static SPlatformSystem* CreateFramework() 
	{ 
#if __WINDOWS__
		return new SWindows;
#elif __LINUX__
		return new SLinux;
#endif
	};
};