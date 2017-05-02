#pragma once

#include "SDirectX12.h"
#include "SDX12Resources.h"
#include "SDX12RootSignature.h"

class SDX12Pipeline
{
public:
	SDX12Pipeline(SDirectX12Device* pDevice, SDX12RootSignature* pRootSignature);
	~SDX12Pipeline();

	void Init(std::wstring vertexShader, std::wstring pixelShader, SDX12Resources* pResources);

	SDX12RootSignature* GetRootSignature() { return m_pRootSignature; }
	ID3D12PipelineState* GetPipelineState() { return m_pPipelineState; }
	unsigned int GetCBVDescriptorOffset();
	char* GetMappedConstantBuffer(unsigned int index);
	bool HasType(unsigned int);
	D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView() { return m_VertexBufferView; }
	D3D12_INDEX_BUFFER_VIEW GetIndexBufferView() { return m_IndexBufferView; }

	void Populate(ID3D12GraphicsCommandList* commandList, unsigned int index);
	void CreateConstantBuffer(ID3D12DescriptorHeap* pDescriptorHeap, unsigned int descriptorOffset, unsigned int descriptorSize, SBatchProxy batchProxy);
	void SetVertexBufferView(D3D12_GPU_VIRTUAL_ADDRESS bufferLocation, unsigned int sizeInBytes, unsigned int strideInBytes);
	void SetIndexBufferView(D3D12_GPU_VIRTUAL_ADDRESS bufferLocation, unsigned int sizeInBytes, DXGI_FORMAT format);

private:
	SDirectX12Device* m_pDevice;
	SDX12Resources* m_pResources;
	std::vector<byte> m_vertexShader, m_pixelShader;
	SDX12RootSignature* m_pRootSignature;
	ID3D12PipelineState* m_pPipelineState;
	D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView;
	D3D12_INDEX_BUFFER_VIEW m_IndexBufferView;
};