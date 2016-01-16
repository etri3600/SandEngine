#pragma once

#include "SShaderResourceManager.h"
#include "SDirectX12Device.h"

class SDX12ShaderResourceManager : public SShaderResourceManager
{
public:
	SDX12ShaderResourceManager(SDirectX12Device* pDevice , ID3D12GraphicsCommandList* pCommandList);
	virtual ~SDX12ShaderResourceManager();
	void CreateShaderResource(const SModel& model) override;
	void BindShaderResource() override;

private:
	SDirectX12Device* m_pDevice;
	ID3D12GraphicsCommandList* m_pCommandList;

	ID3D12DescriptorHeap* m_pSRVHeap;
	unsigned int m_uiSRVDescriptorSize = 0;
	ID3D12Resource* m_pSRVBuffer;
};

