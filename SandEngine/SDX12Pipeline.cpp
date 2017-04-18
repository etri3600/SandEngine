#include "SDX12Pipeline.h"
#include "SWindows.h"

SDX12Pipeline::SDX12Pipeline(SDirectX12Device * pDevice)
{
	m_pDevice = pDevice;
}

void SDX12Pipeline::Init(std::wstring vertexShader, std::wstring pixelShader)
{
	// Create Root Signal
	D3D12_DESCRIPTOR_RANGE range[1];
	range[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	range[0].NumDescriptors = 64;
	range[0].BaseShaderRegister = 0;
	range[0].RegisterSpace = 0;
	range[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER parameter[3];
	parameter[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	parameter[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	parameter[0].Descriptor.ShaderRegister = 0;
	parameter[0].Descriptor.RegisterSpace = 0;

	parameter[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	parameter[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	parameter[1].Descriptor.ShaderRegister = 1;
	parameter[1].Descriptor.RegisterSpace = 0;

	parameter[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	parameter[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	parameter[2].DescriptorTable.NumDescriptorRanges = 1;
	parameter[2].DescriptorTable.pDescriptorRanges = &range[0];

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
	rootSignatureDesc.NumParameters = 3;
	rootSignatureDesc.pParameters = parameter;
	rootSignatureDesc.NumStaticSamplers = 1;
	rootSignatureDesc.pStaticSamplers = &staticSampler;
	
	ID3DBlob* pBlob = nullptr;
	HRESULT hResult = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pBlob, nullptr);
	SWindows::OutputErrorMessage(hResult);

	hResult = m_pDevice->GetDevice()->CreateRootSignature(0, pBlob->GetBufferPointer(), pBlob->GetBufferSize(), __uuidof(ID3D12RootSignature), reinterpret_cast<void**>(&m_pRootSignature));
	SWindows::OutputErrorMessage(hResult);

	// Create Shader Binary
	ID3DBlob* pVertexShader = nullptr, *pPixelShader = nullptr;
	m_vertexShader = CompileShader(vertexShader.c_str(), "vs_5_1", &pVertexShader);
	m_pixelShader = CompileShader(pixelShader.c_str(), "ps_5_1", &pPixelShader);

	if ((m_vertexShader.size() == 0 && pVertexShader == nullptr) || (m_pixelShader.size() == 0 && pPixelShader == nullptr))
		return;

	// Create Pipeline State
	D3D12_INPUT_ELEMENT_DESC inputLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA , 0 },
		{ "BLENDINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA , 0 },
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
}

std::vector<byte>&& SDX12Pipeline::CompileShader(const wchar_t* fileName, const char* version, ID3DBlob** pBlob)
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