#if __WINDOWS__
#include "DX12DescriptorHeap.h"
#include "DirectX12Device.h"

SDX12DescriptorHeap::SDX12DescriptorHeap(SDirectX12Device* device)
	: m_pDevice(device)
{
}

SDX12DescriptorHeap::~SDX12DescriptorHeap()
{
}

bool SDX12DescriptorHeap::Init(const D3D12_DESCRIPTOR_HEAP_DESC& desc)
{
	ID3D12DescriptorHeap* heap;
	m_pDevice->GetDevice()->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&heap));

	m_pDescriptorHeap = heap;
	heap->Release();

	m_HeapDesc = m_pDescriptorHeap->GetDesc();
	m_HeapStartCPU = m_pDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_HeapStartGPU = m_pDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	m_DescSize = m_pDevice->GetDevice()->GetDescriptorHandleIncrementSize(m_HeapDesc.Type);

	Reset();
	return true;
}

SDX12DescriptorBlock::SDX12DescriptorBlock(const SDescriptorBlock& block)
{
	m_pDescriptorHeap.reset(reinterpret_cast<SDX12DescriptorHeap*>(block.pBuffer));
	m_BlockStart = block.offset;
	m_Capacity = block.size;
	m_Cursor = 0;
}
#endif