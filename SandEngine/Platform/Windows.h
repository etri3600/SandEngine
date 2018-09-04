#pragma once

#include <SDKDDKVer.h>
#include <windows.h>

#include "PlatformSystem.h"

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