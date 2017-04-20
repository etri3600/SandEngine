#pragma once

#include "SDirectX12.h"
#include "SDX12Resources.h"

class SDX12Pipeline
{
public:
	SDX12Pipeline(SDirectX12Device* pDevice, ID3D12RootSignature* pRootSignature);
	~SDX12Pipeline();

	void Init(std::wstring vertexShader, std::wstring pixelShader, SDX12Resources* pResources);

	ID3D12RootSignature* GetRootSignature() { return m_pRootSignature; }
	ID3D12PipelineState* GetPipelineState() { return m_pPipelineState; }
	unsigned int GetCBVDescriptorOffset();
	char* GetMappedConstantBuffer(unsigned int index);
	bool HasType(unsigned int);

	void Populate(ID3D12GraphicsCommandList* commandList);
	void CreateConstantBuffer(ID3D12DescriptorHeap* pDescriptorHeap, unsigned int descriptorSize, SBatchProxy batchProxy);

private:
	SDirectX12Device* m_pDevice;
	SDX12Resources* m_pResources;
	std::vector<byte> m_vertexShader, m_pixelShader;
	ID3D12RootSignature* m_pRootSignature;
	ID3D12PipelineState* m_pPipelineState;
};