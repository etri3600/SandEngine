#pragma once

#include <vector>
#include <string>
#include "SDirectX12.h"
#include "SDX12InputVertexAttributes.h"

class SDX12Pipeline
{
public:
	SDX12Pipeline(SDirectX12Device* pDevice, ID3D12RootSignature* pRootSignature);
	~SDX12Pipeline();

	void Init(std::wstring vertexShader, std::wstring pixelShader, const SDX12InputVertexAttributes attribute);
	std::vector<byte>&& CompileShader(const wchar_t* fileName, const char* version, ID3DBlob** pBlob);

	ID3D12RootSignature* GetRootSignature() { return m_pRootSignature; }
	ID3D12PipelineState* GetPipelineState() { return m_pPipelineState; }

	void Populate(ID3D12GraphicsCommandList* commandList);
	unsigned int GetCBVOffset() { return m_uiCBVDescriptorOffset; }

private:
	SDirectX12Device* m_pDevice;
	std::vector<byte> m_vertexShader, m_pixelShader;
	ID3D12RootSignature* m_pRootSignature;
	ID3D12PipelineState* m_pPipelineState;
	std::vector<ID3D12Resource*> m_pCBVBuffers;
	unsigned int m_uiCBVDescriptorOffset;
	std::vector<ID3D12Resource*> m_pSRVBuffers;
};