#pragma once

#include "SVulkanDevice.h"

class SVulkan : public SIGraphicsInterface
{
public:
	SVulkan();
	~SVulkan();

	// Inherited via SIGraphicsInterface
	bool Initialize(const SPlatformSystem* pPlayformSystem, unsigned int screenWidth, unsigned int screenHeight, bool fullScreen, bool vSync) override;
	void Finalize() override;

	virtual bool Update(const double delta) override;
	virtual void Draw(std::vector<SModel>& models) override;
	virtual bool Render() override;
	virtual void Present() override;
	virtual bool CreateSwapChain(const SPlatformSystem * pPlatformSystem, const int nNumerator, const int nDenominator) override;
	virtual void CreateViewProjection() override;
	virtual void UpdateBoneTransform(const std::vector<SModel>& models) override;

	vk::SurfaceKHR CreateSurface(const SPlatformSystem * pPlayformSystem);

private:
	SVulkanDevice* m_pDevice;
	vk::SurfaceKHR m_surface;
};