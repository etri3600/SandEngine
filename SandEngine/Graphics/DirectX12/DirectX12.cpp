#if __WINDOWS__

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")

#include <d3d12shader.h>
#include <sstream>

#include "DirectX12.h"	
#include "SMath.h"
#include "DX12Helper.h"
#include "DX12ResourceAllocator.h"
#include "DX12DescriptorHeapAllocator.h"
#include "DX12Pipeline.h"
#include "Platform/Windows.h"

std::wostream& operator<<(std::wostream& stream, EGBuffer buffer)
{
	switch (buffer)
	{
	case EGBuffer::GB_COLOR: stream << L"COLOR"; break;
	case EGBuffer::GB_DEPTH: stream << L"DEPTH"; break;
	case EGBuffer::GB_NORMAL: stream << L"NORMAL"; break;
	}

	return stream;
}

SDirectX12::SDirectX12()
{
	memset(m_nFenceValue, 0, sizeof(m_nFenceValue));
}

SDirectX12::~SDirectX12()
{
	Finalize();
}

bool SDirectX12::Initialize(const SPlatformSystem* pPlatformSystem, unsigned int screenWidth, unsigned int screenHeight, bool fullScreen, bool vSync)
{
	HRESULT hResult;


	m_bVSync = vSync;
	m_bFullScreen = fullScreen;
	m_ScreenWidth = screenWidth;
	m_ScreenHeight = screenHeight;

	const SWindows* pWindowsSystem = static_cast<const SWindows*>(pPlatformSystem);
	if (pWindowsSystem == nullptr)
		return false;

	m_pDevice = new SDirectX12Device();
	m_pDevice->Initialize(screenWidth, screenHeight, fullScreen, vSync);


	// Create Command Queue
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc;
	commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	commandQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	commandQueueDesc.NodeMask = 0;
	hResult = m_pDevice->GetDevice()->CreateCommandQueue(&commandQueueDesc, __uuidof(ID3D12CommandQueue), (void**)&m_pCommandQueue);
	SWindows::OutputErrorMessage(hResult);

	// Create Command Allocator
	for (unsigned int i = 0;i < c_BufferingCount; ++i)
	{
		hResult = m_pDevice->GetDevice()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator), (void**)&m_pCommandAllocator[i]);
		SWindows::OutputErrorMessage(hResult);
	}

	hResult = m_pDevice->GetDevice()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_BUNDLE, __uuidof(ID3D12CommandAllocator), (void**)&m_pBundleAllocator);
	SWindows::OutputErrorMessage(hResult);

	// Create Fence
	hResult = m_pDevice->GetDevice()->CreateFence(m_nFenceValue[m_BufferIndex], D3D12_FENCE_FLAG_NONE, __uuidof(ID3D12Fence), (void**)&m_pFence);
	SWindows::OutputErrorMessage(hResult);
	++m_nFenceValue[m_BufferIndex];

	m_hFenceEvent = CreateEventEx(nullptr, FALSE, FALSE, EVENT_ALL_ACCESS);

	CreateSwapChain(pPlatformSystem, m_pDevice->GetDeviceNumerator(), m_pDevice->GetDeviceDenominator());
	
	// Create Shader buffer
	D3D12_DESCRIPTOR_HEAP_DESC cbheapDesc = {};
	cbheapDesc.NumDescriptors = c_NumDescriptorsPerHeap;
	cbheapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	cbheapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	hResult = m_pDevice->GetDevice()->CreateDescriptorHeap(&cbheapDesc, IID_PPV_ARGS(&m_pShaderBufferHeap));
	SWindows::OutputErrorMessage(hResult);
	m_pShaderBufferHeap->SetName(L"Shader Buffer Descriptor Heap");
	m_uiShaderBufferDescriptorSize = m_pDevice->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	CreateGBuffers();
	CreatePostProcessResources();

	std::pair<std::map<EMaterialType, SDX12Pipeline*>::iterator, bool> pipeIt;

	// Create PSO for GBuffers
	DXGI_FORMAT rtvFormats[static_cast<unsigned int>(EGBuffer::GB_NUM)] = { DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_R16G16B16A16_FLOAT };

	m_RootSignatures.push_back(new SDX12RootSignature(m_pDevice, new SDX12BoneRootParameter()));
	m_pipelines.push_back(new SDX12Pipeline(m_pDevice, m_RootSignatures.back(), EMaterialType::SKINNING));
	m_pipelines.back()->Init(L"SkinnedShader", L"SkinnedShader", "vert", "frag", new SDX12SkinnedResources(), 3, rtvFormats);

	m_RootSignatures.push_back(new SDX12RootSignature(m_pDevice, new SDX12TextureRootParameter()));
	m_pipelines.push_back(new SDX12Pipeline(m_pDevice, m_RootSignatures.back(), EMaterialType::TEXTURE));
	m_pipelines.back()->Init(L"TextureVertexShader", L"TexturePixelShader", "main", "main", new SDX12TextureResources(), 3, rtvFormats);

	// Create PSO for Light Pass


	// Create PSO for Deferred Pass
	DXGI_FORMAT deferredFormats[1] = { DXGI_FORMAT_R8G8B8A8_UNORM };
	m_RootSignatures.push_back(new SDX12RootSignature(m_pDevice, new SDX12SsAoRootParameter()));

	SDX12Pipeline* deferredPipeline = new SDX12Pipeline(m_pDevice, m_RootSignatures.back(), EMaterialType::DEFERRED);
	deferredPipeline->Init(L"DeferredLighting", L"DeferredLighting", "vert", "frag", new SDX12DeferredResources(), 1, deferredFormats);
	m_pipelines.push_back(deferredPipeline);

	// Create PSO for Postprocess
	DXGI_FORMAT postprocessFormats[1] = { DXGI_FORMAT_R16_FLOAT };
	m_RootSignatures.push_back(new SDX12RootSignature(m_pDevice, new SDX12SsAoRootParameter()));

	SDX12Pipeline* postprocessPipeline = new SDX12Pipeline(m_pDevice, m_RootSignatures.back(), EMaterialType::POSTPROCESS);
	postprocessPipeline->Init(L"SsAo", L"SsAo", "vert", "frag", new SDX12SsAoResources(), 1, postprocessFormats);
	m_pipelines.push_back(postprocessPipeline);

	// Create Command List
	hResult = m_pDevice->GetDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, GetCommandAllocator(), nullptr, IID_PPV_ARGS(&m_pCommandList));
	SWindows::OutputErrorMessage(hResult);

	hResult = m_pDevice->GetDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_BUNDLE, m_pBundleAllocator, nullptr, IID_PPV_ARGS(&m_pBundleList));
	SWindows::OutputErrorMessage(hResult);

	// Create Sampler
	D3D12_DESCRIPTOR_HEAP_DESC descHeapSampler = {};
	descHeapSampler.NumDescriptors = 1;
	descHeapSampler.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	descHeapSampler.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
	hResult = m_pDevice->GetDevice()->CreateDescriptorHeap(&descHeapSampler, IID_PPV_ARGS(&m_pSamplerHeap));
	SWindows::OutputErrorMessage(hResult);

	D3D12_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	m_pDevice->GetDevice()->CreateSampler(&samplerDesc, m_pSamplerHeap->GetCPUDescriptorHandleForHeapStart());

	InitBundle();

	m_pResourceAllocator[0] = new SDX12ResourceAllocator(m_pDevice, m_pCommandList, HEAP_DEFAULT);
	m_pResourceAllocator[1] = new SDX12ResourceAllocator(m_pDevice, m_pCommandList, HEAP_UPLOAD);

	for (unsigned int i=0;i<D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES;++i)
	{
		m_pDescriptorAllocator[i] = new SDX12DescriptorHeapAllocator(m_pDevice, static_cast<D3D12_DESCRIPTOR_HEAP_TYPE>(i));
	}

	return true;
}

