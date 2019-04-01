#pragma once

#include "DX12InputVertexAttributes.h"

class SDX12Resources
{
public:
	virtual ~SDX12Resources() {}
	
	virtual void CreateConstantBuffer(SDirectX12Device* pDevice, class SDX12DescriptorHeap* pDescriptorHeap, unsigned int descriptorSize, SBatchProxy* batchProxy) = 0;
	virtual ID3D12Resource** GetCBVBuffers(int& count) = 0;
	virtual unsigned int GetCBVOffset() = 0;
	virtual char* GetMappedConstantBuffer(unsigned int index) = 0;
	virtual void UpdateConstantBuffer(SBatchProxy* batchProxy, unsigned int objIndex, SMatrix view, SMatrix projection) = 0;

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

	void CreateConstantBuffer(SDirectX12Device* pDevice, class SDX12DescriptorHeap* pDescriptorHeap, unsigned int descriptorSize, SBatchProxy* batchProxy) override;

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

	void UpdateConstantBuffer(SBatchProxy* batchProxy, unsigned int objIndex, SMatrix view, SMatrix projection) override
	{
		// ModelViewProjection
		SModelViewProjection mvp;
		memset(&mvp, 0, sizeof(SModelViewProjection));
		mvp.View = view;
		mvp.Projection = projection;

		mvp.Model = batchProxy->ObjectProxies[objIndex].Tranformation;
		mvp.NormalMatrix = SMath::NormalMatrix(batchProxy->ObjectProxies[objIndex].Tranformation);
		SModelViewProjection* pMVP = reinterpret_cast<SModelViewProjection*>(GetMappedConstantBuffer(0) + sizeof(SModelViewProjection) * objIndex);

		memcpy(pMVP, &mvp, sizeof(SModelViewProjection));

		// Bone Transformation
		SBoneTransform bones;
		for (unsigned int j = 0; j < MAX_BONES; ++j)
		{
			bones.transform[j] = batchProxy->ObjectProxies[objIndex].BoneTransform[j];
		}
		SBoneTransform* pBone = reinterpret_cast<SBoneTransform*>(GetMappedConstantBuffer(1) + sizeof(SBoneTransform) * objIndex);
		memcpy(pBone, &bones, sizeof(SBoneTransform));
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

	void CreateConstantBuffer(SDirectX12Device* pDevice, class SDX12DescriptorHeap* pDescriptorHeap, unsigned int descriptorSize, SBatchProxy* batchProxy) override;

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

	void UpdateConstantBuffer(SBatchProxy* batchProxy, unsigned int objIndex, SMatrix view, SMatrix projection) override
	{
		// ModelViewProjection
		SModelViewProjection mvp;
		memset(&mvp, 0, sizeof(SModelViewProjection));
		mvp.View = view;
		mvp.Projection = projection;

		mvp.Model = batchProxy->ObjectProxies[objIndex].Tranformation;
		mvp.NormalMatrix = SMath::NormalMatrix(batchProxy->ObjectProxies[objIndex].Tranformation);
		SModelViewProjection* pMVP = reinterpret_cast<SModelViewProjection*>(GetMappedConstantBuffer(0) + sizeof(SModelViewProjection) * objIndex);

		memcpy(pMVP, &mvp, sizeof(SModelViewProjection));
	}

public:
	unsigned int m_uiCBVDescriptorOffset = 0;

private:
	ID3D12Resource* m_pCBVBuffers[1];
	char* m_MappedConstantBuffers[1];
};

class SDX12DeferredResources : public SDX12Resources
{
public:
	SDX12DeferredResources()
	{
		for (int i = 0; i < 1; ++i)
			m_pCBVBuffers[i] = nullptr;
	}

	~SDX12DeferredResources()
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

	void CreateConstantBuffer(SDirectX12Device* pDevice, class SDX12DescriptorHeap* pDescriptorHeap, unsigned int descriptorSize, SBatchProxy* batchProxy) override;

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

	void UpdateConstantBuffer(SBatchProxy* batchProxy, unsigned int objIndex, SMatrix view, SMatrix projection) override
	{
		// Lights
		SLight lights[32];
		memset(lights, 0, sizeof(SLight) * 32);

		SLight* pLights = reinterpret_cast<SLight*>(GetMappedConstantBuffer(0) + sizeof(SLight));

		memcpy(pLights, lights, sizeof(SLight) * 32);
	}

public:
	unsigned int m_uiCBVDescriptorOffset = 0;

private:
	ID3D12Resource* m_pCBVBuffers[1];
	char* m_MappedConstantBuffers[1];
};

class SDX12SsAoResources : public SDX12Resources
{
public:
	SDX12SsAoResources()
	{
		for (int i = 0; i < 1; ++i)
			m_pCBVBuffers[i] = nullptr;
	}

	~SDX12SsAoResources()
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

	void CreateConstantBuffer(SDirectX12Device* pDevice, class SDX12DescriptorHeap* pDescriptorHeap, unsigned int descriptorSize, SBatchProxy* batchProxy) override;

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

	void UpdateConstantBuffer(SBatchProxy* batchProxy, unsigned int objIndex, SMatrix view, SMatrix projection) override
	{
		// Invert Projection
		float a = projection.m[0].x;
		float b = projection.m[1].y;
		float c = projection.m[2].z;
		float d = projection.m[3].z;
		float e = projection.m[2].w;

		SMatrix invProj;
		invProj.m[0].x = 1.0f / a;
		invProj.m[1].y = 1.0f / b;
		invProj.m[2].w = 1.0f / d;
		invProj.m[2].z = 0;
		invProj.m[3].z = 1.0f / e;
		invProj.m[3].w = -c / (d * e);

		SViewProjection viewProj;
		viewProj.InvProjection = invProj;
		viewProj.InvView = view.Inverse();
		viewProj.View = view;
		viewProj.Projection = projection;

		SViewProjection* pViewProj = reinterpret_cast<SViewProjection*>(GetMappedConstantBuffer(0));
		memcpy(pViewProj, &viewProj, sizeof(SViewProjection));
	}

public:
	unsigned int m_uiCBVDescriptorOffset = 0;

private:
	ID3D12Resource* m_pCBVBuffers[1];
	char* m_MappedConstantBuffers[1];
};