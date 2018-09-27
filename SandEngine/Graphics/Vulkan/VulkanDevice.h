#pragma once

#include "../GraphicsInterface.h"
#if __WINDOWS__
#define VK_USE_PLATFORM_WIN32_KHR
#elif __LINUX__
#define VK_USE_PLATFORM_XCB_KHR
#endif
#include <vulkan/vulkan.hpp>


class SVulkanDevice
{
public:
	~SVulkanDevice();

	bool Initialize(unsigned int screenWidth, unsigned int screenHeight, bool fullScreen, bool vSync);
	void Finalize();

	vk::Instance GetDevice() { return m_instance; }

private:
	std::vector<const char*> GetAvailableWSIExtensions();

private:
	vk::Instance m_instance;
};
