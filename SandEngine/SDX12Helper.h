#pragma once

#include "SDirectX12Device.h"

unsigned __int64 UpdateSubresource(ID3D12GraphicsCommandList* pCmdList, ID3D12Resource* pDestinationResource, ID3D12Resource* pIntermediate, unsigned __int64 IntermediateOffset, unsigned int FirstSubresource, unsigned int NumSubresources, D3D12_SUBRESOURCE_DATA* pSrcData);
unsigned __int64 UpdateSubresource(ID3D12GraphicsCommandList* pCmdList, ID3D12Resource* pDestinationResource, ID3D12Resource* pIntermediate, unsigned int FirstSubresource, unsigned int NumSubresources, unsigned __int64 RequiredSize, const D3D12_PLACED_SUBRESOURCE_FOOTPRINT* pLayouts, const unsigned int* pNumRows, const unsigned __int64* pRowSizeInBytes, const D3D12_SUBRESOURCE_DATA* pSrcData);
inline void MemcpySubresource(const D3D12_MEMCPY_DEST* pDest, const D3D12_SUBRESOURCE_DATA* pSrc, size_t RowSizeInBytes, unsigned int NumRows, unsigned int NumSlices);
inline unsigned __int64 GetRequiredIntermediateSize(ID3D12Resource* pDestinationResource, unsigned int FirstSubresource, unsigned int NumSubresources);