void SDirectX12::Finalize()
{
	WaitForIdle();

	for (auto& pipeline : m_pipelines)
	{
		delete pipeline;
	}
	m_pipelines.clear();

	if (m_pVertexBuffer)
		m_pVertexBuffer->Release();
	
	if (m_pIndexBuffer)
		m_pIndexBuffer->Release();

	if (m_pSwapChain)
		m_pSwapChain->SetFullscreenState(false, nullptr);

	CloseHandle(m_hFenceEvent);

	if (m_pFence)
		m_pFence->Release();

	if (m_pBundleList)
		m_pBundleList->Release();

	if (m_pCommandList)
		m_pCommandList->Release();

	for (int i = 0;i < c_BufferingCount; ++i)
	{
		if (m_pBackBufferRenterTarget[i])
			m_pBackBufferRenterTarget[i]->Release();

		if (m_pCommandAllocator[i])
			m_pCommandAllocator[i]->Release();
	}

	if (m_pShaderBufferHeap)
		m_pShaderBufferHeap->Release();

	if (m_pGBufferRTVHeap)
		m_pGBufferRTVHeap->Release();

	if (m_pGBufferCbvSrvHeap)
		m_pGBufferCbvSrvHeap->Release();

	if (m_pRenderTargetViewHeap)
		m_pRenderTargetViewHeap->Release();

	if (m_pSwapChain)
		m_pSwapChain->Release();
	
	if (m_pCommandQueue)
		m_pCommandQueue->Release();
	
	if (m_pDevice)
	{
		delete m_pDevice;
		m_pDevice = nullptr;
	}
}

