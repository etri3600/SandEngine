#include "SDX12Pipeline.h"
#include "SWindows.h"
#include "SDX12Helper.h"

SDX12Pipeline::SDX12Pipeline(SDirectX12Device * pDevice, SDX12RootSignature* pRootSignature, EMaterialType materialType)
	:m_MaterialType(materialType)
{
	m_pDevice = pDevice;
	m_pRootSignature = pRootSignature;
}

SDX12Pipeline::~SDX12Pipeline()
{
	if (m_pPipelineState)
	{
		m_pPipelineState->Release();
		m_pPipelineState = nullptr;
	}

	if (m_pResources)
	{
		delete m_pResources;
		m_pResources = nullptr;
	}
}



void SDX12Pipeline::Init(std::wstring vertexShader, std::wstring fragmentShader, std::string vertexEntrypointName, std::string fragmentEntrypointName, SDX12Resources* pResources, UINT renderTargetCount, DXGI_FORMAT* renderTargetFormats)
{
	// Create Shader Binary
	ID3DBlob* pVertexShader = nullptr, *pPixelShader = nullptr;
	CompileShader(vertexShader.c_str(), "vs_5_1", vertexEntrypointName.c_str(), &pVertexShader);
	CompileShader(fragmentShader.c_str(), "ps_5_1", fragmentEntrypointName.c_str(), &pPixelShader);

	if (pVertexShader == nullptr || pPixelShader == nullptr)
		return;

	// Create Pipeline State
	m_pResources = pResources;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineStateDesc = {};
	pipelineStateDesc.InputLayout = { pResources->Attributes.GetAttributes(), pResources->Attributes.GetCount() };
	pipelineStateDesc.pRootSignature = m_pRootSignature->Get();
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
	pipelineStateDesc.NumRenderTargets = renderTargetCount;
	for(int i = 0; i < renderTargetCount; ++i)
		pipelineStateDesc.RTVFormats[i] = renderTargetFormats[i];
	pipelineStateDesc.SampleDesc.Count = 1;
	HRESULT hResult = m_pDevice->GetDevice()->CreateGraphicsPipelineState(&pipelineStateDesc, __uuidof(ID3D12PipelineState), reinterpret_cast<void**>(&m_pPipelineState));
	SWindows::OutputErrorMessage(hResult);
}

unsigned int SDX12Pipeline::GetCBVDescriptorOffset()
{
	return m_pResources->GetCBVOffset();
}

char * SDX12Pipeline::GetMappedConstantBuffer(unsigned int index)
{
	return m_pResources->GetMappedConstantBuffer(index);
}

void SDX12Pipeline::Populate(ID3D12GraphicsCommandList * commandList, unsigned int index)
{
	int count = 0;
	auto buffers = m_pResources->GetCBVBuffers(count);
	for (int i = 0; i < count; ++i)
	{
		commandList->SetGraphicsRootConstantBufferView(i, buffers[i]->GetGPUVirtualAddress());
	}
}

void SDX12Pipeline::CreateConstantBuffer(ID3D12DescriptorHeap* pDescriptorHeap, unsigned int descriptorOffset, unsigned int descriptorSize, SBatchProxy* batchProxy)
{
	m_pResources->CreateConstantBuffer(m_pDevice, pDescriptorHeap, descriptorOffset, descriptorSize, batchProxy);
}

void SDX12Pipeline::SetVertexBufferView(D3D12_GPU_VIRTUAL_ADDRESS bufferLocation, unsigned int sizeInBytes, unsigned int strideInBytes)
{
	m_VertexBufferView.BufferLocation = bufferLocation;
	m_VertexBufferView.SizeInBytes = sizeInBytes;
	m_VertexBufferView.StrideInBytes = strideInBytes;
}

void SDX12Pipeline::SetIndexBufferView(D3D12_GPU_VIRTUAL_ADDRESS bufferLocation, unsigned int sizeInBytes, DXGI_FORMAT format)
{
	m_IndexBufferView.BufferLocation = bufferLocation;
	m_IndexBufferView.SizeInBytes = sizeInBytes;
	m_IndexBufferView.Format = format;
}

void SDX12Pipeline::UpdateConstantBuffer(SBatchProxy* batchProxy, unsigned int objIndex, SMatrix view, SMatrix projection)
{
	m_pResources->UpdateConstantBuffer(batchProxy, objIndex, view, projection);
}