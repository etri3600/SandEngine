#pragma once

#include "../IRendering.h"
#include "DirectX12.h"
#include <vector>

class SDX12DeferredRendering : public IRendering
{
public:
	SDX12DeferredRendering(class SDirectX12Device* Device);
	~SDX12DeferredRendering();

	void Render() override;

private:
	void CreateGBuffers();
	void CreatePostProcessResources();
	void CreateRenderingPipeline();

private:
	class SDirectX12Device* m_pDevice;
	std::vector<SDX12RootSignature*> m_RootSignatures;
	std::vector<SDX12Pipeline*> m_pipelines;

	static constexpr unsigned short DescriptorNum = 32;
	ID3D12DescriptorHeap* m_pGBufferRTVHeap = nullptr;
	ID3D12DescriptorHeap* m_pGBufferCbvSrvHeap = nullptr;
	ID3D12Resource* m_pGBuffers[static_cast<unsigned short>(EGBuffer::GB_NUM)];
	ID3D12Resource* m_pPostProcessResource = nullptr;
	SDX12DescriptorHeap PostProcessRTV;
	SDX12DescriptorHeap PostProcessSRV;

	ID3D12DescriptorHeap* m_pSamplerHeap = nullptr;
};