bool SDirectX12::CreateSwapChain(const SPlatformSystem* pPlatformSystem, const int nNumerator, const int nDenominator)
{
	const SWindows* pWindowsSystem = static_cast<const SWindows*>(pPlatformSystem);
	if (pWindowsSystem == nullptr)
		return false;

	for (int i = 0;i < c_BufferingCount; ++i)
	{
		m_pBackBufferRenterTarget[i] = nullptr;
	}
	m_pRenderTargetViewHeap = nullptr;

	if (m_pSwapChain)
	{
		m_pSwapChain->ResizeBuffers(c_BufferingCount, m_ScreenWidth, m_ScreenHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
	}
	else
	{
		DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
		swapChainDesc.BufferCount = c_BufferingCount;
		swapChainDesc.BufferDesc.Width = m_ScreenWidth;
		swapChainDesc.BufferDesc.Height = m_ScreenHeight;
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.OutputWindow = pWindowsSystem->GetWindowHandle();
		swapChainDesc.Windowed = !m_bFullScreen;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = m_bVSync ? nDenominator : 1;
		swapChainDesc.BufferDesc.RefreshRate.Numerator = m_bVSync ? nNumerator : 0;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.Flags = 0;

		IDXGISwapChain* swapChain = nullptr;
		HRESULT hResult = m_pDevice->GetDXGIFactory()->CreateSwapChain(m_pCommandQueue, &swapChainDesc, &swapChain);
		SWindows::OutputErrorMessage(hResult);

		hResult = swapChain->QueryInterface(__uuidof(IDXGISwapChain), (void**)&m_pSwapChain);
		SWindows::OutputErrorMessage(hResult);
	}

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.NumDescriptors = c_BufferingCount;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	m_pDevice->GetDevice()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_pRenderTargetViewHeap));

	for (unsigned int i = 0;i < c_BufferingCount; ++i)
	{
		m_nFenceValue[i] = m_nFenceValue[m_BufferIndex];
	}
	m_BufferIndex = 0;

	D3D12_CPU_DESCRIPTOR_HANDLE RTVDescriptor = m_pRenderTargetViewHeap->GetCPUDescriptorHandleForHeapStart();
	m_RenderTargetViewDescriptorSize = m_pDevice->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	for (unsigned int n = 0; n < c_BufferingCount; n++)
	{
		m_pSwapChain->GetBuffer(n, IID_PPV_ARGS(&m_pBackBufferRenterTarget[n]));
		m_pDevice->GetDevice()->CreateRenderTargetView(m_pBackBufferRenterTarget[n], nullptr, RTVDescriptor);
		RTVDescriptor.ptr += m_RenderTargetViewDescriptorSize;
	}

	m_Viewport = { 0.0f, 0.0f, static_cast<float>(m_ScreenWidth), static_cast<float>(m_ScreenHeight), 0.0f, 1.0f };

	return true;
}

void SDirectX12::CreateViewProjection()
{
	float aspectRatio = static_cast<float>(m_ScreenWidth) / static_cast<float>(m_ScreenHeight);
	float fovAngleY = static_cast<float>(70.0f * S_PI) / 180.0f;

	if (aspectRatio < 1.0f)
	{
		fovAngleY *= 2.0f;
	}

	SMatrix perspectiveMatrix = MatrixPerspectiveFOV(
		fovAngleY,
		aspectRatio,
		0.01f,
		1000.0f
		);

	Projection = perspectiveMatrix;

	const SVector3 eye = { 0.0f, 0.0f, 15.f };
	const SVector3 at = { 0.0f, 0.0f, -1.0f };
	const SVector3 up = { 0.0f, 1.0f, 0.0f };

	View = MatrixLookAt(eye, at, up);
}

void SDirectX12::UpdateBoneTransform(const std::map<EMaterialType, std::vector<SSceneObj*>>& material_obj_map)
{
	for (auto it = material_obj_map.begin(); it != material_obj_map.end(); ++it)
	{
		for (unsigned int i = 0; i < it->second.size(); ++i)
		{
			auto model = dynamic_cast<SModel*>(it->second[i]);
			if (model)
			{
				if (model->HasAnimation())
				{
					const auto& boneTransforms = model->GetFinalTransform();
					for (unsigned int j = 0; j < std::fmin(MAX_BONES, boneTransforms.size()); ++j)
					{
						m_SceneProxy.BatchProxies[it->first].ObjectProxies[i].BoneTransform[j] = boneTransforms[j];
					}
				}
			}
		}
	}
}

void SDirectX12::Reset()
{
	for (auto& proxy : m_SceneProxy.BatchProxies)
	{
		proxy.second.ObjectProxies.clear();
	} 
	m_SceneProxy.BatchProxies.clear();
}

bool SDirectX12::Update(const double delta, std::map<EMaterialType, std::vector<SSceneObj*>>& objs)
{
	for (auto it = objs.begin(); it != objs.end(); ++it)
	{
		unsigned int oldSize = m_SceneProxy.BatchProxies[it->first].ObjectProxies.size();
		m_SceneProxy.BatchProxies[it->first].ObjectProxies.resize(oldSize + objs.size());

		for (unsigned int i = oldSize; i < oldSize + objs.size(); ++i)
		{
			auto model = static_cast<SModel*>(it->second[i - oldSize]);
			if (model)
			{
				m_SceneProxy.BatchProxies[it->first].ObjectProxies[i].MeshProxy = model->MeshInfoes;
				m_SceneProxy.BatchProxies[it->first].ObjectProxies[i].Tranformation = SMath::Transform(model->Scale, model->Location, model->Rotation);
				m_SceneProxy.BatchProxies[it->first].ObjectProxies[i].Vertices = model->Vertices;
				m_SceneProxy.BatchProxies[it->first].ObjectProxies[i].Indices = model->Indices;
				m_SceneProxy.BatchProxies[it->first].ObjectProxies[i].BaseVertexLocation = m_SceneProxy.BatchProxies[it->first].VertexSize / sizeof(SSkinnedModelVertex);
				m_SceneProxy.BatchProxies[it->first].ObjectProxies[i].VertexSize = static_cast<unsigned int>(model->VertexSize());
				m_SceneProxy.BatchProxies[it->first].ObjectProxies[i].StartIndexLocation = m_SceneProxy.BatchProxies[it->first].IndexSize / sizeof(unsigned int);
				m_SceneProxy.BatchProxies[it->first].ObjectProxies[i].IndexSize = static_cast<unsigned int>(model->IndexSize());
				m_SceneProxy.BatchProxies[it->first].ObjectProxies[i].Textures = model->GetTextures();
				m_SceneProxy.BatchProxies[it->first].VertexSize += m_SceneProxy.BatchProxies[it->first].ObjectProxies[i].VertexSize;
				m_SceneProxy.BatchProxies[it->first].IndexSize += m_SceneProxy.BatchProxies[it->first].ObjectProxies[i].IndexSize;

				for (auto boneTransform : m_SceneProxy.BatchProxies[it->first].ObjectProxies[i].BoneTransform)
				{
					memcpy(m_SceneProxy.BatchProxies[it->first].ObjectProxies[i].BoneTransform, &SMatrix::Identity, sizeof(SMatrix));
				}
			}
		}
	}

	return true;
}

