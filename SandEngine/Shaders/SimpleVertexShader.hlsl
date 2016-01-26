struct ViewProjection
{
	column_major matrix model;
	column_major matrix view;
	column_major matrix projection;
	column_major matrix normalMatrix;
};

#define MAX_BONES 128

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
	float3 NormalW : NORMAL0;
	float3 TangentW : TANGENT0;
	float2 uv : TEXCOORD0;
};

static column_major matrix Identity = 
{
	{ 1.0f, 0.0f, 0.0f, 0.0f },
	{ 0.0f, 1.0f, 0.0f, 0.0f },
	{ 0.0f, 0.0f, 1.0f, 0.0f },
	{ 0.0f, 0.0f, 0.0f, 1.0f }
};

PixelShaderInput main(VertexShaderInput input)
{
	PixelShaderInput output;

	column_major matrix boneTransform = mul(BoneTransformConstantBuffer.transform[input.bone[0]], input.weight[0]);
	boneTransform += mul(BoneTransformConstantBuffer.transform[input.bone[1]], input.weight[1]);
	boneTransform += mul(BoneTransformConstantBuffer.transform[input.bone[2]], input.weight[2]);
	boneTransform += mul(BoneTransformConstantBuffer.transform[input.bone[3]], input.weight[3]);
	
	float4 posW = mul(boneTransform, float4(input.pos, 1.0f));
	posW = mul(ViewProjectionConstantBuffer.model, posW);
	float4 pos = mul(ViewProjectionConstantBuffer.view, posW);
	pos = mul(ViewProjectionConstantBuffer.projection, pos);

	output.fragPos = pos;
	output.PosW = posW.xyz;
	output.color = input.color;
	output.NormalW = mul(ViewProjectionConstantBuffer.normalMatrix, mul(boneTransform, input.normal)).xyz;
	output.TangentW = mul(ViewProjectionConstantBuffer.normalMatrix, mul(boneTransform, input.tangent)).xyz;
	output.uv = input.uv;

	return output;
}