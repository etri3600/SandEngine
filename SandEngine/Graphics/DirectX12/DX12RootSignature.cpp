#include "DX12RootSignature.h"
#include "Windows.h"

SDX12RootSignature::SDX12RootSignature(SDirectX12Device* pDevice, SDX12RootParameter* pRootParameter)
{
	m_pRootParameter = pRootParameter;

	D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | // Only the input assembler stage needs access to the constant buffer.
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;


	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Flags = rootSignatureFlags;
	rootSignatureDesc.NumParameters = pRootParameter->GetParameterCount();
	rootSignatureDesc.pParameters = pRootParameter->GetParameters();
	rootSignatureDesc.NumStaticSamplers = pRootParameter->GetSamplerDescCount();
	rootSignatureDesc.pStaticSamplers = pRootParameter->GetSamplerDescs();

	ID3DBlob* pBlob = nullptr;
	HRESULT hResult = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pBlob, nullptr);

	hResult = pDevice->GetDevice()->CreateRootSignature(0, pBlob->GetBufferPointer(), pBlob->GetBufferSize(), __uuidof(ID3D12RootSignature), reinterpret_cast<void**>(&m_pRootSignature));
}

unsigned int SDX12RootSignature::GetTableLocation()
{
	return m_pRootParameter->GetTableLocation();
}