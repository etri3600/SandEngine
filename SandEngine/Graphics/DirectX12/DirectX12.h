#pragma once

#include <ostream>

#include "../GraphicsInterface.h"

#if __WINDOWS__
#include "DirectX12Device.h"

struct SObjectProxy
{
	unsigned int CBVDescriptorSize = 0;
	unsigned int RTVDescriptorSize = 0;
	unsigned int BaseVertexLocation = 0;
	unsigned int VertexSize = 0;
	unsigned int StartIndexLocation = 0;
	unsigned int IndexSize = 0;
	std::vector<SMeshInfo> MeshProxy;
	std::vector<SSkinnedModelVertex> Vertices;
	std::vector<unsigned int> Indices;
	SMatrix Tranformation;
	SMatrix BoneTransform[MAX_BONES];
	std::vector<STexture*> Textures;
};

struct SBatchProxy
{
	unsigned int VertexSize = 0;
	unsigned int IndexSize = 0;
	std::vector<SObjectProxy> ObjectProxies;
};

struct SSceneProxy
{
	std::map<EMaterialType, SBatchProxy> BatchProxies;
};

enum class EGBuffer : unsigned short
{
	GB_COLOR = 0,
	GB_DEPTH,
	GB_NORMAL,
	GB_NUM
};

std::wostream& operator<<(std::wostream& stream, EGBuffer buffer);

class SDX12DescriptorHeap;
class SDX12Pipeline;
class SDX12RootSignature;
class SDirectX12 : public IGraphicsInterface
{
public:
	SDirectX12();
	~SDirectX12();

	bool Initialize(const SPlatformSystem* pPlayformSystem, unsigned int screenWidth, unsigned int screenHeight, bool fullScreen, bool vSync) override;
	void Finalize() override;

	bool CreateSwapChain(const SPlatformSystem* pPlatformSystem, const int nNumerator, const int nDenominator) override;
	void CreateViewProjection() override;

	void UpdateBoneTransform(const std::map<EMaterialType, std::vector<SSceneObj*>>& objs) override;

	void Reset() override;
	bool Update(const double delta, std::map<EMaterialType, std::vector<SSceneObj*>>& objs) override;
	void Draw() override;
	bool Render() override;
	void Present() override;

private:
	void WaitForGPU(unsigned long long fenceValue);
	void WaitForIdle();
	void InitBundle();

	ID3D12CommandAllocator* GetCommandAllocator() const { return m_pCommandAllocator[m_BufferIndex]; }
	ID3D12Resource* GetRenderTarget() const { return m_pBackBufferRenterTarget[m_BufferIndex]; }

private:
	SDirectX12Device* m_pDevice;

	ID3D12CommandQueue* m_pCommandQueue = nullptr;
	ID3D12CommandAllocator* m_pCommandAllocator[c_BufferingCount];
	ID3D12GraphicsCommandList* m_pCommandList = nullptr;

	ID3D12CommandAllocator* m_pBundleAllocator;
	ID3D12GraphicsCommandList* m_pBundleList;

	ID3D12Fence* m_pFence = nullptr;
	unsigned __int64 m_nFenceValue[c_BufferingCount];
	IDXGISwapChain4* m_pSwapChain = nullptr;
	unsigned int m_RenderTargetViewDescriptorSize = 0;
	D3D12_VIEWPORT m_Viewport;

	IRendering* Rendering;

	ID3D12DescriptorHeap* m_pRenderTargetViewHeap = nullptr;
	ID3D12Resource* m_pBackBufferRenterTarget[c_BufferingCount];

	ID3D12Resource* m_pVertexBuffer = nullptr;
	std::vector<byte> m_vertexShader;

	std::vector<byte> m_pixelShader;

	ID3D12Resource* m_pIndexBuffer = nullptr;

	ID3D12DescriptorHeap* m_pPostProcessRTVHeap = nullptr;
	ID3D12DescriptorHeap* m_pPostProcessCbvSrvHeap = nullptr;

	SSceneProxy m_SceneProxy;

	HANDLE m_hFenceEvent;
	bool m_bVSync;
	bool m_bFullScreen;

private:
	static constexpr unsigned int c_NumDescriptorsPerHeap = 64;
};
#endif