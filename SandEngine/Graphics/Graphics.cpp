#include "Graphics.h"
#include "Platform/PlatformManager.h"
#include "DirectX12/DirectX12.h"
#include "Vulkan/Vulkan.h"

IGraphicsInterface* gGraphics = nullptr;

EGraphicsInterfaceEnum SGraphics::s_eGraphicInterface = EGraphicsInterfaceEnum::DX12;

IGraphicsInterface* SGraphics::Initialize(const EGraphicsInterfaceEnum giInterface)
{
	s_eGraphicInterface = giInterface;
#if __WINDOWS__
    return new SDirectX12;
#else
    return new SVulkan;
#endif
}

void SGraphics::Fianalize()
{
	if (gGraphics)
	{
		delete gGraphics;
	}
}