void SDirectX12::Draw()
{
	HRESULT hResult;
	// Setting Default, Upload Heap
	D3D12_HEAP_PROPERTIES defaultHeapProperties;
	defaultHeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	defaultHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	defaultHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	defaultHeapProperties.CreationNodeMask = 1;
	defaultHeapProperties.VisibleNodeMask = 1;

	D3D12_HEAP_PROPERTIES uploadHeapProperties = defaultHeapProperties;
	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;

	for (auto& pipeline : m_pipelines)
	{	
		auto it = m_SceneProxy.BatchProxies.find(pipeline->GetMaterialType());
		if (it != m_SceneProxy.BatchProxies.end())
		{
			SBatchProxy* pBatchProxy = &it->second;

			std::vector<SSkinnedModelVertex> vertices;
			std::vector<unsigned int> indices;
			for (unsigned int i = 0; i < pBatchProxy->ObjectProxies.size(); ++i)
			{
				vertices.insert(std::end(vertices), std::begin(pBatchProxy->ObjectProxies[i].Vertices), std::end(pBatchProxy->ObjectProxies[i].Vertices));
				indices.insert(std::end(indices), std::begin(pBatchProxy->ObjectProxies[i].Indices), std::end(pBatchProxy->ObjectProxies[i].Indices));
			}

			// Vertex Buffer
			ID3D12Resource* vertexBufferUpload = nullptr;

			D3D12_RESOURCE_DESC vertexBufferDesc;
			vertexBufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
			vertexBufferDesc.Alignment = 0;
			vertexBufferDesc.Width = pBatchProxy->VertexSize;
			vertexBufferDesc.Height = 1;
			vertexBufferDesc.DepthOrArraySize = 1;
			vertexBufferDesc.MipLevels = 1;
			vertexBufferDesc.Format = DXGI_FORMAT_UNKNOWN;
			vertexBufferDesc.SampleDesc.Count = 1;
			vertexBufferDesc.SampleDesc.Quality = 0;
			vertexBufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
			vertexBufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

			hResult = m_pDevice->GetDevice()->CreateCommittedResource(&defaultHeapProperties, D3D12_HEAP_FLAG_NONE, &vertexBufferDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&m_pVertexBuffer));
			SWindows::OutputErrorMessage(hResult);
			m_pVertexBuffer->SetName(L"VertexBuffer");

			hResult = m_pDevice->GetDevice()->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &vertexBufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&vertexBufferUpload));
			SWindows::OutputErrorMessage(hResult);

			vertexBufferUpload->SetName(L"VertexBufferUpload");

			D3D12_SUBRESOURCE_DATA vertexData = {};
			vertexData.pData = vertices.data();
			vertexData.RowPitch = vertices.size() * sizeof(SSkinnedModelVertex);
			vertexData.SlicePitch = vertexData.RowPitch;

			UpdateSubresource(m_pCommandList, m_pVertexBuffer, vertexBufferUpload, 0, 0, 1, &vertexData);

			D3D12_RESOURCE_BARRIER vertexBufferResourceBarrier;
			vertexBufferResourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			vertexBufferResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			vertexBufferResourceBarrier.Transition.pResource = m_pVertexBuffer;
			vertexBufferResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
			vertexBufferResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
			vertexBufferResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

			m_pCommandList->ResourceBarrier(1, &vertexBufferResourceBarrier);


			// Index Buffer
			ID3D12Resource* indexbufferUpload = nullptr;
			D3D12_RESOURCE_DESC indexBufferDesc;
			indexBufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
			indexBufferDesc.Alignment = 0;
			indexBufferDesc.Width = pBatchProxy->IndexSize;
			indexBufferDesc.Height = 1;
			indexBufferDesc.DepthOrArraySize = 1;
			indexBufferDesc.MipLevels = 1;
			indexBufferDesc.Format = DXGI_FORMAT_UNKNOWN;
			indexBufferDesc.SampleDesc.Count = 1;
			indexBufferDesc.SampleDesc.Quality = 0;
			indexBufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
			indexBufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

			hResult = m_pDevice->GetDevice()->CreateCommittedResource(&defaultHeapProperties, D3D12_HEAP_FLAG_NONE, &indexBufferDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&m_pIndexBuffer));
			SWindows::OutputErrorMessage(hResult);
			m_pIndexBuffer->SetName(L"IndexBuffer");

			hResult = m_pDevice->GetDevice()->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &indexBufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&indexbufferUpload));
			SWindows::OutputErrorMessage(hResult);

			indexbufferUpload->SetName(L"IndexBufferUpload");

			D3D12_SUBRESOURCE_DATA indexData = {};
			indexData.pData = indices.data();
			indexData.RowPitch = indices.size() * sizeof(unsigned int);
			indexData.SlicePitch = indexData.RowPitch;

			UpdateSubresource(m_pCommandList, m_pIndexBuffer, indexbufferUpload, 0, 0, 1, &indexData);


			D3D12_RESOURCE_BARRIER indexBufferResourceBarrier;
			indexBufferResourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			indexBufferResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			indexBufferResourceBarrier.Transition.pResource = m_pIndexBuffer;
			indexBufferResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
			indexBufferResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_INDEX_BUFFER;
			indexBufferResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

			m_pCommandList->ResourceBarrier(1, &indexBufferResourceBarrier);

			pipeline->SetVertexBufferView(m_pVertexBuffer->GetGPUVirtualAddress(), pBatchProxy->VertexSize, sizeof(SSkinnedModelVertex));
			pipeline->SetIndexBufferView(m_pIndexBuffer->GetGPUVirtualAddress(), pBatchProxy->IndexSize, DXGI_FORMAT_R32_UINT);
		}
	}

	unsigned int descriptorOffset = 0;
	for (auto it = m_pipelines.begin(); it != m_pipelines.end(); ++it)
	{
		EMaterialType mat_type = (*it)->GetMaterialType();
		auto batchProxyIter = m_SceneProxy.BatchProxies.find(mat_type);
		// gbuffers
		if (batchProxyIter != m_SceneProxy.BatchProxies.end())
		{
			(*it)->CreateConstantBuffer(m_pShaderBufferHeap, descriptorOffset, m_uiShaderBufferDescriptorSize, &batchProxyIter->second);
			descriptorOffset += (*it)->GetCBVDescriptorOffset();
			descriptorOffset += (*it)->CreateShaderResources(m_pCommandList, m_pShaderBufferHeap, &batchProxyIter->second, descriptorOffset);
		}
		// others
		else
		{
			(*it)->CreateConstantBuffer(m_pGBufferCbvSrvHeap, m_RenderTargetViewDescriptorSize * 3, m_uiShaderBufferDescriptorSize, nullptr);
		}
	}

	// Execute Command
	m_pCommandList->Close();
	ID3D12CommandList* ppCommandList[] = { m_pCommandList };
	m_pCommandQueue->ExecuteCommandLists(sizeof(ppCommandList) / sizeof(ppCommandList[0]), ppCommandList);
}

