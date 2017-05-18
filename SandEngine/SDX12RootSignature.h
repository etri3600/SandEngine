#pragma once

#include "SDX12RootParameter.h"

class SDX12RootSignature
{
public:
	SDX12RootSignature(SDirectX12Device* pDevice, SDX12RootParameter* pRootParameter);
	ID3D12RootSignature* Get() { return m_pRootSignature; }
	MaterialType GetType() { return m_pRootParameter->Type; }
	unsigned int GetTableLocation();

private:
	ID3D12RootSignature* m_pRootSignature;
	SDX12RootParameter* m_pRootParameter;
};