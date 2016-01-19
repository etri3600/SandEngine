struct ViewProjection
{
	column_major matrix model;
	column_major matrix view;
	column_major matrix projection;
	column_major matrix invertTransposeModel;
};

struct BoneTransform
{
	column_major matrix transform[96];
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
	float weight : BLENDWEIGHT0;
};

struct PixelShaderInput
{
	float4 fragPos : SV_POSITION;
	float3 worldPos : POSITION0;
	float4 color : COLOR0;
	float3 worldNormal : NORMAL0;
	float3 worldTangent : TANGENT0;
	float2 uv : TEXCOORD0;
};

PixelShaderInput main(VertexShaderInput input)
{
	PixelShaderInput output;

	float weight0 = input.weight;
	float weight1 = 1.0f - weight0;
	column_major matrix a = BoneTransformConstantBuffer.transform[input.bone[0]];
	float4 positionOffset = weight0 * mul(BoneTransformConstantBuffer.transform[input.bone[0]], float4(input.pos, 1.0f));
	positionOffset += weight1 * mul(BoneTransformConstantBuffer.transform[input.bone[1]], float4(input.pos, 1.0f));
	positionOffset.w = 1.0f;
	
	float4 normalOffset = weight0 * mul(BoneTransformConstantBuffer.transform[input.bone[0]], float4(input.normal, 0.0f));
	normalOffset += weight1 * mul(BoneTransformConstantBuffer.transform[input.bone[1]], float4(input.normal, 0.0f));
	normalOffset.w = 0.0f;

	float4 tangetOffset = weight0 * mul(BoneTransformConstantBuffer.transform[input.bone[0]], input.tangent);
	tangetOffset += weight1 * mul(BoneTransformConstantBuffer.transform[input.bone[1]], input.tangent);
	tangetOffset.w = 0.0f;

	float4 pos = float4(input.pos, 1.0f);
	pos = mul(ViewProjectionConstantBuffer.model, pos);
	pos = mul(ViewProjectionConstantBuffer.view, pos);
	pos = mul(ViewProjectionConstantBuffer.projection, pos);


	output.fragPos = pos;
	output.worldPos = mul(ViewProjectionConstantBuffer.model, positionOffset).xyz;
	output.color = input.color;
	output.worldNormal = mul(ViewProjectionConstantBuffer.invertTransposeModel, normalOffset).xyz;
	output.worldTangent = mul(ViewProjectionConstantBuffer.model, tangetOffset).xyz;
	output.uv = input.uv;

	return output;
}