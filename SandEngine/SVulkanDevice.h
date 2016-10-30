#pragma once

#include <vulkan/vulkan.hpp>

class SVulkanDevice
{
public:
	bool Initialize(unsigned int screenWidth, unsigned int screenHeight, bool fullScreen, bool vSync);
	void Finalize();

	vk::Instance GetDevice() { return m_instance; }

private:
	std::vector<const char*> GetAvailableWSIExtensions();

private:
	vk::Instance m_instance;
};