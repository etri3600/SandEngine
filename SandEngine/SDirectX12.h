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
	std::vector<SModelVertex> Vertices;
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
	unsigned int TotalVertexSize = 0;
	unsigned int TotalIndexSize = 0;
	std::map<unsigned int, SBatchProxy> Proxies;
};

class SDX12ResourceAllocator;
class SDX12DescriptorHeapAllocator;
class SDX12Pipeline;
class SDirectX12 : public SIGraphicsInterface
{
public:
	SDirectX12();
	~SDirectX12();

	bool Initialize(const SPlatformSystem* pPlayformSystem, unsigned int screenWidth, unsigned int screenHeight, bool fullScreen, bool vSync) override;
	void Finalize() override;

	bool CreateSwapChain(const SPlatformSystem* pPlatformSystem, const int nNumerator, const int nDenominator) override;
	void CreateViewProjection() override;

	void UpdateBoneTransform(const std::map<unsigned int, std::vector<SModel>>& models) override;

	void Reset() override;
	bool Update(const double delta, std::map<unsigned int, std::vector<SModel>>& models) override;
	void Draw() override;
	bool Render() override;
	void Present() override;

	std::vector<byte>&& CompileShader(const wchar_t* fileName, const char* version, ID3DBlob** pBlob);

protected:
	void CreateConstantBuffer(SBatchProxy sceneProxy);
	void CreateShaderResources(SBatchProxy sceneProxy);

	void UpdateConstantBuffer(unsigned int sceneIndex, unsigned char* pMappedConstant);
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
	std::vector<SDX12Pipeline*> m_pipelines;

	ID3D12CommandQueue* m_pCommandQueue;
	ID3D12DescriptorHeap* m_pRenderTargetViewHeap;
	ID3D12Resource* m_pBackBufferRenterTarget[c_BufferingCount];
	ID3D12CommandAllocator* m_pCommandAllocator[c_BufferingCount];
	ID3D12GraphicsCommandList* m_pCommandList;

	ID3D12CommandAllocator* m_pBundleAllocator;
	ID3D12GraphicsCommandList* m_pBundleList;

	ID3D12RootSignature* m_pRootSignature;
	//ID3D12PipelineState* m_pPipelineState;
	ID3D12Fence* m_pFence;
	unsigned __int64 m_nFenceValue[c_BufferingCount];
	IDXGISwapChain3* m_pSwapChain;
	unsigned int m_RenderTargetViewDescriptorSize = 0;
	D3D12_VIEWPORT m_Viewport;

	D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView;
	ID3D12Resource* m_pVertexBuffer;
	std::vector<byte> m_vertexShader;

	std::vector<byte> m_pixelShader;

	D3D12_INDEX_BUFFER_VIEW m_IndexBufferView;
	ID3D12Resource* m_pIndexBuffer;

	ID3D12DescriptorHeap* m_pShaderBufferHeap;
	unsigned int m_uiShaderBufferDescriptorSize = 0;
	//ID3D12Resource* m_pCBVBuffer[2];
	//unsigned int m_uiCBVDescriptorOffset = 0;
	//ID3D12Resource* m_pSRVBuffer;

	ID3D12DescriptorHeap* m_pSamplerHeap;

	SSceneProxy m_SceneProxy;

	HANDLE m_hFenceEvent;
	bool m_bVSync;
	bool m_bFullScreen;

private:
	static constexpr unsigned int c_NumDescriptorsPerHeap = 64;
};