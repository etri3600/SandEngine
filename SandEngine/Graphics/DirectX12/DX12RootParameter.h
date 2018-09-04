#pragma once

#include "DirectX12.h"

struct SDX12RootParameter
{
	virtual ~SDX12RootParameter() {}
	virtual const D3D12_ROOT_PARAMETER* GetParameters() = 0;
	virtual unsigned int GetParameterCount() = 0;
	virtual const D3D12_STATIC_SAMPLER_DESC* GetSamplerDescs() = 0;
	virtual unsigned int GetSamplerDescCount() = 0;
	virtual unsigned int GetTableLocation() = 0;

	EMaterialType Type;
};

struct SDX12BoneRootParameter : SDX12RootParameter
{
	SDX12BoneRootParameter()
	{
		Type = EMaterialType::SKINNING;

		m_range[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		m_range[0].NumDescriptors = 32;
		m_range[0].BaseShaderRegister = 0;
		m_range[0].RegisterSpace = 0;
		m_range[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		m_parameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		m_parameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
		m_parameters[0].Descriptor.ShaderRegister = 0;
		m_parameters[0].Descriptor.RegisterSpace = 0;

		m_parameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		m_parameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
		m_parameters[1].Descriptor.ShaderRegister = 1;
		m_parameters[1].Descriptor.RegisterSpace = 0;

		m_parameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		m_parameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		m_parameters[2].DescriptorTable.NumDescriptorRanges = 1;
		m_parameters[2].DescriptorTable.pDescriptorRanges = &m_range[0];

		m_samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		m_samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		m_samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		m_samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		m_samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		m_samplerDesc.MinLOD = 0;
		m_samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
		m_samplerDesc.MipLODBias = 0.0f;
		m_samplerDesc.MaxAnisotropy = 1;
		m_samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
		m_samplerDesc.ShaderRegister = 0;
		m_samplerDesc.RegisterSpace = 0;
	}

	unsigned int GetParameterCount() { return sizeof(m_parameters) / sizeof(m_parameters[0]); }
	const D3D12_ROOT_PARAMETER* GetParameters() override { return m_parameters; }

	const D3D12_STATIC_SAMPLER_DESC* GetSamplerDescs() override { return &m_samplerDesc; };
	unsigned int GetSamplerDescCount() override { return 1; }

	unsigned int GetTableLocation() override { return 2; }
public:
	D3D12_DESCRIPTOR_RANGE m_range[1];
	D3D12_ROOT_PARAMETER m_parameters[3];
	D3D12_STATIC_SAMPLER_DESC m_samplerDesc;
};

struct SDX12TextureRootParameter : SDX12RootParameter
{
	SDX12TextureRootParameter()
	{
		Type = EMaterialType::TEXTURE;

		m_range[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		m_range[0].NumDescriptors = 32;
		m_range[0].BaseShaderRegister = 0;
		m_range[0].RegisterSpace = 0;
		m_range[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		m_parameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		m_parameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
		m_parameters[0].Descriptor.ShaderRegister = 0;
		m_parameters[0].Descriptor.RegisterSpace = 0;

		m_parameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		m_parameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		m_parameters[1].DescriptorTable.NumDescriptorRanges = 1;
		m_parameters[1].DescriptorTable.pDescriptorRanges = &m_range[0];

		m_samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		m_samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		m_samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		m_samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		m_samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		m_samplerDesc.MinLOD = 0;
		m_samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
		m_samplerDesc.MipLODBias = 0.0f;
		m_samplerDesc.MaxAnisotropy = 1;
		m_samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
		m_samplerDesc.ShaderRegister = 0;
		m_samplerDesc.RegisterSpace = 0;
	}

	unsigned int GetParameterCount() { return sizeof(m_parameters) / sizeof(m_parameters[0]); }
	const D3D12_ROOT_PARAMETER* GetParameters() override { return m_parameters; }
	
	const D3D12_STATIC_SAMPLER_DESC* GetSamplerDescs() override { return &m_samplerDesc; };
	unsigned int GetSamplerDescCount() override { return 1; }

	unsigned int GetTableLocation() override { return 1; }
private:
	D3D12_DESCRIPTOR_RANGE m_range[1];
	D3D12_ROOT_PARAMETER m_parameters[2];
	D3D12_STATIC_SAMPLER_DESC m_samplerDesc;
};

struct SDX12SsAoRootParameter : SDX12RootParameter
{
	SDX12SsAoRootParameter()
	{
		Type = EMaterialType::POSTPROCESS;

		m_range[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		m_range[0].NumDescriptors = 32;
		m_range[0].BaseShaderRegister = 0;
		m_range[0].RegisterSpace = 0;
		m_range[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		m_parameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		m_parameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		m_parameters[0].Descriptor.ShaderRegister = 0;
		m_parameters[0].Descriptor.RegisterSpace = 0;

		m_parameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		m_parameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		m_parameters[1].DescriptorTable.NumDescriptorRanges = 1;
		m_parameters[1].DescriptorTable.pDescriptorRanges = &m_range[0];

		m_samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		m_samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		m_samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		m_samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		m_samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		m_samplerDesc.MinLOD = 0;
		m_samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
		m_samplerDesc.MipLODBias = 0.0f;
		m_samplerDesc.MaxAnisotropy = 1;
		m_samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
		m_samplerDesc.ShaderRegister = 0;
		m_samplerDesc.RegisterSpace = 0;
	}

	unsigned int GetParameterCount() { return sizeof(m_parameters) / sizeof(m_parameters[0]); }
	const D3D12_ROOT_PARAMETER* GetParameters() override { return m_parameters; }

	const D3D12_STATIC_SAMPLER_DESC* GetSamplerDescs() override { return &m_samplerDesc; };
	unsigned int GetSamplerDescCount() override { return 1; }

	unsigned int GetTableLocation() override { return 1; }
public:
	D3D12_DESCRIPTOR_RANGE m_range[1];
	D3D12_ROOT_PARAMETER m_parameters[2];
	D3D12_STATIC_SAMPLER_DESC m_samplerDesc;
};

struct SDX12LightRootParameter : SDX12RootParameter
{
	SDX12LightRootParameter()
	{
		Type = EMaterialType::DEFERRED;

		m_range[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		m_range[0].NumDescriptors = 32;
		m_range[0].BaseShaderRegister = 0;
		m_range[0].RegisterSpace = 0;
		m_range[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		m_parameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		m_parameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		m_parameters[0].Descriptor.ShaderRegister = 0;
		m_parameters[0].Descriptor.RegisterSpace = 0;

		m_parameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		m_parameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		m_parameters[1].DescriptorTable.NumDescriptorRanges = 1;
		m_parameters[1].DescriptorTable.pDescriptorRanges = &m_range[0];

		m_samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		m_samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		m_samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		m_samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		m_samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		m_samplerDesc.MinLOD = 0;
		m_samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
		m_samplerDesc.MipLODBias = 0.0f;
		m_samplerDesc.MaxAnisotropy = 1;
		m_samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
		m_samplerDesc.ShaderRegister = 0;
		m_samplerDesc.RegisterSpace = 0;
	}

	unsigned int GetParameterCount() { return sizeof(m_parameters) / sizeof(m_parameters[0]); }
	const D3D12_ROOT_PARAMETER* GetParameters() override { return m_parameters; }

	const D3D12_STATIC_SAMPLER_DESC* GetSamplerDescs() override { return &m_samplerDesc; };
	unsigned int GetSamplerDescCount() override { return 1; }

	unsigned int GetTableLocation() override { return 1; }
public:
	D3D12_DESCRIPTOR_RANGE m_range[1];
	D3D12_ROOT_PARAMETER m_parameters[2];
	D3D12_STATIC_SAMPLER_DESC m_samplerDesc;
};