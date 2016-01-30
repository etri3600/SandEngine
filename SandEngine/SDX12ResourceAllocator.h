#pragma once

#include "SDirectX12Device.h"

enum HEAP_TYPE
{
	HEAP_DEFAULT,
	HEAP_UPLOAD,

	HEAP_TYPE_NUM
};

struct SDX12DescriptorPage
{
	SDX12DescriptorPage(ID3D12Resource& resource, D3D12_RESOURCE_STATES usage, HEAP_TYPE type)
		:m_Resource(resource), m_Usage(usage), m_HeapType(type)
	{
		m_GpuVirtualAddress = m_Resource.GetGPUVirtualAddress();
		m_Resource.Map(0, nullptr, &m_CpuVirtualAddress);
		m_Size = m_Resource.GetDesc().Width;
	}

	ID3D12Resource& m_Resource;
	D3D12_RESOURCE_STATES m_Usage;
	HEAP_TYPE m_HeapType;

	unsigned int m_Size = 0U;
	unsigned int m_Offset = 0U;

	void* m_CpuVirtualAddress = nullptr;
	D3D12_GPU_VIRTUAL_ADDRESS m_GpuVirtualAddress;
};

struct SDX12DescriptorHeap
{
	SDX12DescriptorHeap(SDX12DescriptorPage& page, unsigned int offset, unsigned int size)
		:Page(page), Offset(offset), Size(size)
	{}

	SDX12DescriptorPage& Page;
	unsigned int Offset;
	unsigned int Size;
	void* DataPtr;
	D3D12_GPU_VIRTUAL_ADDRESS GpuAddress;
};

class SDX12ResourceAllocator
{
public:
	SDX12ResourceAllocator(SDirectX12Device* pDevice , ID3D12GraphicsCommandList* pCommandList, HEAP_TYPE heapType);
	virtual ~SDX12ResourceAllocator();

	SDX12DescriptorHeap Allocate(unsigned int size);
	void CleanUp();

private:
	SDX12DescriptorPage* CreatePage();
	inline unsigned int GetHeapSize() const;

private:
	static constexpr unsigned int c_PageNum = 256;
	static constexpr unsigned int c_DefaultWidth = 65536;
	static constexpr unsigned int c_UploadWidth = 1048576;

private:
	SDirectX12Device* m_pDevice;
	ID3D12GraphicsCommandList* m_pCommandList;

	SDX12DescriptorPage* m_PageList[c_PageNum];
	int m_CurrentPage = 0;

	HEAP_TYPE m_HeapType;
};

