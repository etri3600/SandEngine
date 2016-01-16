#include "SDX12ShaderResourceManager.h"
#include "SDirectX12.h"
#include "SWindows.h"
#include "SDX12Helper.h"

SDX12ShaderResourceManager::SDX12ShaderResourceManager(SDirectX12Device* pDevice, ID3D12GraphicsCommandList * pCommandList)
	:m_pDevice(pDevice) , m_pCommandList(pCommandList)
{
}

SDX12ShaderResourceManager::~SDX12ShaderResourceManager()
{
	RELEASE_DX(m_pSRVHeap);
	RELEASE_DX(m_pSRVBuffer);
}

void SDX12ShaderResourceManager::CreateShaderResource(const SModel& model)
{
	auto textures = model.GetTextures();

	if (textures.size() == 0)
		return;

	unsigned int TextureSize = model.Textures.size();
	unsigned int TextureWidth = model.TextureWidth(0), TextureHeight = model.TextureHeight(0);

	// Create Texture
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.NumDescriptors = TextureSize;
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	HRESULT hResult = m_pDevice->GetDevice()->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_pSRVHeap));
	SWindows::OutputErrorMessage(hResult);
	m_pSRVHeap->SetName(L"Shader Resource View Descriptor Heap");

	D3D12_HEAP_PROPERTIES defaultHeapProperties;
	defaultHeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	defaultHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	defaultHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	defaultHeapProperties.CreationNodeMask = 1;
	defaultHeapProperties.VisibleNodeMask = 1;

	D3D12_HEAP_PROPERTIES uploadHeapProperties = defaultHeapProperties;
	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;

	ID3D12Resource* texturebuffer = nullptr;
	D3D12_RESOURCE_DESC textureDesc = {};
	textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	textureDesc.Alignment = 0;
	textureDesc.Width = SIGraphicsInterface::c_BufferingCount * model.TextureWidth(0);
	textureDesc.Height = SIGraphicsInterface::c_BufferingCount * model.TextureHeight(0);
	textureDesc.DepthOrArraySize = 1;
	textureDesc.MipLevels = 1;
	textureDesc.Format = static_cast<DXGI_FORMAT>(model.TextureFormat(0));
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	hResult = m_pDevice->GetDevice()->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &textureDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&texturebuffer));
	SWindows::OutputErrorMessage(hResult);
	texturebuffer->SetName(L"Upload Texture2D");

	hResult = m_pDevice->GetDevice()->CreateCommittedResource(&defaultHeapProperties, D3D12_HEAP_FLAG_NONE, &textureDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&m_pSRVBuffer));
	SWindows::OutputErrorMessage(hResult);
	m_pSRVBuffer->SetName(L"Texture2D");

	D3D12_SUBRESOURCE_DATA textureData = {};
	textureData.pData = model.TextureSerialize(0);
	textureData.RowPitch = TextureWidth * model.TextureFormat(0);
	textureData.SlicePitch = textureData.RowPitch * TextureHeight;

	UpdateSubresource(m_pCommandList, m_pSRVBuffer, texturebuffer, 0, 0, 1, &textureData);

	D3D12_RESOURCE_BARRIER SRVResourceBarrier;
	SRVResourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	SRVResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	SRVResourceBarrier.Transition.pResource = m_pSRVBuffer;
	SRVResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	SRVResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	SRVResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	m_pCommandList->ResourceBarrier(1, &SRVResourceBarrier);

	// Describe and create a SRV for the texture.
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	m_pDevice->GetDevice()->CreateShaderResourceView(m_pSRVBuffer, &srvDesc, m_pSRVHeap->GetCPUDescriptorHandleForHeapStart());

	D3D12_GPU_VIRTUAL_ADDRESS srvGPUAddress = texturebuffer->GetGPUVirtualAddress();
	D3D12_CPU_DESCRIPTOR_HANDLE srvCPUHandle = m_pSRVHeap->GetCPUDescriptorHandleForHeapStart();
	m_uiSRVDescriptorSize = m_pDevice->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	for (int i = 0;i < SIGraphicsInterface::c_BufferingCount; ++i)
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC textureViewDesc = {};
		textureViewDesc.Format = static_cast<DXGI_FORMAT>(model.TextureFormat(0));
		textureViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		textureViewDesc.Shader4ComponentMapping = D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_0;
		textureViewDesc.Texture2D.MipLevels = 0;
		textureViewDesc.Texture2D.MostDetailedMip = 0;
		textureViewDesc.Texture2D.ResourceMinLODClamp = 0.0f;

		m_pDevice->GetDevice()->CreateShaderResourceView(texturebuffer, &textureViewDesc, srvCPUHandle);

		srvGPUAddress += model.TextureWidth(0) * model.TextureHeight(0);
		srvCPUHandle.ptr += m_uiSRVDescriptorSize;
	}
}

void SDX12ShaderResourceManager::BindShaderResource()
{
}
