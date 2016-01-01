#pragma once

#include "SGraphicsInterface.h"
#include "SDirectX12Device.h"

struct SDX12SceneProxy
{
	unsigned int CBVDescriptorSize = 0;
	unsigned int RTVDescriptorSize = 0;
	unsigned int BaseVertexLocation = 0;
	unsigned int IndexCountPerInstance = 0;
	unsigned int StartIndexLocation = 0;
	SMatrix Tranformation;
};

class SDirectX12 : public SIGraphicsInterface
{
public:
	SDirectX12();
	~SDirectX12();

	bool Initialize(const SPlatformSystem* pPlayformSystem, unsigned int screenWidth, unsigned int screenHeight, bool fullScreen, bool vSync) override;
	void Finalize() override;

	bool CreateSwapChain(const SPlatformSystem* pPlatformSystem, const int nNumerator, const int nDenominator) override;
	void CreateViewProjection() override;

	bool Update(const double delta) override;
	void Draw(std::vector<SModel>& models) override;
	bool Render() override;
	void Present() override;

	std::vector<byte>&& CompileShader(const wchar_t* fileName, const char* version, ID3DBlob** pBlob);

protected:
	VOID CreateShaderResources(std::vector<SModel>& models);
	VOID CreateConstantBuffer(std::vector<SModel>& models);
	void UpdateConstantBuffer();

private:
	void WaitForGPU();
	void InitBundle();

	ID3D12CommandAllocator* GetCommandAllocator() const { return m_pCommandAllocator[m_BufferIndex]; }
	ID3D12Resource* GetRenderTarget() const { return m_pBackBufferRenterTarget[m_BufferIndex]; }

	unsigned __int64 UpdateSubresource(ID3D12GraphicsCommandList* pCmdList, ID3D12Resource* pDestinationResource, ID3D12Resource* pIntermediate, unsigned __int64 IntermediateOffset, unsigned int FirstSubresource, unsigned int NumSubresources, D3D12_SUBRESOURCE_DATA* pSrcData);
	unsigned __int64 UpdateSubresource(ID3D12GraphicsCommandList* pCmdList, ID3D12Resource* pDestinationResource, ID3D12Resource* pIntermediate, unsigned int FirstSubresource, unsigned int NumSubresources, unsigned __int64 RequiredSize, const D3D12_PLACED_SUBRESOURCE_FOOTPRINT* pLayouts, const unsigned int* pNumRows, const unsigned __int64* pRowSizeInBytes, const D3D12_SUBRESOURCE_DATA* pSrcData);
	inline void MemcpySubresource(const D3D12_MEMCPY_DEST* pDest, const D3D12_SUBRESOURCE_DATA* pSrc, size_t RowSizeInBytes, unsigned int NumRows, unsigned int NumSlices);
	inline unsigned __int64 GetRequiredIntermediateSize(ID3D12Resource* pDestinationResource, unsigned int FirstSubresource, unsigned int NumSubresources);

private:
	SDirectX12Device* m_pDevice;

	ID3D12CommandQueue* m_pCommandQueue;
	ID3D12DescriptorHeap* m_pRenderTargetViewHeap;
	ID3D12Resource* m_pBackBufferRenterTarget[c_BufferingCount];
	ID3D12CommandAllocator* m_pCommandAllocator[c_BufferingCount];
	ID3D12GraphicsCommandList* m_pCommandList;

	ID3D12CommandAllocator* m_pBundleAllocator;
	ID3D12GraphicsCommandList* m_pBundleList;

	ID3D12PipelineState* m_pPipelineState;
	ID3D12Fence* m_pFence;
	unsigned __int64 m_nFenceValue[c_BufferingCount];
	IDXGISwapChain3* m_pSwapChain;
	ID3D12RootSignature* m_pRootSignature;
	unsigned int m_RenderTargetViewDescriptorSize = 0;
	D3D12_VIEWPORT m_Viewport;

	D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView;
	ID3D12Resource* m_pVertexBuffer;
	std::vector<byte> m_vertexShader;

	std::vector<byte> m_pixelShader;

	D3D12_INDEX_BUFFER_VIEW m_IndexBufferView;
	ID3D12Resource* m_pIndexBuffer;

	ID3D12DescriptorHeap* m_pCBVHeap;
	unsigned int m_uiCBVDescriptorSize = 0;
	ID3D12Resource* m_pConstantBuffer;

	ID3D12DescriptorHeap* m_pSRVHeap;
	unsigned int m_uiSRVDescriptorSize = 0;
	ID3D12Resource* m_pSRVBuffer;

	ID3D12DescriptorHeap* m_pSamplerHeap;

	std::vector<SDX12SceneProxy> m_SceneProxy;

	HANDLE m_hFenceEvent;
	bool m_bVSync;
	bool m_bFullScreen;
};