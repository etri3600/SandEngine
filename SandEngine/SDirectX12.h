#pragma once

#include "SGraphicsInterface.h"
#include "SDirectX12Device.h"

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

enum class EGBuffer : unsigned int
{
	GB_POSITION = 0,
	GB_NORMAL,
	GB_NUM
};

class SDX12ResourceAllocator;
class SDX12DescriptorHeapAllocator;
class SDX12Pipeline;
class SDX12RootSignature;
class SDirectX12 : public SIGraphicsInterface
{
public:
	SDirectX12();
	~SDirectX12();

	bool Initialize(const SPlatformSystem* pPlayformSystem, unsigned int screenWidth, unsigned int screenHeight, bool fullScreen, bool vSync) override;
	void Finalize() override;

	bool CreateSwapChain(const SPlatformSystem* pPlatformSystem, const int nNumerator, const int nDenominator) override;
	void CreateViewProjection() override;

	void UpdateBoneTransform(const std::map<EMaterialType, std::vector<SModel>>& models) override;

	void Reset() override;
	bool Update(const double delta, std::map<EMaterialType, std::vector<SModel>>& models) override;
	void Draw() override;
	bool Render() override;
	void Present() override;

protected:
	unsigned int CreateShaderResources(SDX12Pipeline* pipeline, SBatchProxy batchProxy, unsigned int offset);
	void CreateGBuffers();

	void UpdateConstantBuffer(SDX12Pipeline* pipeline, SBatchProxy batchProxy, unsigned int objIndex);
	void BindShaderResource(unsigned int sceneIndex, unsigned int meshIndex);

private:
	void WaitForGPU(unsigned long long fenceValue);
	void WaitForIdle();
	void InitBundle();

	ID3D12CommandAllocator* GetCommandAllocator() const { return m_pCommandAllocator[m_BufferIndex]; }
	ID3D12Resource* GetRenderTarget() const { return m_pBackBufferRenterTarget[m_BufferIndex]; }

private:
	SDirectX12Device* m_pDevice;
	SDX12ResourceAllocator* m_pResourceAllocator[2];
	SDX12DescriptorHeapAllocator* m_pDescriptorAllocator[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];
	std::map<EMaterialType, SDX12Pipeline*> m_pipelines;
	std::map<EMaterialType, SDX12RootSignature*> m_RootSignatures;

	ID3D12CommandQueue* m_pCommandQueue;
	ID3D12CommandAllocator* m_pCommandAllocator[c_BufferingCount];
	ID3D12GraphicsCommandList* m_pCommandList;

	ID3D12CommandAllocator* m_pBundleAllocator;
	ID3D12GraphicsCommandList* m_pBundleList;

	//ID3D12PipelineState* m_pPipelineState;
	ID3D12Fence* m_pFence;
	unsigned __int64 m_nFenceValue[c_BufferingCount];
	IDXGISwapChain4* m_pSwapChain;
	unsigned int m_RenderTargetViewDescriptorSize = 0;
	D3D12_VIEWPORT m_Viewport;

	ID3D12DescriptorHeap* m_pRenderTargetViewHeap;
	ID3D12Resource* m_pBackBufferRenterTarget[c_BufferingCount];

	ID3D12Resource* m_pVertexBuffer;
	std::vector<byte> m_vertexShader;

	std::vector<byte> m_pixelShader;

	ID3D12Resource* m_pIndexBuffer;

	ID3D12DescriptorHeap* m_pShaderBufferHeap;
	unsigned int m_uiShaderBufferDescriptorSize = 0;
	//ID3D12Resource* m_pCBVBuffer[2];
	//unsigned int m_uiCBVDescriptorOffset = 0;
	//ID3D12Resource* m_pSRVBuffer;

	ID3D12DescriptorHeap* m_pSamplerHeap;

	ID3D12DescriptorHeap* m_pGBufferRTVHeap;
	ID3D12DescriptorHeap* m_pGBufferSRVHeap;
	ID3D12Resource* m_pGBuffers[EGBuffer::GB_NUM];

	SSceneProxy m_SceneProxy;

	HANDLE m_hFenceEvent;
	bool m_bVSync;
	bool m_bFullScreen;

private:
	static constexpr unsigned int c_NumDescriptorsPerHeap = 64;
};