bool SDirectX12::Render()
{
	HRESULT hResult = GetCommandAllocator()->Reset();
	SWindows::OutputErrorMessage(hResult);

	hResult = m_pCommandList->Reset(GetCommandAllocator(), nullptr);
	SWindows::OutputErrorMessage(hResult);

	// GBuffer
	m_pCommandList->RSSetViewports(1, &m_Viewport);
	D3D12_RECT ScissorRect = { 0L, 0L, static_cast<long>(m_Viewport.Width), static_cast<long>(m_Viewport.Height) };
	m_pCommandList->RSSetScissorRects(1, &ScissorRect);

	D3D12_RESOURCE_BARRIER gBufferResourceBarriers[static_cast<unsigned int>(EGBuffer::GB_NUM)] = {};
	gBufferResourceBarriers[0].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	gBufferResourceBarriers[0].Transition.pResource = m_pGBuffers[static_cast<unsigned short>(EGBuffer::GB_COLOR)];
	gBufferResourceBarriers[0].Transition.StateBefore = D3D12_RESOURCE_STATE_GENERIC_READ;
	gBufferResourceBarriers[0].Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	gBufferResourceBarriers[0].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	gBufferResourceBarriers[0].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;

	gBufferResourceBarriers[1].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	gBufferResourceBarriers[1].Transition.pResource = m_pGBuffers[static_cast<unsigned short>(EGBuffer::GB_DEPTH)];
	gBufferResourceBarriers[1].Transition.StateBefore = D3D12_RESOURCE_STATE_GENERIC_READ;
	gBufferResourceBarriers[1].Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	gBufferResourceBarriers[1].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	gBufferResourceBarriers[1].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;

	gBufferResourceBarriers[2].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	gBufferResourceBarriers[2].Transition.pResource = m_pGBuffers[static_cast<unsigned short>(EGBuffer::GB_NORMAL)];
	gBufferResourceBarriers[2].Transition.StateBefore = D3D12_RESOURCE_STATE_GENERIC_READ;
	gBufferResourceBarriers[2].Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	gBufferResourceBarriers[2].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	gBufferResourceBarriers[2].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	m_pCommandList->ResourceBarrier(static_cast<UINT>(EGBuffer::GB_NUM), gBufferResourceBarriers);

	const float color[4] = { 0.1f, 0.5f, 0.7f, 1.0f };
	D3D12_CPU_DESCRIPTOR_HANDLE renderTargetViewHandle;
	renderTargetViewHandle.ptr = m_pGBufferRTVHeap->GetCPUDescriptorHandleForHeapStart().ptr;
	
	for (int i = 0; i < static_cast<int>(EGBuffer::GB_NUM); ++i)
	{
		D3D12_CPU_DESCRIPTOR_HANDLE handle;
		handle.ptr = renderTargetViewHandle.ptr + m_RenderTargetViewDescriptorSize * i;
		m_pCommandList->ClearRenderTargetView(handle, color, 0, nullptr);
	}
	m_pCommandList->OMSetRenderTargets(static_cast<UINT>(EGBuffer::GB_NUM), &renderTargetViewHandle, true, nullptr);
	m_pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	
	ID3D12DescriptorHeap* ppHeaps[] = { m_pShaderBufferHeap };
	m_pCommandList->SetDescriptorHeaps(sizeof(ppHeaps) / sizeof(ppHeaps[0]), ppHeaps);
	for (auto& pipeline : m_pipelines)
	{
		if (pipeline && pipeline->GetMaterialType() < EMaterialType::DEFERRED)
		{
			SBatchProxy* batchProxy = &m_SceneProxy.BatchProxies[pipeline->GetMaterialType()];
			auto signature = pipeline->GetRootSignature();

			m_pCommandList->IASetVertexBuffers(0, 1, &pipeline->GetVertexBufferView());
			m_pCommandList->IASetIndexBuffer(&pipeline->GetIndexBufferView());
			m_pCommandList->SetGraphicsRootSignature(pipeline->GetRootSignature()->Get());
			m_pCommandList->SetPipelineState(pipeline->GetPipelineState());
			{
				D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = m_pShaderBufferHeap->GetGPUDescriptorHandleForHeapStart();
				for (unsigned int i = 0; i < batchProxy->ObjectProxies.size(); ++i)
				{
					gpuHandle.ptr += pipeline->GetCBVDescriptorOffset();
					pipeline->Populate(m_pCommandList, i);
					pipeline->UpdateConstantBuffer(batchProxy, i, View, Projection);
					for (unsigned int j = 0; j < batchProxy->ObjectProxies[i].MeshProxy.size(); ++j)
					{
						m_pCommandList->SetGraphicsRootDescriptorTable(signature->GetTableLocation(), gpuHandle);
						m_pCommandList->DrawIndexedInstanced(batchProxy->ObjectProxies[i].MeshProxy[j].NumIndices, 1, batchProxy->ObjectProxies[i].StartIndexLocation + batchProxy->ObjectProxies[i].MeshProxy[j].BaseIndex, batchProxy->ObjectProxies[i].BaseVertexLocation + batchProxy->ObjectProxies[i].MeshProxy[j].BaseVertex, 0);
						gpuHandle.ptr += m_uiShaderBufferDescriptorSize;
					}
				}
			}
		}
	}

	D3D12_RESOURCE_BARRIER gbuffersResourceBarriers[static_cast<unsigned int>(EGBuffer::GB_NUM)] = {};
	gbuffersResourceBarriers[0].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	gbuffersResourceBarriers[0].Transition.pResource = m_pGBuffers[static_cast<unsigned short>(EGBuffer::GB_COLOR)];
	gbuffersResourceBarriers[0].Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	gbuffersResourceBarriers[0].Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;
	gbuffersResourceBarriers[0].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	gbuffersResourceBarriers[0].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;

	gbuffersResourceBarriers[1].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	gbuffersResourceBarriers[1].Transition.pResource = m_pGBuffers[static_cast<unsigned short>(EGBuffer::GB_DEPTH)];
	gbuffersResourceBarriers[1].Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	gbuffersResourceBarriers[1].Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;
	gbuffersResourceBarriers[1].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	gbuffersResourceBarriers[1].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;

	gbuffersResourceBarriers[2].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	gbuffersResourceBarriers[2].Transition.pResource = m_pGBuffers[static_cast<unsigned short>(EGBuffer::GB_NORMAL)];
	gbuffersResourceBarriers[2].Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	gbuffersResourceBarriers[2].Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;
	gbuffersResourceBarriers[2].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	gbuffersResourceBarriers[2].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	m_pCommandList->ResourceBarrier(static_cast<UINT>(EGBuffer::GB_NUM), gbuffersResourceBarriers);

	// Light and Render
	D3D12_RESOURCE_BARRIER swapResourceBarrier;
	swapResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	swapResourceBarrier.Transition.pResource = GetRenderTarget();
	swapResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_GENERIC_READ;
	swapResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	swapResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	swapResourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;

	float backColor[4] = { 0, 1.0f, 1.0f, 1.0f };
	renderTargetViewHandle.ptr = m_pRenderTargetViewHeap->GetCPUDescriptorHandleForHeapStart().ptr + m_BufferIndex * m_RenderTargetViewDescriptorSize;
	m_pCommandList->ClearRenderTargetView(renderTargetViewHandle, backColor, 0, nullptr);
	m_pCommandList->OMSetRenderTargets(1, &renderTargetViewHandle, false, nullptr);
	m_pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	ID3D12DescriptorHeap* ppDeferredHeaps[] = { m_pGBufferCbvSrvHeap };
	m_pCommandList->SetDescriptorHeaps(sizeof(ppDeferredHeaps) / sizeof(ppDeferredHeaps[0]), ppDeferredHeaps);
	D3D12_GPU_DESCRIPTOR_HANDLE gpuDeferredHandle = m_pGBufferCbvSrvHeap->GetGPUDescriptorHandleForHeapStart();

	for (auto& pipeline : m_pipelines)
	{
		if (pipeline)
		{
			if (pipeline->GetMaterialType() == EMaterialType::DEFERRED)
			{
				auto signature = pipeline->GetRootSignature();
				m_pCommandList->IASetVertexBuffers(0, 1, &pipeline->GetVertexBufferView());
				m_pCommandList->IASetIndexBuffer(&pipeline->GetIndexBufferView());
				m_pCommandList->SetGraphicsRootSignature(pipeline->GetRootSignature()->Get());
				m_pCommandList->SetPipelineState(pipeline->GetPipelineState());				

				D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = m_pGBufferCbvSrvHeap->GetGPUDescriptorHandleForHeapStart();
				gpuHandle.ptr += pipeline->GetCBVDescriptorOffset();

				m_pCommandList->SetGraphicsRootDescriptorTable(signature->GetTableLocation(), gpuHandle);
				m_pCommandList->DrawInstanced(6, 1, 0, 0);
				gpuHandle.ptr += m_uiShaderBufferDescriptorSize;
			}
		}
	}

	swapResourceBarrier.Transition.pResource = GetRenderTarget();
	swapResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	swapResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;
	m_pCommandList->ResourceBarrier(1, &swapResourceBarrier);

	// PostProcess
	//m_pCommandList->RSSetViewports(1, &m_Viewport);
	//m_pCommandList->RSSetScissorRects(1, &ScissorRect);

	D3D12_RESOURCE_BARRIER postprocessResourceBarrier;
	postprocessResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	postprocessResourceBarrier.Transition.pResource = m_pPostProcessResource;
	postprocessResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	postprocessResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	postprocessResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	postprocessResourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	m_pCommandList->ResourceBarrier(1, &postprocessResourceBarrier);

	D3D12_CPU_DESCRIPTOR_HANDLE postprocessRTVHandle;
	postprocessRTVHandle.ptr = m_pPostProcessRTVHeap->GetCPUDescriptorHandleForHeapStart().ptr;
	m_pCommandList->ClearRenderTargetView(postprocessRTVHandle, color, 0, nullptr);
	m_pCommandList->OMSetRenderTargets(1, &postprocessRTVHandle, true, nullptr);
	m_pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	ID3D12DescriptorHeap* ppPostprocessHeaps[] = { m_pPostProcessCbvSrvHeap };
	m_pCommandList->SetDescriptorHeaps(sizeof(ppPostprocessHeaps) / sizeof(ppPostprocessHeaps[0]), ppPostprocessHeaps);
	D3D12_GPU_DESCRIPTOR_HANDLE gpuPostprocessHandle = m_pPostProcessCbvSrvHeap->GetGPUDescriptorHandleForHeapStart();

	m_pCommandList->IASetVertexBuffers(0, 0, nullptr);
	m_pCommandList->IASetIndexBuffer(nullptr);
	m_pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	for (auto& pipeline : m_pipelines)
	{
		if (pipeline && pipeline->GetMaterialType() == EMaterialType::POSTPROCESS)
		{
			gpuPostprocessHandle.ptr += pipeline->GetCBVDescriptorOffset();
			m_pCommandList->SetGraphicsRootSignature(pipeline->GetRootSignature()->Get());
			m_pCommandList->SetPipelineState(pipeline->GetPipelineState());
			pipeline->Populate(m_pCommandList, 0);
			pipeline->UpdateConstantBuffer(nullptr, 0, View, Projection);
			m_pCommandList->SetGraphicsRootDescriptorTable(pipeline->GetRootSignature()->GetTableLocation(), gpuPostprocessHandle);
			m_pCommandList->DrawInstanced(6, 1, 0, 0);
		}
	}

	postprocessResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	postprocessResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	m_pCommandList->ResourceBarrier(1, &postprocessResourceBarrier);

	m_pCommandList->Close();
	ID3D12CommandList* ppCommandLists[] = { m_pCommandList };
	m_pCommandQueue->ExecuteCommandLists(sizeof(ppCommandLists) / sizeof(ppCommandLists[0]), ppCommandLists);

	return true;
}

