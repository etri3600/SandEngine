#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")

#include <d3d12shader.h>

#include "SDirectX12.h"
#include "SMath.h"
#include "SWindows.h"
#include "SDX12Helper.h"

SDirectX12::SDirectX12()
{
	memset(m_nFenceValue, 0, sizeof(m_nFenceValue));
}

SDirectX12::~SDirectX12()
{
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
	m_pDevice->Initialize(0, screenWidth, screenHeight, fullScreen, vSync);


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

	// Create SwapChain
	CreateSwapChain(pPlatformSystem, m_pDevice->GetDeviceNumerator(), m_pDevice->GetDeviceDenominator());

	
	// Create Root Signal
	D3D12_DESCRIPTOR_RANGE range[4];
	range[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	range[0].NumDescriptors = 1;
	range[0].BaseShaderRegister = 0;
	range[0].RegisterSpace = 0;
	range[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	range[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	range[1].NumDescriptors = 1;
	range[1].BaseShaderRegister = 1;
	range[1].RegisterSpace = 0;
	range[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	range[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	range[2].NumDescriptors = 1;
	range[2].BaseShaderRegister = 0;
	range[2].RegisterSpace = 0;
	range[2].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER parameter[1];
	parameter[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	parameter[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	parameter[0].DescriptorTable.NumDescriptorRanges = 3;
	parameter[0].DescriptorTable.pDescriptorRanges = &range[0];

	D3D12_STATIC_SAMPLER_DESC staticSampler;
	staticSampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	staticSampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	staticSampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSampler.MinLOD = 0;
	staticSampler.MaxLOD = D3D12_FLOAT32_MAX;
	staticSampler.MipLODBias = 0.0f;
	staticSampler.MaxAnisotropy = 1;
	staticSampler.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	staticSampler.ShaderRegister = 0;
	staticSampler.RegisterSpace = 0;

	D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | // Only the input assembler stage needs access to the constant buffer.
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;
	
	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Flags = rootSignatureFlags;
	rootSignatureDesc.NumParameters = 1;
	rootSignatureDesc.pParameters = parameter;
	rootSignatureDesc.NumStaticSamplers = 1;
	rootSignatureDesc.pStaticSamplers = &staticSampler;
	
	ID3DBlob* pBlob = nullptr;
	hResult = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pBlob, nullptr);
	SWindows::OutputErrorMessage(hResult);

	hResult = m_pDevice->GetDevice()->CreateRootSignature(0, pBlob->GetBufferPointer(), pBlob->GetBufferSize(), __uuidof(ID3D12RootSignature), reinterpret_cast<void**>(&m_pRootSignature));
	SWindows::OutputErrorMessage(hResult);


	// Create Shader Binary
	ID3DBlob* pVertexShader = nullptr, *pPixelShader = nullptr;
	m_vertexShader = CompileShader(L"SimpleVertexShader", "vs_5_1", &pVertexShader);
	m_pixelShader = CompileShader(L"SimplePixelShader", "ps_5_1", &pPixelShader);

	if ((m_vertexShader.size() == 0 && pVertexShader == nullptr) || (m_pixelShader.size() == 0 && pPixelShader == nullptr))
		return false;
	
	// Create Pipeline State
	D3D12_INPUT_ELEMENT_DESC inputLayout[]=
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA , 0},
		{ "BLENDINDICES", 0, DXGI_FORMAT_R8G8B8A8_UINT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA , 0 },
		{ "BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA , 0 },
	};

	D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineStateDesc = {};
	pipelineStateDesc.InputLayout = { inputLayout, sizeof(inputLayout) / sizeof(inputLayout[0]) };
	pipelineStateDesc.pRootSignature = m_pRootSignature;
	pipelineStateDesc.VS = { pVertexShader->GetBufferPointer(), pVertexShader->GetBufferSize() };
	pipelineStateDesc.PS = { pPixelShader->GetBufferPointer(), pPixelShader->GetBufferSize() };
	
	pipelineStateDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	pipelineStateDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
	pipelineStateDesc.RasterizerState.FrontCounterClockwise = TRUE;
	pipelineStateDesc.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
	pipelineStateDesc.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
	pipelineStateDesc.RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
	pipelineStateDesc.RasterizerState.DepthClipEnable = TRUE;
	pipelineStateDesc.RasterizerState.MultisampleEnable = FALSE;
	pipelineStateDesc.RasterizerState.AntialiasedLineEnable = FALSE;
	pipelineStateDesc.RasterizerState.ForcedSampleCount = 0;
	pipelineStateDesc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	pipelineStateDesc.BlendState.AlphaToCoverageEnable = FALSE;
	pipelineStateDesc.BlendState.IndependentBlendEnable = FALSE;
	const D3D12_RENDER_TARGET_BLEND_DESC defaultRenderTargetBlendDesc =
	{
		FALSE,FALSE,
		D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
		D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
		D3D12_LOGIC_OP_NOOP,
		D3D12_COLOR_WRITE_ENABLE_ALL,
	};
	for (unsigned int i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
		pipelineStateDesc.BlendState.RenderTarget[i] = defaultRenderTargetBlendDesc;

	pipelineStateDesc.DepthStencilState.DepthEnable = FALSE;
	pipelineStateDesc.DepthStencilState.StencilEnable = FALSE;
	pipelineStateDesc.SampleMask = UINT_MAX;
	pipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	pipelineStateDesc.NumRenderTargets = 1;
	pipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	pipelineStateDesc.SampleDesc.Count = 1;
	hResult = m_pDevice->GetDevice()->CreateGraphicsPipelineState(&pipelineStateDesc, __uuidof(ID3D12PipelineState), reinterpret_cast<void**>(&m_pPipelineState));
	SWindows::OutputErrorMessage(hResult);

	m_vertexShader.clear();
	m_pixelShader.clear();

	// Create Command List
	hResult = m_pDevice->GetDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, GetCommandAllocator(), m_pPipelineState, IID_PPV_ARGS(&m_pCommandList));
	SWindows::OutputErrorMessage(hResult);

	hResult = m_pDevice->GetDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_BUNDLE, m_pBundleAllocator, m_pPipelineState, IID_PPV_ARGS(&m_pBundleList));
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

	m_pShaderResourceManager = new SDX12ShaderResourceManager(m_pDevice, m_pCommandList);

	return true;
}

void SDirectX12::Finalize()
{
	if (m_pVertexBuffer)
		m_pVertexBuffer->Release();
	
	if (m_pIndexBuffer)
		m_pIndexBuffer->Release();

	if (m_pShaderBufferHeap)
		m_pShaderBufferHeap->Release();

	if (m_pCBVBuffer[0])
	{
		m_pCBVBuffer[0]->Unmap(0, nullptr);
		m_pCBVBuffer[0]->Release();
		MappedConstantBuffer[0] = nullptr;
	}

	if (m_pCBVBuffer[1])
	{
		m_pCBVBuffer[1]->Unmap(0, nullptr);
		m_pCBVBuffer[1]->Release();
		MappedConstantBuffer[1] = nullptr;
	}

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

	if (m_pRenderTargetViewHeap)
		m_pRenderTargetViewHeap->Release();

	if (m_pSwapChain)
		m_pSwapChain->Release();
	
	if (m_pCommandQueue)
		m_pCommandQueue->Release();
	
	if (m_pPipelineState)
		m_pPipelineState->Release();

	if (m_pDevice)
	{
		m_pDevice->Finalize();
		delete m_pDevice;
		m_pDevice = nullptr;
	}
}

bool SDirectX12::CreateSwapChain(const SPlatformSystem* pPlatformSystem, const int nNumerator, const int nDenominator)
{
	const SWindows* pWindowsSystem = static_cast<const SWindows*>(pPlatformSystem);
	if (pWindowsSystem == nullptr)
		return false;

	WaitForGPU();

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
	float fovAngleY = static_cast<float>(70.0f * M_PI) / 180.0f;

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

void SDirectX12::UpdateBoneTransform(const std::vector<SModel>& models)
{
	for (unsigned int i = 0;i < models.size(); ++i)
	{
		if (models[i].HasAnimation())
		{
			const auto& boneTransforms = models[i].GetFinalTransform();
			for (unsigned int j = 0;j < std::fmin(MAX_BONES, boneTransforms.size()); ++j)
			{
				m_SceneProxy[i].BoneTransform[j] = boneTransforms[j];
			}
		}
	}
}

bool SDirectX12::Update(const double delta)
{

	return true;
}

void SDirectX12::Draw(std::vector<SModel>& models)
{
	HRESULT hResult;
	m_SceneProxy.resize(models.size());
	for(auto sceneProxy : m_SceneProxy)
	{
		for (auto boneTransform : sceneProxy.BoneTransform)
		{
			memcpy(sceneProxy.BoneTransform, &SMatrix::Identity, sizeof(SMatrix));
		}
	}

	// Setting Default, Upload Heap
	D3D12_HEAP_PROPERTIES defaultHeapProperties;
	defaultHeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	defaultHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	defaultHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	defaultHeapProperties.CreationNodeMask = 1;
	defaultHeapProperties.VisibleNodeMask = 1;

	D3D12_HEAP_PROPERTIES uploadHeapProperties = defaultHeapProperties;
	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;

	// Vertex Buffer
	unsigned int vertexBufferSize = 0;
	for (unsigned int i = 0;i < models.size(); ++i)
	{
		m_SceneProxy[i].BaseVertexLocation = vertexBufferSize / sizeof(SModelVertex);
		vertexBufferSize += static_cast<unsigned int>(models[i].VertexSize());
	}

	ID3D12Resource* vertexBufferUpload = nullptr;

	D3D12_RESOURCE_DESC vertexBufferDesc;
	vertexBufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	vertexBufferDesc.Alignment = 0;
	vertexBufferDesc.Width = vertexBufferSize;
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

	m_pDevice->GetDevice()->GetDeviceRemovedReason();

	vertexBufferUpload->SetName(L"VertexBufferUpload");

	std::vector<SModelVertex> vertices;
	std::vector<unsigned int> indices;
	for (unsigned int i = 0;i < models.size(); ++i)
	{
		vertices.insert(std::end(vertices), std::begin(models[i].Vertices), std::end(models[i].Vertices));
		indices.insert(std::end(indices), std::begin(models[i].Indices), std::end(models[i].Indices));
	}

	D3D12_SUBRESOURCE_DATA vertexData = {};
	vertexData.pData = vertices.data();
	vertexData.RowPitch = vertices.size() * sizeof(SModelVertex);
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
	unsigned int indexBufferSize = 0;
	for (unsigned int i = 0;i < models.size(); ++i)
	{
		m_SceneProxy[i].StartIndexLocation = indexBufferSize / sizeof(unsigned int);
		indexBufferSize += static_cast<unsigned int>(models[i].IndexSize());
	}

	ID3D12Resource* indexbufferUpload = nullptr;
	D3D12_RESOURCE_DESC indexBufferDesc;
	indexBufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	indexBufferDesc.Alignment = 0;
	indexBufferDesc.Width = indexBufferSize;
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

	for (unsigned int i = 0; i < models.size(); ++i)
	{
		m_SceneProxy[i].IndexCountPerInstance += static_cast<unsigned int>(models[i].IndexCount());
	}

	// Create Shader buffer
	D3D12_DESCRIPTOR_HEAP_DESC cbheapDesc = {};
	cbheapDesc.NumDescriptors = static_cast<unsigned int>(models.size()) * c_NumShaderBuffer;
	cbheapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	cbheapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	hResult = m_pDevice->GetDevice()->CreateDescriptorHeap(&cbheapDesc, IID_PPV_ARGS(&m_pShaderBufferHeap));
	SWindows::OutputErrorMessage(hResult);
	m_pShaderBufferHeap->SetName(L"Shader Buffer Descriptor Heap");
	m_uiShaderBufferDescriptorSize = m_pDevice->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	CreateConstantBuffer(models);
	CreateShaderResources(models);

	for (unsigned int i = 0;i < models.size(); ++i)
	{
		m_VertexBufferView.SizeInBytes += static_cast<unsigned int>(models[i].VertexSize());
		m_IndexBufferView.SizeInBytes += static_cast<unsigned int>(models[i].IndexSize());
	}

	for (unsigned int i = 0;i < models.size(); ++i)
	{
		m_SceneProxy[i].Tranformation = SMath::Transform(models[i].Scale, models[i].Location, models[i].Rotation);
	}

	// Execute Command
	m_pCommandList->Close();
	ID3D12CommandList* ppCommandList[] = { m_pCommandList };
	m_pCommandQueue->ExecuteCommandLists(sizeof(ppCommandList) / sizeof(ppCommandList[0]), ppCommandList);

	m_VertexBufferView.BufferLocation = m_pVertexBuffer->GetGPUVirtualAddress();
	m_VertexBufferView.StrideInBytes = sizeof(SModelVertex);

	m_IndexBufferView.BufferLocation = m_pIndexBuffer->GetGPUVirtualAddress();
	m_IndexBufferView.Format = DXGI_FORMAT_R32_UINT;

	WaitForGPU();
}

bool SDirectX12::Render()
{
	HRESULT hResult = GetCommandAllocator()->Reset();
	SWindows::OutputErrorMessage(hResult);

	hResult = m_pCommandList->Reset(GetCommandAllocator(), m_pPipelineState);
	SWindows::OutputErrorMessage(hResult);

	m_pCommandList->RSSetViewports(1, &m_Viewport);
	D3D12_RECT ScissorRect = { 0L, 0L, static_cast<long>(m_Viewport.Width), static_cast<long>(m_Viewport.Height) };
	m_pCommandList->RSSetScissorRects(1, &ScissorRect);

	D3D12_RESOURCE_BARRIER renderTargetResourceBarrier = {};
	renderTargetResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	renderTargetResourceBarrier.Transition.pResource = GetRenderTarget();
	renderTargetResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	renderTargetResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	renderTargetResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	renderTargetResourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	m_pCommandList->ResourceBarrier(1, &renderTargetResourceBarrier);

	const float color[4] = { 0.1f, 0.5f, 0.7f, 1.0f };
	D3D12_CPU_DESCRIPTOR_HANDLE renderTargetViewHandle;
	renderTargetViewHandle.ptr = m_pRenderTargetViewHeap->GetCPUDescriptorHandleForHeapStart().ptr + m_BufferIndex * m_RenderTargetViewDescriptorSize;
	m_pCommandList->ClearRenderTargetView(renderTargetViewHandle, color, 0, nullptr);
	m_pCommandList->OMSetRenderTargets(1, &renderTargetViewHandle, false, nullptr);

	m_pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_pCommandList->IASetVertexBuffers(0, 1, &m_VertexBufferView);
	m_pCommandList->IASetIndexBuffer(&m_IndexBufferView);

	UpdateConstantBuffer();
	UpdateShaderResource();

	m_pCommandList->SetGraphicsRootSignature(m_pRootSignature);
	{
		ID3D12DescriptorHeap* ppHeaps[] = { m_pShaderBufferHeap };
		m_pCommandList->SetDescriptorHeaps(sizeof(ppHeaps) / sizeof(ppHeaps[0]), ppHeaps);
		D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = {};
		gpuHandle.ptr = m_pShaderBufferHeap->GetGPUDescriptorHandleForHeapStart().ptr;
		for (unsigned int i = 0;i < m_SceneProxy.size(); ++i)
		{
			m_pCommandList->SetGraphicsRootDescriptorTable(0, gpuHandle);
			m_pCommandList->DrawIndexedInstanced(m_SceneProxy[i].IndexCountPerInstance, 1, m_SceneProxy[i].StartIndexLocation, m_SceneProxy[i].BaseVertexLocation, 0);
			gpuHandle.ptr += m_uiShaderBufferDescriptorSize;
		}
	}


	D3D12_RESOURCE_BARRIER presentResourceBarrier = {};
	presentResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	presentResourceBarrier.Transition.pResource = GetRenderTarget();
	presentResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	presentResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	presentResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	presentResourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	m_pCommandList->ResourceBarrier(1, &presentResourceBarrier);

	m_pCommandList->Close();
	ID3D12CommandList* ppCommandLists[] = { m_pCommandList };
	m_pCommandQueue->ExecuteCommandLists(sizeof(ppCommandLists) / sizeof(ppCommandLists[0]), ppCommandLists);


	return true;
}

void SDirectX12::Present()
{
	m_pSwapChain->Present(1, 0);

	const unsigned __int64 currentFenceValue = m_nFenceValue[m_BufferIndex];
	HRESULT hResult = m_pCommandQueue->Signal(m_pFence, currentFenceValue);
	SWindows::OutputErrorMessage(hResult);

	m_BufferIndex = (m_BufferIndex + 1) % c_BufferingCount;

	if (m_pFence->GetCompletedValue() < m_nFenceValue[m_BufferIndex])
	{
		hResult = m_pFence->SetEventOnCompletion(m_nFenceValue[m_BufferIndex], m_hFenceEvent);
		SWindows::OutputErrorMessage(hResult);
		WaitForSingleObjectEx(m_hFenceEvent, INFINITE, FALSE);
	}

	m_nFenceValue[m_BufferIndex] = currentFenceValue + 1;
}

std::vector<byte>&& SDirectX12::CompileShader(const wchar_t* fileName, const char* version, ID3DBlob** pBlob)
{
	std::vector<byte> shader;
	constexpr auto shaderFolder = LR"(..\SandEngine\Shaders\)";
	constexpr auto shaderExtension = L".hlsl";
	constexpr auto csoFolder = LR"(..\Shaders\)";
	constexpr auto csoExtension = L".cso";
	std::wstring file(fileName);

	D3D_SHADER_MACRO defines[] =
	{
		{ nullptr, nullptr }
	};
	unsigned int flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(_DEBUG)
	flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
	ID3DBlob* pShaderCode = nullptr;
	auto stringFile = shaderFolder + file + shaderExtension;
	const wchar_t* shaderFileName = stringFile.c_str();
	HRESULT hResult = D3DCompileFromFile(shaderFileName, defines, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", version, flags, 0, &pShaderCode, nullptr);
	if (FAILED(hResult))
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
	else
	{
		*pBlob = pShaderCode;
	}

#if defined(_DEBUG)
	ID3D12ShaderReflection* pReflection = nullptr;
	if (SUCCEEDED(D3DReflect(pShaderCode->GetBufferPointer(), pShaderCode->GetBufferSize(), IID_ID3D12ShaderReflection, (void**)&pReflection)))
	{
		D3D12_SIGNATURE_PARAMETER_DESC desc[4];
		pReflection->GetInputParameterDesc(0, &desc[0]);
		pReflection->GetInputParameterDesc(1, &desc[1]);
		pReflection->GetInputParameterDesc(2, &desc[2]);
		pReflection->GetInputParameterDesc(3, &desc[3]);
	}
#endif
	
	return std::move(shader);
}

VOID SDirectX12::CreateConstantBuffer(std::vector<SModel>& models)
{
	D3D12_HEAP_PROPERTIES uploadHeapProperties;
	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
	uploadHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	uploadHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	uploadHeapProperties.CreationNodeMask = 1;
	uploadHeapProperties.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC constantBufferDesc = {};
	constantBufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	constantBufferDesc.Alignment = 0;
	constantBufferDesc.Height = 1;
	constantBufferDesc.DepthOrArraySize = 1;
	constantBufferDesc.MipLevels = 1;
	constantBufferDesc.Format = DXGI_FORMAT_UNKNOWN;
	constantBufferDesc.SampleDesc.Count = 1;
	constantBufferDesc.SampleDesc.Quality = 0;
	constantBufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	constantBufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	constantBufferDesc.Width = sizeof(SModelViewProjection) * models.size();
	HRESULT hResult = m_pDevice->GetDevice()->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &constantBufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_pCBVBuffer[0]));
	SWindows::OutputErrorMessage(hResult);
	m_pCBVBuffer[0]->SetName(L"MVP Buffer");

	constantBufferDesc.Width = sizeof(SBoneTransform) * models.size();
	hResult = m_pDevice->GetDevice()->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &constantBufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_pCBVBuffer[1]));
	SWindows::OutputErrorMessage(hResult);
	m_pCBVBuffer[1]->SetName(L"Bone Buffer");

	D3D12_GPU_VIRTUAL_ADDRESS cbvGPUAddress[2];
	cbvGPUAddress[0] = m_pCBVBuffer[0]->GetGPUVirtualAddress();
	cbvGPUAddress[1] = m_pCBVBuffer[1]->GetGPUVirtualAddress();
	D3D12_CPU_DESCRIPTOR_HANDLE cbvCPUHandle = m_pShaderBufferHeap->GetCPUDescriptorHandleForHeapStart();

	for (unsigned int j = 0;j < models.size(); ++j)
	{
		D3D12_CONSTANT_BUFFER_VIEW_DESC desc;
		desc.BufferLocation = cbvGPUAddress[0];
		desc.SizeInBytes = sizeof(SModelViewProjection);
		m_pDevice->GetDevice()->CreateConstantBufferView(&desc, cbvCPUHandle);

		cbvGPUAddress[0] += desc.SizeInBytes;
		cbvCPUHandle.ptr += m_uiShaderBufferDescriptorSize;

		desc.BufferLocation = cbvGPUAddress[1];
		desc.SizeInBytes = sizeof(SBoneTransform);
		m_pDevice->GetDevice()->CreateConstantBufferView(&desc, cbvCPUHandle);

		cbvGPUAddress[1] += desc.SizeInBytes;
		cbvCPUHandle.ptr += m_uiShaderBufferDescriptorSize;
	}

	m_uiCBVDescriptorOffset += m_uiShaderBufferDescriptorSize * 2;

	D3D12_RANGE readRange;
	readRange.Begin = readRange.End = 0;
	m_pCBVBuffer[0]->Map(0, &readRange, reinterpret_cast<void**>(&MappedConstantBuffer[0]));
	memset(MappedConstantBuffer[0], 0, sizeof(SModelViewProjection) * models.size());

	m_pCBVBuffer[1]->Map(0, &readRange, reinterpret_cast<void**>(&MappedConstantBuffer[1]));
	memset(MappedConstantBuffer[1], 0, sizeof(SBoneTransform) * models.size());
}

VOID SDirectX12::CreateShaderResources(std::vector<SModel>& models)
{
	auto& model = models[0];
	auto textures = model.GetTextures();

	if (textures.size() == 0)
		return;

	unsigned int TextureSize = static_cast<unsigned int>(model.Textures.size());
	unsigned int TextureWidth = static_cast<unsigned int>(model.TextureWidth(0)), TextureHeight = static_cast<unsigned int>(model.TextureHeight(0));

	// Create Texture
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
	textureDesc.Width = model.TextureWidth(0);
	textureDesc.Height = model.TextureHeight(0);
	textureDesc.DepthOrArraySize = 1;
	textureDesc.MipLevels = 1;
	textureDesc.Format = static_cast<DXGI_FORMAT>(model.TextureFormat(0));
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	HRESULT hResult = m_pDevice->GetDevice()->CreateCommittedResource(&defaultHeapProperties, D3D12_HEAP_FLAG_NONE, &textureDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&m_pSRVBuffer));
	SWindows::OutputErrorMessage(hResult);
	m_pSRVBuffer->SetName(L"Texture2D");

	auto uploadBufferSize = GetRequiredIntermediateSize(m_pSRVBuffer, 0, 1);

	textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	textureDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	textureDesc.Format = DXGI_FORMAT_UNKNOWN;
	textureDesc.Width = uploadBufferSize;
	textureDesc.Height = 1;
	hResult = m_pDevice->GetDevice()->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &textureDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&texturebuffer));
	SWindows::OutputErrorMessage(hResult);
	texturebuffer->SetName(L"Upload Texture2D");

	D3D12_SUBRESOURCE_DATA textureData = {};
	textureData.pData = model.TextureSerialize(0);
	textureData.RowPitch = TextureWidth * model.TexelSize(0);
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
	srvDesc.Format = static_cast<DXGI_FORMAT>(model.TextureFormat(0));
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	D3D12_CPU_DESCRIPTOR_HANDLE gpuHandle = m_pShaderBufferHeap->GetCPUDescriptorHandleForHeapStart();
	gpuHandle.ptr += m_uiCBVDescriptorOffset;
	m_pDevice->GetDevice()->CreateShaderResourceView(m_pSRVBuffer, &srvDesc, gpuHandle);
}

