#if __WINDOWS__
#include "DX12ResourceAllocator.h"
#include "DirectX12.h"
#include "Windows.h"
#include "DX12Helper.h"

namespace {

}

SDX12ResourceAllocator::SDX12ResourceAllocator(SDirectX12Device* pDevice, ID3D12GraphicsCommandList * pCommandList, HEAP_TYPE heapType)
	:m_pDevice(pDevice), m_pCommandList(pCommandList), m_HeapType(heapType)
{
}

SDX12ResourceAllocator::~SDX12ResourceAllocator()
{
}

SDX12DescriptorHeap SDX12ResourceAllocator::Allocate(unsigned int size)
{
	SDX12DescriptorPage* page = m_PageList[m_CurrentPage];
	if (page == nullptr)
	{
		page = CreatePage();
	}
	else
	{
		if (page->m_Offset + size > GetHeapSize())
		{
			++m_CurrentPage;
			page = CreatePage();
		}
	}
	
	SDX12DescriptorHeap heap(*page, page->m_Offset, size);
	heap.DataPtr = (unsigned char*)page->m_CpuVirtualAddress + page->m_Offset;
	heap.GpuAddress = page->m_GpuVirtualAddress + page->m_Offset;

	page->m_Offset += size;

	return heap;
}

void SDX12ResourceAllocator::CleanUp()
{
	for (auto i = m_CurrentPage; i >= 0; --i)
	{
		if (m_PageList[i])
		{
			m_PageList[i]->m_Resource.Release();
		}
	}
	m_CurrentPage = 0U;
}

SDX12DescriptorPage* SDX12ResourceAllocator::CreatePage()
{
	D3D12_HEAP_PROPERTIES HeapProps;
	HeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	HeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	HeapProps.CreationNodeMask = 1;
	HeapProps.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC ResourceDesc;
	ResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	ResourceDesc.Alignment = 0;
	ResourceDesc.Height = 1;
	ResourceDesc.DepthOrArraySize = 1;
	ResourceDesc.MipLevels = 1;
	ResourceDesc.Format = DXGI_FORMAT_UNKNOWN;
	ResourceDesc.SampleDesc.Count = 1;
	ResourceDesc.SampleDesc.Quality = 0;
	ResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	D3D12_RESOURCE_STATES DefaultUsage;
	if (m_HeapType == HEAP_DEFAULT)
	{
		HeapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
		ResourceDesc.Width = c_DefaultWidth;
		ResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		DefaultUsage = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
	}
	else
	{
		HeapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
		ResourceDesc.Width = c_UploadWidth;
		ResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
		DefaultUsage = D3D12_RESOURCE_STATE_GENERIC_READ;
	}

	ID3D12Resource* pBuffer = nullptr;
	m_pDevice->GetDevice()->CreateCommittedResource(&HeapProps, D3D12_HEAP_FLAG_NONE, &ResourceDesc, DefaultUsage, nullptr, IID_PPV_ARGS(&pBuffer));
	pBuffer->SetName(L"Allocator Resource");


	return new SDX12DescriptorPage(*pBuffer, DefaultUsage, m_HeapType);
}

unsigned int SDX12ResourceAllocator::GetHeapSize() const
{
	return m_HeapType == HEAP_DEFAULT ? c_DefaultWidth : c_UploadWidth;
}
#endif