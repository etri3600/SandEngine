#pragma once

#include "PlatformSystem.h"

#if __LINUX__

#include <xcb/xcb.h>

class SLinux : public SPlatformSystem
{
public:
	bool Init() override;
	bool Tick() override;

	xcb_connection_t* GetConnection() const { return connectoin; }
	xcb_window_t GetWindow() const { return window; }

protected:
	xcb_connection_t* connection;
	xcb_window_t window;
	xcb_screen_t * screen;
	xcb_atom_t wmProtocols;
	xcb_atom_t wmDeleteWin;
};
#endif