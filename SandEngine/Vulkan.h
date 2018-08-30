#pragma once

#include "VulkanDevice.h"

class SVulkan : public SIGraphicsInterface
{
public:
	SVulkan();
	~SVulkan();

	// Inherited via SIGraphicsInterface
	bool Initialize(const SPlatformSystem* pPlayformSystem, unsigned int screenWidth, unsigned int screenHeight, bool fullScreen, bool vSync) override;
	void Finalize() override;

	virtual void Reset() override;
	virtual bool Update(const double delta, std::map<EMaterialType, std::vector<SSceneObj*>>& objs) override;
	virtual void Draw() override;
	virtual bool Render() override;
	virtual void Present() override;
	virtual bool CreateSwapChain(const SPlatformSystem * pPlatformSystem, const int nNumerator, const int nDenominator) override;
	virtual void CreateViewProjection() override;
	virtual void UpdateBoneTransform(const std::map<EMaterialType,std::vector<SSceneObj*>>& objs) override;

	vk::SurfaceKHR CreateSurface(const SPlatformSystem * pPlayformSystem);

private:
	SVulkanDevice* m_pDevice;
	vk::SurfaceKHR m_surface;

};