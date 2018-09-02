#pragma once

#include "PlatformSystem.h"
#include "Utils.h"
#include "ModelLoader.h"
#include "ConstantBuffer.h"
#include "Lighting.h"

enum class EGraphicsInterfaceEnum : unsigned char
{
	DX12,
	//GL4,
	VULKAN
};

class IGraphicsInterface
{
public:
	virtual ~IGraphicsInterface() {}

	virtual bool Initialize(const SPlatformSystem* pPlatformSystem, unsigned int screenWidth, unsigned int screenHeight, bool fullScreen, bool vSync) = 0;
	virtual void Finalize() = 0;

	virtual void Reset() = 0;
	virtual bool Update(const double delta, std::map<EMaterialType, std::vector<SSceneObj*>>& objs) = 0;
	virtual void Draw() = 0;
	virtual bool Render() = 0;
	virtual void Present() = 0;

	virtual bool CreateSwapChain(const SPlatformSystem* pPlatformSystem, const int nNumerator, const int nDenominator) = 0;
	virtual void CreateViewProjection() = 0;

	virtual void UpdateBoneTransform(const std::map<EMaterialType, std::vector<SSceneObj*>>& objs) = 0;

public:
	static constexpr unsigned int c_BufferingCount = 3;

protected:
	SModelLoader* ModelLoader = nullptr;
	SMatrix View, Projection;

	unsigned int m_BufferIndex = 0;

	unsigned int m_ScreenWidth = 0, m_ScreenHeight = 0;
};
