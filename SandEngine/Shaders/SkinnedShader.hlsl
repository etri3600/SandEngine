#include "Common.hlsli"
#include "Skinned.hlsli"

struct ViewProjection
{
	column_major matrix model;
	column_major matrix view;
	column_major matrix projection;
	column_major matrix normalMatrix;
};

struct BoneTransform
{
	column_major matrix transform[MAX_BONES];
};

ConstantBuffer<ViewProjection> ViewProjectionConstantBuffer : register(b0);
ConstantBuffer<BoneTransform> BoneTransformConstantBuffer : register(b1);

struct VertexShaderInput
{
	float3 pos : POSITION;
	float4 color : COLOR0;
	float3 normal : NORMAL0;
	float4 tangent : TANGENT0;
	float2 uv : TEXCOORD0;
	uint4 bone : BLENDINDICES0;
	float4 weight : BLENDWEIGHT0;
};

struct PixelShaderInput
{
	float4 fragPos : SV_POSITION;
	float3 PosW : POSITION0;
	float4 color : COLOR0;
	float4 NormalV : NORMAL0;
	float4 TangentV : TANGENT0;
	float2 uv : TEXCOORD0;
	float4 PosV : TEXCOORD1;
};

PixelShaderInput vert(VertexShaderInput input)
{
	PixelShaderInput output;

	column_major matrix boneTransform = CalcBoneTransform(BoneTransformConstantBuffer.transform, input.bone, input.weight);

	float4 posW = mul(boneTransform, float4(input.pos, 1.0f));
	posW = mul(ViewProjectionConstantBuffer.model, posW);
	float4 pos = mul(ViewProjectionConstantBuffer.view, posW);
	pos = mul(ViewProjectionConstantBuffer.projection, pos);

	output.fragPos = pos;
	output.PosW = posW.xyz;
	output.color = input.color;	
	float4 normalW = mul(ViewProjectionConstantBuffer.normalMatrix, mul(boneTransform, float4(input.normal, 1)));
	float4 tangentW = mul(ViewProjectionConstantBuffer.normalMatrix, mul(boneTransform, input.tangent));
	output.NormalV = mul(ViewProjectionConstantBuffer.view, normalW);
	output.TangentV = mul(ViewProjectionConstantBuffer.view, tangentW);
	output.uv = input.uv;
	output.PosV = mul(ViewProjectionConstantBuffer.view, posW);

	return output;
}

Texture2D<half4> diff : register(t0);
SamplerState samp : register(s0);

struct FragmentOutput
{
	float4 Color : SV_Target0;
	float4 Depth : SV_Target1;
	float4 Normal : SV_Target2;
};

FragmentOutput frag(PixelShaderInput input)
{
	FragmentOutput output;
	output.Color = diff.Sample(samp, input.uv);
	output.Depth = input.PosV;
	output.Normal = input.NormalV;
	return output;
}
