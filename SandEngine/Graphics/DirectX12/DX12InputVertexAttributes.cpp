#if __WINDOWS__
#include "DX12InputVertexAttributes.h"

void SDX12InputVertexAttributes::AddAttribute(D3D12_INPUT_ELEMENT_DESC attribute)
{
	m_attributes.push_back(attribute);
}

const D3D12_INPUT_ELEMENT_DESC* SDX12InputVertexAttributes::GetAttributes() const
{
	return m_attributes.data();
}

unsigned int SDX12InputVertexAttributes::GetCount() const
{
	return m_attributes.size();
}
#endif