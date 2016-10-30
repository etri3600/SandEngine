#pragma once

#include "SGraphicsInterface.h"
#if __WINDOWS__
#define VK_USE_PLATFORM_WIN32_KHR
#endif
#include "SVulkanDevice.h"

class SVulkan : public SIGraphicsInterface
{
public:
	SVulkan();
	~SVulkan();

	bool Initialize(const SPlatformSystem* pPlayformSystem, unsigned int screenWidth, unsigned int screenHeight, bool fullScreen, bool vSync) override;
	vk::SurfaceKHR CreateSurface(const SPlatformSystem * pPlayformSystem);
	void Finalize() override;

private:
	SVulkanDevice* m_pDevice;
	vk::SurfaceKHR m_surface;
};