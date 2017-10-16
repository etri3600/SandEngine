#pragma once

#include "SDX12InputVertexAttributes.h"

class SDX12Resources
{
public:
	virtual ~SDX12Resources() {}
	
	virtual void CreateConstantBuffer(SDirectX12Device* pDevice, ID3D12DescriptorHeap* pDescriptorHeap, unsigned int descriptorOffset, unsigned int descriptorSize, SBatchProxy batchProxy) = 0;
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
		for (int i = 0; i < 2; ++i)
			m_pCBVBuffers[i] = nullptr;

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

	void CreateConstantBuffer(SDirectX12Device* pDevice, ID3D12DescriptorHeap* pDescriptorHeap, unsigned int descriptorOffset, unsigned int descriptorSize, SBatchProxy batchProxy) override;

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
	unsigned int m_uiCBVDescriptorOffset = 0;

private:
	ID3D12Resource* m_pCBVBuffers[2];
	char* m_MappedConstantBuffers[2];
};

class SDX12TextureResources : public SDX12Resources
{
public:
	SDX12TextureResources()
	{
		for (int i = 0; i < 1; ++i)
			m_pCBVBuffers[i] = nullptr;

		Attributes.AddAttribute({ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
		Attributes.AddAttribute({ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
		Attributes.AddAttribute({ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
		Attributes.AddAttribute({ "TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
		Attributes.AddAttribute({ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
	}

	~SDX12TextureResources()
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

	void CreateConstantBuffer(SDirectX12Device* pDevice, ID3D12DescriptorHeap* pDescriptorHeap, unsigned int descriptorOffset, unsigned int descriptorSize, SBatchProxy batchProxy) override;

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
	unsigned int m_uiCBVDescriptorOffset = 0;

private:
	ID3D12Resource* m_pCBVBuffers[1];
	char* m_MappedConstantBuffers[1];
};

class SDX12LightResources : public SDX12Resources
{
public:
	SDX12LightResources()
	{
		for (int i = 0; i < 1; ++i)
			m_pCBVBuffers[i] = nullptr;

		Attributes.AddAttribute({ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
	}

	~SDX12LightResources()
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

	void CreateConstantBuffer(SDirectX12Device* pDevice, ID3D12DescriptorHeap* pDescriptorHeap, unsigned int descriptorOffset, unsigned int descriptorSize, SBatchProxy batchProxy) override;

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
	unsigned int m_uiCBVDescriptorOffset = 0;

private:
	ID3D12Resource* m_pCBVBuffers[1];
	char* m_MappedConstantBuffers[1];
};