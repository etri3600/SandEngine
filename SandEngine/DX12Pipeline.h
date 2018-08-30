#pragma once

#include "DirectX12.h"
#include "DX12Resources.h"
#include "DX12RootSignature.h"

class SDX12Pipeline
{
public:
	SDX12Pipeline(SDirectX12Device* pDevice, SDX12RootSignature* pRootSignature, EMaterialType materialType);
	~SDX12Pipeline();

	void Init(std::wstring vertexShader, std::wstring fragmentShader, std::string vertexEntrypointName, std::string fragmentEntrypointName, SDX12Resources* pResources, UINT renderTargetCount, DXGI_FORMAT* renderTargetFormat);

	void CreateConstantBuffer(ID3D12DescriptorHeap* pDescriptorHeap, unsigned int descriptorOffset, unsigned int descriptorSize, SBatchProxy* batchProxy);
	unsigned int CreateShaderResources(ID3D12GraphicsCommandList* commandList, ID3D12DescriptorHeap* heap, SBatchProxy* batchProxy, unsigned int offset);
	void SetVertexBufferView(D3D12_GPU_VIRTUAL_ADDRESS bufferLocation, unsigned int sizeInBytes, unsigned int strideInBytes);
	void SetIndexBufferView(D3D12_GPU_VIRTUAL_ADDRESS bufferLocation, unsigned int sizeInBytes, DXGI_FORMAT format);
	void UpdateConstantBuffer(SBatchProxy* batchProxy, unsigned int objIndex, SMatrix view, SMatrix projection);
	void Populate(ID3D12GraphicsCommandList* commandList, unsigned int index);

	SDX12RootSignature* GetRootSignature() { return m_pRootSignature; }
	ID3D12PipelineState* GetPipelineState() { return m_pPipelineState; }
	unsigned int GetCBVDescriptorOffset();
	char* GetMappedConstantBuffer(unsigned int index);
	D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView() { return m_VertexBufferView; }
	D3D12_INDEX_BUFFER_VIEW GetIndexBufferView() { return m_IndexBufferView; }
	EMaterialType GetMaterialType() const { return m_MaterialType; }

private:
	SDirectX12Device* m_pDevice;
	SDX12Resources* m_pResources;
	SDX12RootSignature* m_pRootSignature;
	ID3D12PipelineState* m_pPipelineState;
	D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView;
	D3D12_INDEX_BUFFER_VIEW m_IndexBufferView;
	EMaterialType m_MaterialType;
};