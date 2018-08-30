#pragma once

#include "DirectX12Device.h"

class SDX12DescriptorHeapAllocator
{
public:
	SDX12DescriptorHeapAllocator(SDirectX12Device* pDevice, D3D12_DESCRIPTOR_HEAP_TYPE Type)
		:m_pDevice(pDevice), m_Type(Type), m_CurrentHeap(nullptr)
	{}

	D3D12_CPU_DESCRIPTOR_HANDLE Allocate(unsigned int Count);

protected:
	static constexpr unsigned int c_NumDescriptorsPerHeap = 256;

protected:
	ID3D12DescriptorHeap* RequestNewHeap(D3D12_DESCRIPTOR_HEAP_TYPE Type);

	SDirectX12Device* m_pDevice;
	D3D12_DESCRIPTOR_HEAP_TYPE m_Type;
	ID3D12DescriptorHeap* m_CurrentHeap;
	D3D12_CPU_DESCRIPTOR_HANDLE m_CurrentHandle;
	unsigned int m_DescriptorSize;
	unsigned int m_RemainingFreeHandles;
};