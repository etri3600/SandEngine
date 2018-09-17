#pragma once

#include "GraphicsInterface.h"
#include "GraphicsFormat.h"

class SGraphics
{
public:
	static IGraphicsInterface* Initialize(const EGraphicsInterfaceEnum giInterface);
	static void Fianalize();

	static EGraphicsInterfaceEnum s_eGraphicInterface;
};
