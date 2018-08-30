#include "Graphics.h"
#include "PlatformManager.h"
#include "DirectX12.h"
#include "Vulkan.h"

SIGraphicsInterface* gGraphics = nullptr;

EGraphicsInterfaceEnum SGraphics::s_eGraphicInterface = EGraphicsInterfaceEnum::DX12;

SIGraphicsInterface* SGraphics::Initialize(const EGraphicsInterfaceEnum giInterface)
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
