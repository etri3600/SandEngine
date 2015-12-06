#include "SGraphics.h"
#include "SPlatformManager.h"
#include "SDirectX12.h"

GraphicsInterfaceEnum SGraphics::s_eGraphicInterface = GraphicsInterfaceEnum::GI_DX_12;

SIGraphicsInterface* SGraphics::Initialize(const GraphicsInterfaceEnum giInterface)
{
	s_eGraphicInterface = giInterface;
	switch (giInterface)
	{
	case GraphicsInterfaceEnum::GI_DX_12:
		return new SDirectX12;
		break;
	}

	return nullptr;
}

void SGraphics::Fianalize()
{
}
