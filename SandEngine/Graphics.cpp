#include "Graphics.h"
#include "PlatformManager.h"
#include "DirectX12.h"
#include "Vulkan.h"

IGraphicsInterface* gGraphics = nullptr;

EGraphicsInterfaceEnum SGraphics::s_eGraphicInterface = EGraphicsInterfaceEnum::DX12;

IGraphicsInterface* SGraphics::Initialize(const EGraphicsInterfaceEnum giInterface)
{
	s_eGraphicInterface = giInterface;
	switch (giInterface)
	{
	case EGraphicsInterfaceEnum::DX12:
		gGraphics = new SDirectX12;
		break;
	case EGraphicsInterfaceEnum::VULKAN:
		gGraphics = new SVulkan;
		break;
	}

	return gGraphics;
}

void SGraphics::Fianalize()
{
	if (gGraphics)
	{
		delete gGraphics;
	}
}
