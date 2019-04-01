#pragma once

#include "Graphics/GraphicsBuffer.h"

#include <mutex>
#include <vector>
#include <queue>
#include <string>
#include <cassert>
#include <d3d12.h>

struct SDX12Cursor
{
	SDX12Cursor() : m_Cursor(0) {}

	inline void SetCursor(unsigned int value) { m_Cursor = value; }
	inline unsigned int GetCursor() const { return m_Cursor; }
	inline void IncrementCursor(unsigned int step = 1) { m_Cursor += step; }
	inline void Reset() { m_Cursor = 0; }

	unsigned int m_Cursor;
};

class SDX12DescriptorHeap : public SDX12Cursor
{
public:
	SDX12DescriptorHeap(class SDirectX12Device* device);
	virtual ~SDX12DescriptorHeap();

	bool Init(const D3D12_DESCRIPTOR_HEAP_DESC& desc);

	inline ID3D12DescriptorHeap* GetD3D12DescriptorHeap() const
	{
		return m_pDescriptorHeap;
	}

	inline unsigned int GetDescriptorSize() const
	{
		return m_DescSize;
	}

	inline unsigned int GetCapacity() const
	{
		return m_HeapDesc.NumDescriptors;
	}

	inline D3D12_CPU_DESCRIPTOR_HANDLE GetHandleOffsetCPUFromCursor(INT offset) const
	{
		D3D12_CPU_DESCRIPTOR_HANDLE handle;
		handle.ptr = m_HeapStartCPU.ptr + (m_Cursor + offset) * m_DescSize;
		return handle;
	}

	inline D3D12_CPU_DESCRIPTOR_HANDLE GetHandleOffsetCPU(INT offset) const
	{
		D3D12_CPU_DESCRIPTOR_HANDLE handle;
		handle.ptr = m_HeapStartCPU.ptr + offset * m_DescSize;
		return handle;
	}

	inline D3D12_GPU_DESCRIPTOR_HANDLE GetHandleOffsetGPUFromCursor(INT offset) const
	{
		D3D12_GPU_DESCRIPTOR_HANDLE handle;
		handle.ptr = m_HeapStartGPU.ptr + (m_Cursor + offset) * m_DescSize;
		return handle;
	}

	inline D3D12_GPU_DESCRIPTOR_HANDLE GetHandleOffsetGPU(INT offset) const
	{
		D3D12_GPU_DESCRIPTOR_HANDLE handle;
		handle.ptr = m_HeapStartGPU.ptr + offset * m_DescSize;
	}

	inline D3D12_GPU_DESCRIPTOR_HANDLE GetHandleGPUFromCPU(D3D12_CPU_DESCRIPTOR_HANDLE handle) const
	{
		assert(GetHandleOffsetCPUFromCursor(0).ptr <= handle.ptr && handle.ptr < GetHandleOffsetCPUFromCursor(GetCapacity()).ptr);
		D3D12_GPU_DESCRIPTOR_HANDLE rebase;
		rebase.ptr = m_HeapStartGPU.ptr + handle.ptr - m_HeapStartCPU.ptr;
		return rebase;
	}

	inline D3D12_CPU_DESCRIPTOR_HANDLE GetHandleCPUFromGPU(D3D12_GPU_DESCRIPTOR_HANDLE handle) const
	{
		assert(GetHandleOffsetGPUFromCursor(0).ptr <= handle.ptr && handle.ptr < GetHandleOffsetGPUFromCursor(GetCapacity()).ptr);
		D3D12_CPU_DESCRIPTOR_HANDLE rebase;
		rebase.ptr = m_HeapStartCPU.ptr + handle.ptr - m_HeapStartGPU.ptr;
		return rebase;
	}

	inline INT GetOffsetFromCPUHandle(D3D12_CPU_DESCRIPTOR_HANDLE handle)
	{
		return static_cast<INT>((handle.ptr - m_HeapStartCPU.ptr) / m_DescSize);
	}

private:
	class SDirectX12Device* m_pDevice;
	ID3D12DescriptorHeap* m_pDescriptorHeap;
	D3D12_DESCRIPTOR_HEAP_DESC m_HeapDesc;
	D3D12_CPU_DESCRIPTOR_HANDLE m_HeapStartCPU;
	D3D12_GPU_DESCRIPTOR_HANDLE m_HeapStartGPU;
	unsigned int m_DescSize;
};

class SDX12DescriptorBlock : public SDX12Cursor
{
public:
	SDX12DescriptorBlock()
		: m_BlockStart(0)
		, m_Capacity(0)
	{}

	SDX12DescriptorBlock(SDX12DescriptorHeap* pHeap, unsigned int cursor, unsigned int capacity)
		: m_pDescriptorHeap(pHeap)
		, m_BlockStart(cursor)
		, m_Capacity(capacity)
	{
		assert(cursor + capacity <= pHeap->GetCapacity());
	}

	SDX12DescriptorBlock(const SDescriptorBlock& block);

	inline SDX12DescriptorHeap* GetDescriptorHeap() const
	{
		return m_pDescriptorHeap.get();
	}

	inline unsigned int GetDescriptorSize() const
	{
		return m_pDescriptorHeap->GetDescriptorSize();
	}

	inline unsigned int GetCapacity() const
	{
		return m_Capacity;
	}

	inline D3D12_CPU_DESCRIPTOR_HANDLE GetHandleOffsetCPU(INT offset) const
	{
		assert((offset < 0) || (m_Cursor + offset < m_Capacity));
		return m_pDescriptorHeap->GetHandleOffsetCPU(static_cast<INT>(m_BlockStart + m_Cursor) + offset);
	}

	inline D3D12_GPU_DESCRIPTOR_HANDLE GetHandleOffsetGPU(INT offset) const
	{
		assert((offset < 0) || (m_Cursor + offset < m_Capacity));
		return m_pDescriptorHeap->GetHandleOffsetGPU(static_cast<INT>(m_BlockStart + m_Cursor) + offset);
	}

	inline D3D12_GPU_DESCRIPTOR_HANDLE GetHandleGPUFromCPU(D3D12_CPU_DESCRIPTOR_HANDLE handle) const
	{
		assert(m_pDescriptorHeap->GetHandleOffsetCPUFromCursor(m_BlockStart).ptr <= handle.ptr && handle.ptr < m_pDescriptorHeap->GetHandleOffsetCPUFromCursor(m_BlockStart + GetCapacity()).ptr);
		return m_pDescriptorHeap->GetHandleGPUFromCPU(handle);
	}

	inline D3D12_CPU_DESCRIPTOR_HANDLE GetHandleCPUFromGPU(D3D12_GPU_DESCRIPTOR_HANDLE handle) const
	{
		assert(m_pDescriptorHeap->GetHandleOffsetGPUFromCursor(m_BlockStart).ptr <= handle.ptr && handle.ptr < m_pDescriptorHeap->GetHandleOffsetGPUFromCursor(m_BlockStart + GetCapacity()).ptr);
		return m_pDescriptorHeap->GetHandleCPUFromGPU(handle);
	}

	inline unsigned int GetStartOffset() const { return m_BlockStart; }

private:
	std::shared_ptr<SDX12DescriptorHeap> m_pDescriptorHeap;
	unsigned int m_BlockStart;
	unsigned int m_Capacity;
};