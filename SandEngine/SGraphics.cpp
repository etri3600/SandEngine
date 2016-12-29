#include "SGraphics.h"
#include "SPlatformManager.h"
#include "SDirectX12.h"

SIGraphicsInterface* gGraphics = nullptr;

GraphicsInterfaceEnum SGraphics::s_eGraphicInterface = GraphicsInterfaceEnum::GI_DX_12;

SIGraphicsInterface* SGraphics::Initialize(const GraphicsInterfaceEnum giInterface)
{
	s_eGraphicInterface = giInterface;
	switch (giInterface)
	{
	case GraphicsInterfaceEnum::GI_DX_12:
		gGraphics = new SDirectX12;
		break;
	}

	return gGraphics;
}

void SGraphics::Fianalize()
{
}
