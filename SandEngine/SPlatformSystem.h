#pragma once

#if __WINDOWS__ || _WIN32
#define __WINDOWS__ 1
#elif __linux__
#define __LINUX__ 1
#endif

class SPlatformSystem
{
public:
	virtual bool Init() = 0;
	virtual bool Tick() = 0;
};