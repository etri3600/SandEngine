#pragma once

#include "SGraphicsInterface.h"

class SGraphics
{
public:
	static SIGraphicsInterface* Initialize(const GraphicsInterfaceEnum giInterface);
	static void Fianalize();

	static GraphicsInterfaceEnum s_eGraphicInterface;
};