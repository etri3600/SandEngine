#pragma once

#include "SPlatformSystem.h"
#include "SUtils.h"
#include "SModelLoader.h"
#include "SConstantBuffer.h"

enum class GraphicsInterfaceEnum
{
	//GI_DX_11,
	GI_DX_12,
	//GI_GL_4,
	GI_VULKAN
};
class SIGraphicsInterface
{
public:
	virtual bool Initialize(const SPlatformSystem* pPlatformSystem, unsigned int screenWidth, unsigned int screenHeight, bool fullScreen, bool vSync) = 0;
	virtual void Finalize() = 0;

	virtual void Reset() = 0;
	virtual bool Update(const double delta, std::vector<SModel>& models) = 0;
	virtual void Draw() = 0;
	virtual bool Render() = 0;
	virtual void Present() = 0;

	virtual bool CreateSwapChain(const SPlatformSystem* pPlatformSystem, const int nNumerator, const int nDenominator) = 0;
	virtual void CreateViewProjection() = 0;

	virtual void UpdateBoneTransform(const std::vector<SModel>& models) = 0;

public:
	static constexpr unsigned int c_BufferingCount = 3;

protected:
	SModelLoader* ModelLoader = nullptr;
	char* MappedConstantBuffer[2];
	SMatrix View, Projection;

	unsigned int m_BufferIndex = 0;

	unsigned int m_ScreenWidth = 0, m_ScreenHeight = 0;
};