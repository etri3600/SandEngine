#pragma once

#include "SDX12InputVertexAttributes.h"

class SDX12Resources
{
public:
	virtual ~SDX12Resources() {}
	
	virtual void CreateConstantBuffer(SDirectX12Device* pDevice, ID3D12DescriptorHeap* pDescriptorHeap, unsigned int descriptorSize, SBatchProxy batchProxy) = 0;
	virtual ID3D12Resource** GetCBVBuffers(int& count) = 0;
	virtual unsigned int GetCBVOffset() = 0;
	virtual char* GetMappedConstantBuffer(unsigned int index) = 0;
	virtual bool HasType(unsigned int index) = 0;

public:
	SDX12InputVertexAttributes Attributes;
};


class SDX12SkinnedResources : public SDX12Resources
{
public:
	SDX12SkinnedResources()
	{
		Attributes.AddAttribute({ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
		Attributes.AddAttribute({ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
		Attributes.AddAttribute({ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
		Attributes.AddAttribute({ "TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
		Attributes.AddAttribute({ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
		Attributes.AddAttribute({ "BLENDINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
		Attributes.AddAttribute({ "BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
	}

	~SDX12SkinnedResources()
	{
		for (int i = 0; i < 2; ++i)
		{
			if (m_pCBVBuffers[i])
			{
				m_pCBVBuffers[i]->Unmap(0, nullptr);
				m_pCBVBuffers[i]->Release();
				m_pCBVBuffers[i] = nullptr;
			}
		}
	}

	void CreateConstantBuffer(SDirectX12Device* pDevice, ID3D12DescriptorHeap* pDescriptorHeap, unsigned int descriptorSize, SBatchProxy batchProxy) override
	{
		D3D12_HEAP_PROPERTIES uploadHeapProperties;
		uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
		uploadHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		uploadHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		uploadHeapProperties.CreationNodeMask = 1;
		uploadHeapProperties.VisibleNodeMask = 1;

		D3D12_RESOURCE_DESC constantBufferDesc = {};
		constantBufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		constantBufferDesc.Alignment = 0;
		constantBufferDesc.Height = 1;
		constantBufferDesc.DepthOrArraySize = 1;
		constantBufferDesc.MipLevels = 1;
		constantBufferDesc.Format = DXGI_FORMAT_UNKNOWN;
		constantBufferDesc.SampleDesc.Count = 1;
		constantBufferDesc.SampleDesc.Quality = 0;
		constantBufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		constantBufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		constantBufferDesc.Width = sizeof(SModelViewProjection) * batchProxy.ObjectProxies.size();
		HRESULT hResult = pDevice->GetDevice()->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &constantBufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_pCBVBuffers[0]));
		m_pCBVBuffers[0]->SetName(L"MVP Buffer");

		constantBufferDesc.Width = sizeof(SBoneTransform) * batchProxy.ObjectProxies.size();
		hResult = pDevice->GetDevice()->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &constantBufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_pCBVBuffers[1]));
		m_pCBVBuffers[1]->SetName(L"Bone Buffer");

		D3D12_GPU_VIRTUAL_ADDRESS cbvGPUAddress[2];
		cbvGPUAddress[0] = m_pCBVBuffers[0]->GetGPUVirtualAddress();
		cbvGPUAddress[1] = m_pCBVBuffers[1]->GetGPUVirtualAddress();
		D3D12_CPU_DESCRIPTOR_HANDLE cbvCPUHandle = pDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

		D3D12_CONSTANT_BUFFER_VIEW_DESC desc;

		for (unsigned int i = 0; i < batchProxy.ObjectProxies.size(); ++i)
		{
			desc.BufferLocation = cbvGPUAddress[0];
			desc.SizeInBytes = sizeof(SModelViewProjection);
			pDevice->GetDevice()->CreateConstantBufferView(&desc, cbvCPUHandle);

			cbvGPUAddress[0] += desc.SizeInBytes;
			cbvCPUHandle.ptr += descriptorSize;

			desc.BufferLocation = cbvGPUAddress[1];
			desc.SizeInBytes = sizeof(SBoneTransform);
			pDevice->GetDevice()->CreateConstantBufferView(&desc, cbvCPUHandle);

			cbvGPUAddress[1] += desc.SizeInBytes;
			cbvCPUHandle.ptr += descriptorSize;

			m_uiCBVDescriptorOffset += descriptorSize * sizeof(cbvGPUAddress) / sizeof(cbvGPUAddress[0]);
		}

		D3D12_RANGE readRange;
		readRange.Begin = readRange.End = 0;
		m_pCBVBuffers[0]->Map(0, &readRange, reinterpret_cast<void**>(&m_MappedConstantBuffers[0]));
		memset(m_MappedConstantBuffers[0], 0, sizeof(SModelViewProjection) * batchProxy.ObjectProxies.size());

		m_pCBVBuffers[1]->Map(0, &readRange, reinterpret_cast<void**>(&m_MappedConstantBuffers[1]));
		memset(m_MappedConstantBuffers[1], 0, sizeof(SBoneTransform) * batchProxy.ObjectProxies.size());
	}

	ID3D12Resource** GetCBVBuffers(int& count) override
	{
		count = sizeof(m_pCBVBuffers) / sizeof(m_pCBVBuffers[0]);
		return m_pCBVBuffers;
	}

	unsigned int GetCBVOffset() override
	{
		return m_uiCBVDescriptorOffset;
	}

	char* GetMappedConstantBuffer(unsigned int index) override
	{
		return m_MappedConstantBuffers[index];
	}

	bool HasType(unsigned int index) override
	{
		return index < 2;
	}

public:
	unsigned int m_uiCBVDescriptorOffset;

private:
	ID3D12Resource* m_pCBVBuffers[2];
	char* m_MappedConstantBuffers[2];
};

class SDX12SimpleResources : public SDX12Resources
{
public:
	SDX12SimpleResources()
	{
		Attributes.AddAttribute({ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
		Attributes.AddAttribute({ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
		Attributes.AddAttribute({ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
		Attributes.AddAttribute({ "TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
		Attributes.AddAttribute({ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
	}

	~SDX12SimpleResources()
	{
		for (int i = 0; i < 1; ++i)
		{
			if (m_pCBVBuffers[i])
			{
				m_pCBVBuffers[i]->Unmap(0, nullptr);
				m_pCBVBuffers[i]->Release();
				m_pCBVBuffers[i] = nullptr;
			}
		}
	}

	void CreateConstantBuffer(SDirectX12Device* pDevice, ID3D12DescriptorHeap* pDescriptorHeap, unsigned int descriptorSize, SBatchProxy batchProxy) override
	{
		D3D12_HEAP_PROPERTIES uploadHeapProperties;
		uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
		uploadHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		uploadHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		uploadHeapProperties.CreationNodeMask = 1;
		uploadHeapProperties.VisibleNodeMask = 1;

		D3D12_RESOURCE_DESC constantBufferDesc = {};
		constantBufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		constantBufferDesc.Alignment = 0;
		constantBufferDesc.Height = 1;
		constantBufferDesc.DepthOrArraySize = 1;
		constantBufferDesc.MipLevels = 1;
		constantBufferDesc.Format = DXGI_FORMAT_UNKNOWN;
		constantBufferDesc.SampleDesc.Count = 1;
		constantBufferDesc.SampleDesc.Quality = 0;
		constantBufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		constantBufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		constantBufferDesc.Width = sizeof(SModelViewProjection) * batchProxy.ObjectProxies.size();
		HRESULT hResult = pDevice->GetDevice()->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &constantBufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_pCBVBuffers[0]));
		m_pCBVBuffers[0]->SetName(L"MVP Buffer");

		D3D12_GPU_VIRTUAL_ADDRESS cbvGPUAddress[1];
		cbvGPUAddress[0] = m_pCBVBuffers[0]->GetGPUVirtualAddress();
		D3D12_CPU_DESCRIPTOR_HANDLE cbvCPUHandle = pDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

		D3D12_CONSTANT_BUFFER_VIEW_DESC desc;

		for (unsigned int i = 0; i < batchProxy.ObjectProxies.size(); ++i)
		{
			desc.BufferLocation = cbvGPUAddress[0];
			desc.SizeInBytes = sizeof(SModelViewProjection);
			pDevice->GetDevice()->CreateConstantBufferView(&desc, cbvCPUHandle);

			cbvGPUAddress[0] += desc.SizeInBytes;
			cbvCPUHandle.ptr += descriptorSize;

			m_uiCBVDescriptorOffset += descriptorSize;
		}

		D3D12_RANGE readRange;
		readRange.Begin = readRange.End = 0;
		m_pCBVBuffers[0]->Map(0, &readRange, reinterpret_cast<void**>(&m_MappedConstantBuffers[0]));
		memset(m_MappedConstantBuffers[0], 0, sizeof(SModelViewProjection) * batchProxy.ObjectProxies.size());
	}

	ID3D12Resource** GetCBVBuffers(int& count) override
	{
		count = sizeof(m_pCBVBuffers) / sizeof(m_pCBVBuffers[0]);
		return m_pCBVBuffers;
	}

	unsigned int GetCBVOffset() override
	{
		return m_uiCBVDescriptorOffset;
	}

	char* GetMappedConstantBuffer(unsigned int index) override
	{
		return m_MappedConstantBuffers[index];
	}

	bool HasType(unsigned int index) override
	{
		return index < 1;
	}

public:
	unsigned int m_uiCBVDescriptorOffset;

private:
	ID3D12Resource* m_pCBVBuffers[1];
	char* m_MappedConstantBuffers[1];
};