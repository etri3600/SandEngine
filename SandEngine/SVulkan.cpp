#include "SVulkan.h"
#include "SWindows.h"

SVulkan::SVulkan()
{
}

SVulkan::~SVulkan()
{
	Finalize();
}

bool SVulkan::Initialize(const SPlatformSystem* pPlayformSystem, unsigned int screenWidth, unsigned int screenHeight, bool fullScreen, bool vSync)
{
	m_pDevice = new SVulkanDevice();
	m_pDevice->Initialize(screenWidth, screenHeight, fullScreen, vSync);

	return CreateSurface(pPlayformSystem);
}

vk::SurfaceKHR SVulkan::CreateSurface(const SPlatformSystem* pPlayformSystem)
{
#if __WINDOWS__
	const SWindows* pWindowsSystem = static_cast<const SWindows*>(pPlayformSystem);
	vk::Win32SurfaceCreateInfoKHR surfaceInfo = vk::Win32SurfaceCreateInfoKHR()
		.setHinstance(pWindowsSystem->GetInstance())
		.setHwnd(pWindowsSystem->GetWindowHandle());
#endif

	return m_pDevice->GetDevice().createWin32SurfaceKHR(surfaceInfo);
}

void SVulkan::Finalize()
{
	m_pDevice->GetDevice().destroySurfaceKHR(m_surface);
	m_pDevice->Finalize();
}

void SVulkan::Reset()
{
}

bool SVulkan::Update(const double delta, std::map<MaterialType, std::vector<SModel>>& models)
{
	return false;
}

void SVulkan::Draw()
{
}

bool SVulkan::Render()
{
	return false;
}

void SVulkan::Present()
{
}

bool SVulkan::CreateSwapChain(const SPlatformSystem * pPlatformSystem, const int nNumerator, const int nDenominator)
{
	return false;
}

void SVulkan::CreateViewProjection()
{
}

void SVulkan::UpdateBoneTransform(const std::map<MaterialType, std::vector<SModel>>& models)
{
}
