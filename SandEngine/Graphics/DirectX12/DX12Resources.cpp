#if __WINDOWS__
#include "DX12Resources.h"
#include "DX12DescriptorHeap.h"

#define ALIGNED_SIZE(size, align) ((size + (align-1)) & ~(align-1))

void SDX12SkinnedResources::CreateConstantBuffer(SDirectX12Device* pDevice, SDX12DescriptorHeap* pDescriptorHeap, unsigned int descriptorSize, SBatchProxy* batchProxy)
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

	constantBufferDesc.Width = sizeof(SModelViewProjection) * batchProxy->ObjectProxies.size();
	HRESULT hResult = pDevice->GetDevice()->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &constantBufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_pCBVBuffers[0]));
	m_pCBVBuffers[0]->SetName(L"MVP Buffer");

	constantBufferDesc.Width = sizeof(SBoneTransform) * batchProxy->ObjectProxies.size();
	hResult = pDevice->GetDevice()->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &constantBufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_pCBVBuffers[1]));
	m_pCBVBuffers[1]->SetName(L"Bone Buffer");

	D3D12_GPU_VIRTUAL_ADDRESS cbvGPUAddress[2];
	cbvGPUAddress[0] = m_pCBVBuffers[0]->GetGPUVirtualAddress();
	cbvGPUAddress[1] = m_pCBVBuffers[1]->GetGPUVirtualAddress();
	
	D3D12_CONSTANT_BUFFER_VIEW_DESC desc;

	for (unsigned int i = 0; i < batchProxy->ObjectProxies.size(); ++i)
	{
		desc.BufferLocation = cbvGPUAddress[0];
		desc.SizeInBytes = sizeof(SModelViewProjection);
		pDevice->GetDevice()->CreateConstantBufferView(&desc, pDescriptorHeap->GetHandleOffsetCPUFromCursor(0));

		cbvGPUAddress[0] += desc.SizeInBytes;
		pDescriptorHeap->IncrementCursor();

		desc.BufferLocation = cbvGPUAddress[1];
		desc.SizeInBytes = sizeof(SBoneTransform);
		pDevice->GetDevice()->CreateConstantBufferView(&desc, pDescriptorHeap->GetHandleOffsetCPUFromCursor(0));

		cbvGPUAddress[1] += desc.SizeInBytes;
		pDescriptorHeap->IncrementCursor();

		m_uiCBVDescriptorOffset += descriptorSize * sizeof(cbvGPUAddress) / sizeof(cbvGPUAddress[0]);
	}

	D3D12_RANGE readRange;
	readRange.Begin = readRange.End = 0;
	m_pCBVBuffers[0]->Map(0, &readRange, reinterpret_cast<void**>(&m_MappedConstantBuffers[0]));
	memset(m_MappedConstantBuffers[0], 0, sizeof(SModelViewProjection) * batchProxy->ObjectProxies.size());

	m_pCBVBuffers[1]->Map(0, &readRange, reinterpret_cast<void**>(&m_MappedConstantBuffers[1]));
	memset(m_MappedConstantBuffers[1], 0, sizeof(SBoneTransform) * batchProxy->ObjectProxies.size());
}

void SDX12TextureResources::CreateConstantBuffer(SDirectX12Device* pDevice, SDX12DescriptorHeap* pDescriptorHeap, unsigned int descriptorSize, SBatchProxy* batchProxy)
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

	constantBufferDesc.Width = sizeof(SModelViewProjection) * batchProxy->ObjectProxies.size();
	HRESULT hResult = pDevice->GetDevice()->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &constantBufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_pCBVBuffers[0]));
	m_pCBVBuffers[0]->SetName(L"MVP Buffer");

	D3D12_GPU_VIRTUAL_ADDRESS cbvGPUAddress[1];
	cbvGPUAddress[0] = m_pCBVBuffers[0]->GetGPUVirtualAddress();

	D3D12_CONSTANT_BUFFER_VIEW_DESC desc;

	for (unsigned int i = 0; i < batchProxy->ObjectProxies.size(); ++i)
	{
		desc.BufferLocation = cbvGPUAddress[0];
		desc.SizeInBytes = sizeof(SModelViewProjection);
		pDevice->GetDevice()->CreateConstantBufferView(&desc, pDescriptorHeap->GetHandleOffsetCPUFromCursor(0));

		cbvGPUAddress[0] += desc.SizeInBytes;
		pDescriptorHeap->IncrementCursor();

		m_uiCBVDescriptorOffset += descriptorSize;
	}

	D3D12_RANGE readRange;
	readRange.Begin = readRange.End = 0;
	m_pCBVBuffers[0]->Map(0, &readRange, reinterpret_cast<void**>(&m_MappedConstantBuffers[0]));
	memset(m_MappedConstantBuffers[0], 0, sizeof(SModelViewProjection) * batchProxy->ObjectProxies.size());
}

void SDX12DeferredResources::CreateConstantBuffer(SDirectX12Device* pDevice, SDX12DescriptorHeap* pDescriptorHeap, unsigned int descriptorSize, SBatchProxy* batchProxy)
{
}

void SDX12SsAoResources::CreateConstantBuffer(SDirectX12Device* pDevice, SDX12DescriptorHeap* pDescriptorHeap, unsigned int descriptorSize, SBatchProxy* batchProxy)
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

	constantBufferDesc.Width = ALIGNED_SIZE(sizeof(SViewProjection), 256);
	HRESULT hResult = pDevice->GetDevice()->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &constantBufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_pCBVBuffers[0]));
	m_pCBVBuffers[0]->SetName(L"Inverse Projection Matrix");

	D3D12_GPU_VIRTUAL_ADDRESS cbvGPUAddress[1];
	cbvGPUAddress[0] = m_pCBVBuffers[0]->GetGPUVirtualAddress();

	D3D12_CONSTANT_BUFFER_VIEW_DESC desc;
	desc.BufferLocation = cbvGPUAddress[0];
	desc.SizeInBytes = ALIGNED_SIZE(sizeof(SViewProjection), 256);
	pDevice->GetDevice()->CreateConstantBufferView(&desc, pDescriptorHeap->GetHandleOffsetCPUFromCursor(0));
	pDescriptorHeap->IncrementCursor();

	D3D12_RANGE readRange;
	readRange.Begin = readRange.End = 0;
	m_pCBVBuffers[0]->Map(0, &readRange, reinterpret_cast<void**>(&m_MappedConstantBuffers[0]));
	memset(m_MappedConstantBuffers[0], 0, ALIGNED_SIZE(sizeof(SViewProjection), 256));
}
#endif