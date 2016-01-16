#include "SDX12Helper.h"

unsigned __int64 UpdateSubresource(ID3D12GraphicsCommandList * pCmdList, ID3D12Resource * pDestinationResource, ID3D12Resource * pIntermediate, unsigned __int64 IntermediateOffset, unsigned int FirstSubresource, unsigned int NumSubresources, D3D12_SUBRESOURCE_DATA * pSrcData)
{
	unsigned __int64 RequiredSize = 0;
	unsigned __int64 MemToAlloc = static_cast<unsigned __int64>(sizeof(D3D12_PLACED_SUBRESOURCE_FOOTPRINT) + sizeof(unsigned int) + sizeof(unsigned __int64)) * NumSubresources;

	if (MemToAlloc > SIZE_MAX)
		return 0;

	void* pMem = HeapAlloc(GetProcessHeap(), 0, static_cast<size_t>(MemToAlloc));
	if (pMem == nullptr)
		return 0;

	D3D12_PLACED_SUBRESOURCE_FOOTPRINT* pLayouts = reinterpret_cast<D3D12_PLACED_SUBRESOURCE_FOOTPRINT*>(pMem);
	unsigned int* pNumRows = reinterpret_cast<unsigned int*>(pLayouts + NumSubresources);
	unsigned __int64* pRowSizeInBytes = reinterpret_cast<unsigned __int64*>(pNumRows + NumSubresources);

	D3D12_RESOURCE_DESC desc = pDestinationResource->GetDesc();
	ID3D12Device* pDevice = nullptr;
	pDestinationResource->GetDevice(__uuidof(*pDevice), reinterpret_cast<void**>(&pDevice));
	pDevice->GetCopyableFootprints(&desc, FirstSubresource, NumSubresources, IntermediateOffset, pLayouts, pNumRows, pRowSizeInBytes, &RequiredSize);
	pDevice->Release();

	unsigned __int64 Result = UpdateSubresource(pCmdList, pDestinationResource, pIntermediate, FirstSubresource, NumSubresources, RequiredSize, pLayouts, pNumRows, pRowSizeInBytes, pSrcData);
	HeapFree(GetProcessHeap(), 0, pMem);

	return Result;
}

unsigned __int64 UpdateSubresource(ID3D12GraphicsCommandList* pCmdList, ID3D12Resource* pDestinationResource, ID3D12Resource* pIntermediate, unsigned int FirstSubresource, unsigned int NumSubresources, unsigned __int64 RequiredSize, const D3D12_PLACED_SUBRESOURCE_FOOTPRINT* pLayouts, const unsigned int* pNumRows, const unsigned __int64* pRowSizeInBytes, const D3D12_SUBRESOURCE_DATA* pSrcData)
{
	D3D12_RESOURCE_DESC intermediateDesc = pIntermediate->GetDesc();
	D3D12_RESOURCE_DESC destinationDesc = pDestinationResource->GetDesc();
	if (intermediateDesc.Dimension != D3D12_RESOURCE_DIMENSION_BUFFER ||
		intermediateDesc.Width < RequiredSize + pLayouts[0].Offset ||
		RequiredSize > SIZE_MAX ||
		(destinationDesc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER && (FirstSubresource != 0 || NumSubresources != 1)))
	{
		return 0;
	}

	byte* pData = nullptr;
	HRESULT hResult = pIntermediate->Map(0, nullptr, reinterpret_cast<void**>(&pData));
	if (FAILED(hResult))
		return 0;

	for (unsigned int i = 0;i < NumSubresources; ++i)
	{
		if (pRowSizeInBytes[i] > SIZE_MAX)
			return 0;
		D3D12_MEMCPY_DEST destData = { pData + pLayouts[i].Offset, pLayouts[i].Footprint.RowPitch, pLayouts[i].Footprint.RowPitch * pNumRows[i] };
		MemcpySubresource(&destData, &pSrcData[i], (size_t)pRowSizeInBytes[i], pNumRows[i], pLayouts[i].Footprint.Depth);
	}
	pIntermediate->Unmap(0, nullptr);

	if (destinationDesc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
	{
		//D3D12_BOX SrcBox{ unsigned int(pLayouts[0].Offset), 0, 0, unsigned int(pLayouts[0].Offset + pLayouts[0].Footprint.Width), 1, 1 };
		pCmdList->CopyBufferRegion(pDestinationResource, 0, pIntermediate, pLayouts[0].Offset, pLayouts[0].Footprint.Width);
	}
	else
	{
		for (unsigned int i = 0;i < NumSubresources; ++i)
		{
			D3D12_TEXTURE_COPY_LOCATION Dest{ pDestinationResource, D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX, i + FirstSubresource };
			D3D12_TEXTURE_COPY_LOCATION Src{ pIntermediate, D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT, pLayouts[i] };
			pCmdList->CopyTextureRegion(&Dest, 0, 0, 0, &Src, nullptr);
		}
	}

	return RequiredSize;
}