void SDirectX12::Present()
{
	m_pSwapChain->Present(1, 0);

	const auto currentFenceValue = m_nFenceValue[m_BufferIndex];
	HRESULT hResult = m_pCommandQueue->Signal(m_pFence, currentFenceValue);
	
	SWindows::OutputErrorMessage(hResult);

	m_BufferIndex = (m_BufferIndex + 1) % c_BufferingCount;
	UINT64 completedValue = m_pFence->GetCompletedValue();
	if (completedValue < m_nFenceValue[m_BufferIndex])
	{
		WaitForGPU(m_nFenceValue[m_BufferIndex]);
	}

	m_nFenceValue[m_BufferIndex] = currentFenceValue + 1;

	m_pResourceAllocator[0]->CleanUp();
	m_pResourceAllocator[1]->CleanUp();
}

void SDirectX12::CreateGBuffers()
{
	HRESULT hResult;

	D3D12_HEAP_PROPERTIES defaultHeapProperties;
	defaultHeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	defaultHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	defaultHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	defaultHeapProperties.CreationNodeMask = 1;
	defaultHeapProperties.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC gbufferDesc = {};
	gbufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	gbufferDesc.Alignment = 0;
	gbufferDesc.Width = m_pDevice->m_ScreenWidth;
	gbufferDesc.Height = m_pDevice->m_ScreenHeight;
	gbufferDesc.DepthOrArraySize = 1;
	gbufferDesc.MipLevels = 1;
	gbufferDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	gbufferDesc.SampleDesc.Count = 1;
	gbufferDesc.SampleDesc.Quality = 0;
	gbufferDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	gbufferDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	D3D12_CLEAR_VALUE clearValue;
	clearValue.Color[0] = 0.1f;
	clearValue.Color[1] = 0.5f;
	clearValue.Color[2] = 0.7f;
	clearValue.Color[3] = 1.0f;
	// todo format
	for (auto i = 0; i < static_cast<unsigned short>(EGBuffer::GB_NUM); ++i)
	{
		clearValue.Format = gbufferDesc.Format;
		hResult = m_pDevice->GetDevice()->CreateCommittedResource(&defaultHeapProperties, D3D12_HEAP_FLAG_NONE, &gbufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, &clearValue, IID_PPV_ARGS(&m_pGBuffers[i]));
		
		std::wostringstream name;
		name << L"GBuffer " << static_cast<EGBuffer>(i);
		m_pGBuffers[i]->SetName(name.str().c_str());
	}


	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	heapDesc.NumDescriptors = static_cast<UINT>(EGBuffer::GB_NUM);
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	m_pGBufferRTVHeap = nullptr;
	m_pDevice->GetDevice()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_pGBufferRTVHeap));

	D3D12_RENDER_TARGET_VIEW_DESC rtDesc = {};
	rtDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	rtDesc.Texture2D.MipSlice = 0;
	rtDesc.Texture2D.PlaneSlice = 0;
	rtDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;

	D3D12_CPU_DESCRIPTOR_HANDLE rtvCpuHandle = m_pGBufferRTVHeap->GetCPUDescriptorHandleForHeapStart();
	for (int i = 0; i < static_cast<int>(EGBuffer::GB_NUM); ++i)
	{
		m_pDevice->GetDevice()->CreateRenderTargetView(m_pGBuffers[i], &rtDesc, rtvCpuHandle);
		rtvCpuHandle.ptr += m_RenderTargetViewDescriptorSize;
	}
	
	m_pGBufferCbvSrvHeap = nullptr;
	heapDesc.NumDescriptors = c_NumDescriptorsPerHeap;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	m_pDevice->GetDevice()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_pGBufferCbvSrvHeap));

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Texture2D.MipLevels = gbufferDesc.MipLevels;
	srvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	D3D12_CPU_DESCRIPTOR_HANDLE srvCpuHandle = m_pGBufferCbvSrvHeap->GetCPUDescriptorHandleForHeapStart();
	for (int i = 0; i < static_cast<int>(EGBuffer::GB_NUM); ++i) {
		m_pDevice->GetDevice()->CreateShaderResourceView(m_pGBuffers[i], &srvDesc, srvCpuHandle);
		srvCpuHandle.ptr += m_uiShaderBufferDescriptorSize;
	}
}

