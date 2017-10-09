#pragma once

#include "SDirectX12Device.h"
#include <vector>

unsigned __int64 UpdateSubresource(ID3D12GraphicsCommandList* pCmdList, ID3D12Resource* pDestinationResource, ID3D12Resource* pIntermediate, unsigned __int64 IntermediateOffset, unsigned int FirstSubresource, unsigned int NumSubresources, D3D12_SUBRESOURCE_DATA* pSrcData);
unsigned __int64 UpdateSubresource(ID3D12GraphicsCommandList* pCmdList, ID3D12Resource* pDestinationResource, ID3D12Resource* pIntermediate, unsigned int FirstSubresource, unsigned int NumSubresources, unsigned __int64 RequiredSize, const D3D12_PLACED_SUBRESOURCE_FOOTPRINT* pLayouts, const unsigned int* pNumRows, const unsigned __int64* pRowSizeInBytes, const D3D12_SUBRESOURCE_DATA* pSrcData);
std::vector<byte>&& CompileShader(const wchar_t* fileName, const char* version, const char* entrypointName, ID3DBlob** pBlob);


inline void MemcpySubresource(
	const D3D12_MEMCPY_DEST* pDest,
	const D3D12_SUBRESOURCE_DATA* pSrc,
	size_t RowSizeInBytes,
	unsigned int NumRows,
	unsigned int NumSlices)
{
	for (unsigned int z = 0; z < NumSlices; ++z)
	{
		byte* pDestSlice = reinterpret_cast<byte*>(pDest->pData) + pDest->SlicePitch * z;
		const byte* pSrcSlice = reinterpret_cast<const byte*>(pSrc->pData) + pSrc->SlicePitch * z;
		for (unsigned int y = 0; y < NumRows; ++y)
		{
			memcpy(pDestSlice + pDest->RowPitch * y,
				pSrcSlice + pSrc->RowPitch * y,
				RowSizeInBytes);
		}
	}
}

inline unsigned __int64 GetRequiredIntermediateSize(ID3D12Resource * pDestinationResource, unsigned int FirstSubresource, unsigned int NumSubresources)
{
	D3D12_RESOURCE_DESC Desc = pDestinationResource->GetDesc();
	unsigned __int64 RequiredSize = 0;

	ID3D12Device* pDevice;
	pDestinationResource->GetDevice(__uuidof(*pDevice), reinterpret_cast<void**>(&pDevice));
	pDevice->GetCopyableFootprints(&Desc, FirstSubresource, NumSubresources, 0, nullptr, nullptr, nullptr, &RequiredSize);
	pDevice->Release();

	return RequiredSize;
}
