#include "DX12DescriptorHeapAllocator.h"

D3D12_CPU_DESCRIPTOR_HANDLE SDX12DescriptorHeapAllocator::Allocate(unsigned int Count)
{
	if (m_CurrentHeap == nullptr || m_RemainingFreeHandles < Count)
	{
		m_CurrentHeap = RequestNewHeap(m_Type);
		m_CurrentHandle = m_CurrentHeap->GetCPUDescriptorHandleForHeapStart();
		m_RemainingFreeHandles = c_NumDescriptorsPerHeap;

		if (m_DescriptorSize == 0)
			m_DescriptorSize = m_pDevice->GetDevice()->GetDescriptorHandleIncrementSize(m_Type);
	}

	D3D12_CPU_DESCRIPTOR_HANDLE ret = m_CurrentHandle;
	m_CurrentHandle.ptr += Count * m_DescriptorSize;
	m_RemainingFreeHandles -= Count;
	return ret;
}

ID3D12DescriptorHeap* SDX12DescriptorHeapAllocator::RequestNewHeap(D3D12_DESCRIPTOR_HEAP_TYPE Type)
{
	D3D12_DESCRIPTOR_HEAP_DESC Desc;
	Desc.Type = Type;
	Desc.NumDescriptors = c_NumDescriptorsPerHeap;
	Desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	Desc.NodeMask = 1;

	ID3D12DescriptorHeap* pHeap;
	m_pDevice->GetDevice()->CreateDescriptorHeap(&Desc, IID_PPV_ARGS(&pHeap));
	return pHeap;
}
