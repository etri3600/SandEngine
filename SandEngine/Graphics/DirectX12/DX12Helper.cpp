#if __WINDOWS__
#include "DX12Helper.h"
#include <fstream>
#include <string>
#include <iterator>
#include "Windows.h"

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

void CompileShader(const wchar_t* fileName, const char* version, const char* entrypointName, ID3DBlob** pBlob)
{
	std::vector<byte> shader;
	constexpr auto shaderFolder = LR"(../../SandEngine/Shaders/)";
	constexpr auto shaderExtension = L".hlsl";
	constexpr auto csoFolder = LR"(../../Shaders/)";
	constexpr auto csoExtension = L".cso";
	std::wstring file(fileName);

	D3D_SHADER_MACRO defines[] =
	{
		NULL, NULL
	};
	unsigned int flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(_DEBUG)
	flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
	ID3DBlob* pShaderCode = nullptr;
	ID3DBlob* pErrorCode = nullptr;
	auto stringFile = shaderFolder + file + shaderExtension;
	const wchar_t* shaderFileName = stringFile.c_str();
	HRESULT hResult = D3DCompileFromFile(shaderFileName, defines, D3D_COMPILE_STANDARD_FILE_INCLUDE, entrypointName, version, flags, 0, &pShaderCode, &pErrorCode);
	if (FAILED(hResult))
	{
		if (pErrorCode)
		{
			std::string errorMsg(static_cast<const char*>(pErrorCode->GetBufferPointer()), pErrorCode->GetBufferSize());
			OutputDebugStringA(errorMsg.c_str());
		}
		else if(pShaderCode)
		{
			pShaderCode->Release();

			std::streampos end;

			std::ifstream vertexFile(csoFolder + file + csoExtension, std::ios::in | std::ios::binary | std::ios::ate);
			std::noskipws(vertexFile);
			if (vertexFile.is_open())
			{
				end = vertexFile.tellg();
				vertexFile.seekg(0, std::ios::beg);
				shader.reserve(end);
				char* c = new char[end];
				shader.assign(std::istream_iterator<byte>(vertexFile), std::istream_iterator<byte>());
				vertexFile.close();
			}
		}
	}
	else
	{
		*pBlob = pShaderCode;
	}

#if defined(_DEBUG)
	ID3D12ShaderReflection* pReflection = nullptr;
	if (pShaderCode && SUCCEEDED(D3DReflect(pShaderCode->GetBufferPointer(), pShaderCode->GetBufferSize(), IID_ID3D12ShaderReflection, (void**)&pReflection)))
	{
		D3D12_SIGNATURE_PARAMETER_DESC desc[4];
		pReflection->GetInputParameterDesc(0, &desc[0]);
		pReflection->GetInputParameterDesc(1, &desc[1]);
		pReflection->GetInputParameterDesc(2, &desc[2]);
		pReflection->GetInputParameterDesc(3, &desc[3]);
	}
#endif

	shader.clear();
}
#endif