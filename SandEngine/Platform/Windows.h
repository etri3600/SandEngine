#pragma once

#include "PlatformSystem.h"

#if __WINDOWS__

#include <SDKDDKVer.h>
#include <windows.h>

class SWindows : public SPlatformSystem
{
public:
	bool Init() override;
	bool Tick() override;

	HINSTANCE GetInstance() const { return mhInstance; }
	HWND GetWindowHandle() const { return mhWnd; }

	static void OutputErrorMessage(HRESULT hr);

private:
	HINSTANCE mhInstance;
	HWND mhWnd;
};

#endif // __WINDOWS__