void SDirectX12::UpdateConstantBuffer()
{
	// ModelViewProjection
	{
		SModelViewProjection mvp;
		memset(&mvp, 0, sizeof(SModelViewProjection));
		mvp.View = View;
		mvp.Projection = Projection;

		for (unsigned int i = 0; i < m_SceneProxy.size(); ++i)
		{
			mvp.Model = m_SceneProxy[i].Tranformation;
			mvp.NormalMatrix = SMath::NormalMatrix(m_SceneProxy[i].Tranformation);
			auto offset = i * sizeof(SModelViewProjection);
			SModelViewProjection* pMVP = reinterpret_cast<SModelViewProjection*>(MappedConstantBuffer[0] + offset);

			memcpy(pMVP, &mvp, sizeof(SModelViewProjection));
		}
	}

	// Bone Transformation
	{
		SBoneTransform bones;
		for (unsigned int i = 0;i < m_SceneProxy.size(); ++i)
		{
			for (unsigned int j = 0;j < MAX_BONES; ++j)
			{
				bones.transform[j] = m_SceneProxy[i].BoneTransform[j];
			}

			auto offset = i * sizeof(SBoneTransform);
			SBoneTransform* pBone = reinterpret_cast<SBoneTransform*>(MappedConstantBuffer[1] + offset);
			memcpy(pBone, &bones, sizeof(SBoneTransform));
		}
	}
}

void SDirectX12::UpdateShaderResource()
{
}

void SDirectX12::WaitForGPU()
{
	m_pCommandQueue->Signal(m_pFence, m_nFenceValue[m_BufferIndex]);
	m_pFence->SetEventOnCompletion(m_nFenceValue[m_BufferIndex], m_hFenceEvent);
	WaitForSingleObjectEx(m_hFenceEvent, INFINITE, FALSE);
	++m_nFenceValue[m_BufferIndex];
}

void SDirectX12::InitBundle()
{
	m_pBundleList->Close();
}