void SDirectX12::CreatePostProcessResources()
{
	D3D12_HEAP_PROPERTIES defaultHeapProperties;
	defaultHeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	defaultHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	defaultHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	defaultHeapProperties.CreationNodeMask = 1;
	defaultHeapProperties.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC prostprocessDesc = {};
	prostprocessDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	prostprocessDesc.Alignment = 0;
	prostprocessDesc.Width = m_pDevice->m_ScreenWidth;
	prostprocessDesc.Height = m_pDevice->m_ScreenHeight;
	prostprocessDesc.DepthOrArraySize = 1;
	prostprocessDesc.MipLevels = 1;
	prostprocessDesc.Format = DXGI_FORMAT_R16_FLOAT;
	prostprocessDesc.SampleDesc.Count = 1;
	prostprocessDesc.SampleDesc.Quality = 0;
	prostprocessDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	prostprocessDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	HRESULT hResult = m_pDevice->GetDevice()->CreateCommittedResource(&defaultHeapProperties, D3D12_HEAP_FLAG_NONE, &prostprocessDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_pPostProcessResource));
	m_pPostProcessResource->SetName(L"PostProcess");

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	heapDesc.NumDescriptors = 1;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	m_pPostProcessRTVHeap = nullptr;
	hResult = m_pDevice->GetDevice()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_pPostProcessRTVHeap));

	D3D12_RENDER_TARGET_VIEW_DESC rtDesc = {};
	rtDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	rtDesc.Texture2D.MipSlice = 0;
	rtDesc.Texture2D.PlaneSlice = 0;
	rtDesc.Format = DXGI_FORMAT_R16_FLOAT;

	D3D12_CPU_DESCRIPTOR_HANDLE rtvCpuHandle = m_pPostProcessRTVHeap->GetCPUDescriptorHandleForHeapStart();
	m_pDevice->GetDevice()->CreateRenderTargetView(m_pPostProcessResource, &rtDesc, rtvCpuHandle);

	m_pPostProcessCbvSrvHeap = nullptr;
	heapDesc.NumDescriptors = c_NumDescriptorsPerHeap;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	hResult = m_pDevice->GetDevice()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_pPostProcessCbvSrvHeap));

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Texture2D.MipLevels = prostprocessDesc.MipLevels;
	srvDesc.Format = DXGI_FORMAT_R16_FLOAT;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	D3D12_CPU_DESCRIPTOR_HANDLE srvCpuHandle = m_pPostProcessCbvSrvHeap->GetCPUDescriptorHandleForHeapStart();
	m_pDevice->GetDevice()->CreateShaderResourceView(m_pPostProcessResource, &srvDesc, srvCpuHandle);
}

void SDirectX12::WaitForGPU(unsigned long long fenceValue)
{
	m_pFence->SetEventOnCompletion(fenceValue, m_hFenceEvent);
	WaitForSingleObjectEx(m_hFenceEvent, INFINITE, FALSE);
}

void SDirectX12::WaitForIdle()
{
	WaitForGPU(m_nFenceValue[m_BufferIndex] - 1);
}

void SDirectX12::InitBundle()
{
	m_pBundleList->Close();
}

#endif