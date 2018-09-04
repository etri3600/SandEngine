#pragma once

#include <vector>
#include "DirectX12.h"

struct SDX12InputVertexAttributes
{
	void AddAttribute(D3D12_INPUT_ELEMENT_DESC attribute);

	const D3D12_INPUT_ELEMENT_DESC* GetAttributes() const;
	unsigned int GetCount() const;

private:
	std::vector<D3D12_INPUT_ELEMENT_DESC> m_attributes;
};