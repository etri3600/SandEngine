#pragma once

#include "Windows.h"
#include "Linux.h"

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